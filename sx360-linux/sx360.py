#!/usr/bin/env python3
"""Experimental SX360 HID screen client, Python 3.8+, no dependencies."""
import argparse
import datetime
import json
import os
from pathlib import Path
import struct
import sys
import time
import math
import signal

DESCRIPTOR = bytes.fromhex('05010900a101150025ff1901290895407508810219012908954075089102c0')


def discover():
    result = []
    for node in sorted(Path('/sys/class/hidraw').glob('hidraw*')):
        dev = node / 'device'
        try:
            info = dict(line.split('=', 1) for line in (dev / 'uevent').read_text().splitlines() if '=' in line)
            bus, vendor, product = info.get('HID_ID', '0:0:0').split(':')
            if int(vendor, 16) != 0x0145:
                continue
            descriptor = (dev / 'report_descriptor').read_bytes()
            result.append({'path': '/dev/' + node.name, 'vid': int(vendor, 16),
                           'pid': int(product, 16), 'name': info.get('HID_NAME', ''),
                           'descriptor_matches': descriptor == DESCRIPTOR,
                           'report_descriptor': descriptor.hex()})
        except (OSError, ValueError):
            continue
    return result


def cpu_temperature(sensor_path=None):
    if sensor_path:
        path = Path(sensor_path)
        value = float(path.read_text().strip()) / 1000
        if not math.isfinite(value) or not 0 <= value <= 120:
            raise ValueError('温度传感器读数异常：{}'.format(value))
        return round(value), str(path)
    for hwmon in sorted(Path('/sys/class/hwmon').glob('hwmon*')):
        try:
            name = (hwmon / 'name').read_text().strip()
        except OSError:
            continue
        if name not in ('k10temp', 'coretemp', 'zenpower', 'zen5temp'):
            continue
        for sensor in sorted(hwmon.glob('temp*_input')):
            try:
                value = float(sensor.read_text()) / 1000
                label = sensor.with_name(sensor.name.replace('_input', '_label'))
                text = label.read_text().strip() if label.exists() else ''
                if (text in ('Tctl', 'Tdie', 'Package id 0') or not text) and math.isfinite(value) and 0 <= value <= 120:
                    return round(value), str(sensor)
            except (OSError, ValueError):
                continue
    raise ValueError('找不到真实 CPU 温度。请先启用支持此 CPU 的温度驱动，或用 --sensor 指定毫摄氏度传感器文件。--cpu-temp 仅用于固定值测试。')


def cpu_counters():
    fields = Path('/proc/stat').read_text().splitlines()[0].split()
    values = [int(value) for value in fields[1:9]]
    return sum(values), values[3] + (values[4] if len(values) > 4 else 0)


def cpu_frequency():
    values = []
    for line in Path('/proc/cpuinfo').read_text().splitlines():
        if line.startswith('cpu MHz'):
            values.append(float(line.split(':', 1)[1]))
    return min(65535, round(sum(values) / len(values))) if values else 0


def select_device(path=None):
    devices = [d for d in discover() if d['pid'] == 0x1001 and d['descriptor_matches']]
    if path:
        devices = [d for d in devices if d['path'] == path]
    if len(devices) != 1:
        raise ValueError('未找到唯一的兼容屏幕，请运行 probe 并通过 --device 指定路径。')
    return devices[0]['path']


def send_packet(path, packet):
    # Recheck each update, since hidraw numbers can change after reconnection.
    path = select_device(path)
    report = b'\x00' + packet
    fd = os.open(path, os.O_WRONLY | os.O_NONBLOCK)
    try:
        written = os.write(fd, report)
        if written != len(report):
            raise OSError('HID 写入不完整：{}/{}'.format(written, len(report)))
    finally:
        os.close(fd)


def positive_interval(value):
    result = float(value)
    if not math.isfinite(result) or result < 0.5:
        raise argparse.ArgumentTypeError('刷新间隔必须是至少 0.5 秒的有限数值。')
    return result


def build_packet(cpu_temp, gpu_temp, cpu_load, gpu_load, cpu_mhz, gpu_mhz, rpm, mode, now=None):
    """64-byte payload inferred from TempComm.dll RVA 0x1df4; unverified on hardware."""
    now = now or datetime.datetime.now()
    packet = bytearray(64)
    packet[0:7] = bytes([2, cpu_temp, gpu_temp, now.hour, now.minute, now.second, now.isoweekday()])
    struct.pack_into('>H', packet, 7, now.year)
    packet[9:13] = bytes([now.month, now.day, 0, cpu_load])
    struct.pack_into('>H', packet, 13, cpu_mhz)
    packet[15] = gpu_load
    struct.pack_into('>H', packet, 16, gpu_mhz)
    packet[48] = mode
    struct.pack_into('>H', packet, 49, rpm)
    return bytes(packet)


def bounded(low, high):
    def parse(value):
        number = int(value, 0)
        if not low <= number <= high:
            raise argparse.ArgumentTypeError('数值必须在 {} 到 {} 之间'.format(low, high))
        return number
    return parse


def main():
    parser = argparse.ArgumentParser(description='SX360 实验性屏显程序：默认只识别设备，不写入。')
    parser.add_argument('command', choices=['probe', 'preview', 'send', 'watch'], nargs='?', default='probe')
    parser.add_argument('--device', help='明确指定 /dev/hidrawN')
    parser.add_argument('--cpu-temp', type=bounded(0, 120))
    parser.add_argument('--sensor', help='CPU 温度传感器文件，单位为毫摄氏度')
    parser.add_argument('--interval', type=positive_interval, default=1.0, help='watch 刷新间隔，默认 1 秒')
    parser.add_argument('--count', type=bounded(1, 1000000), help='watch 刷新次数，默认持续运行')
    parser.add_argument('--dry-run', action='store_true', help='watch 只读取并打印，不写屏幕')
    parser.add_argument('--gpu-temp', type=bounded(0, 120), default=0)
    parser.add_argument('--cpu-load', type=bounded(0, 100))
    parser.add_argument('--gpu-load', type=bounded(0, 100), default=0)
    parser.add_argument('--cpu-mhz', type=bounded(0, 65535))
    parser.add_argument('--gpu-mhz', type=bounded(0, 65535), default=0)
    parser.add_argument('--rpm', type=bounded(0, 65535), default=0, help='显示的转速，不调节转速')
    parser.add_argument('--mode', type=bounded(0, 255), default=0, help='原始模式值，具体含义未验证')
    parser.add_argument('--experimental-write', action='store_true', help='允许发送一次尚未实机验证的状态包')
    args = parser.parse_args()
    if args.command == 'probe':
        print(json.dumps(discover(), ensure_ascii=False, indent=2))
        return 0
    if args.cpu_temp is not None and args.sensor:
        raise ValueError('--cpu-temp 和 --sensor 不能同时指定。')
    if args.command == 'watch':
        if not args.dry_run and not args.experimental_write:
            raise ValueError('持续写屏幕需指定 --experimental-write，或用 --dry-run 只读预览。')
        # Verify a real sensor before any device write.
        if args.cpu_temp is None:
            cpu_temperature(args.sensor)
        else:
            print('固定温度测试模式，不是真实 CPU 温度。', flush=True)
        previous = cpu_counters()
        completed = 0
        while args.count is None or completed < args.count:
            time.sleep(args.interval)
            temp, source = (args.cpu_temp, '固定值测试') if args.cpu_temp is not None else cpu_temperature(args.sensor)
            current = cpu_counters()
            total, idle = current[0] - previous[0], current[1] - previous[1]
            previous = current
            load = max(0, min(100, round(100 * (total - idle) / total))) if total > 0 else 0
            load = args.cpu_load if args.cpu_load is not None else load
            mhz = args.cpu_mhz if args.cpu_mhz is not None else cpu_frequency()
            packet = build_packet(temp, args.gpu_temp, load, args.gpu_load, mhz, args.gpu_mhz, args.rpm, args.mode)
            if not args.dry_run:
                send_packet(select_device(args.device), packet)
            completed += 1
            print('{} CPU={}°C 使用率={}％ 频率={}MHz {} 来源={}'.format(
                datetime.datetime.now().strftime('%H:%M:%S'), temp, load, mhz,
                '预览' if args.dry_run else '已发送', source), flush=True)
        return 0
    temp, source = (args.cpu_temp, '命令行固定值') if args.cpu_temp is not None else cpu_temperature(args.sensor)
    if not 0 <= temp <= 120:
        raise ValueError('CPU 温度读数异常：{}'.format(temp))
    packet = build_packet(temp, args.gpu_temp, args.cpu_load or 0, args.gpu_load,
                          args.cpu_mhz if args.cpu_mhz is not None else cpu_frequency(), args.gpu_mhz, args.rpm, args.mode)
    print('CPU 温度：{} °C；来源：{}'.format(temp, source))
    print('64 字节载荷：' + packet.hex(' '))
    if args.command == 'preview':
        print('仅预览，没有打开或写入 USB 设备。')
        return 0
    if not args.experimental_write or not args.device:
        raise ValueError('发送需要同时指定 --device 和 --experimental-write。')
    send_packet(args.device, packet)
    print('已发送一次状态包；写入成功不代表屏幕已经接受，请观察屏幕。')
    return 0


if __name__ == '__main__':
    def stop(signum, frame):
        raise KeyboardInterrupt
    signal.signal(signal.SIGTERM, stop)
    try:
        sys.exit(main())
    except KeyboardInterrupt:
        print('\n已停止刷新。', file=sys.stderr)
        sys.exit(0)
    except (OSError, ValueError) as exc:
        print('错误：{}'.format(exc), file=sys.stderr)
        sys.exit(1)
