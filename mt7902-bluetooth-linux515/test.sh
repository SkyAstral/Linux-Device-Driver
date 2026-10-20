#!/bin/bash
set -euo pipefail
cd -- "$(dirname -- "${BASH_SOURCE[0]}")"
[[ $EUID == 0 ]] || { echo '请运行 sudo bash ./test.sh'; exit 1; }
[[ $(uname -r) == 5.15.0-139-generic ]] || { echo '内核版本不匹配，请先重新编译'; exit 1; }
[[ ! -e state ]] || { echo '上次测试状态仍存在，请先运行 rollback.sh'; exit 1; }
iface=''
for dev in /sys/bus/usb/devices/*; do
 [[ -f "$dev/idVendor" && -f "$dev/idProduct" ]] || continue
 if [[ $(cat "$dev/idVendor") == 13d3 && $(cat "$dev/idProduct") == 3596 ]]; then
  iface="$(basename "$dev"):1.0"
  break
 fi
done
[[ -n $iface && -e /sys/bus/usb/devices/$iface ]] || { echo '没有找到 13d3:3596 接口'; exit 1; }
mkdir state
printf '%s\n' "$iface" > state/interface
fw=/lib/firmware/mediatek/BT_RAM_CODE_MT7902_1_1_hdr.bin
if [[ -e $fw ]]; then cp -a "$fw" state/firmware.original; else touch state/firmware.absent; fi
trap 'echo "测试失败，执行回滚"; bash ./rollback.sh' ERR
install -m 644 firmware/BT_RAM_CODE_MT7902_1_1_hdr.bin "$fw"
modprobe btusb
modprobe btintel
modprobe btbcm
modprobe btrtl
insmod ./btusb_mt7902.ko enable_autosuspend=0
if [[ -L /sys/bus/usb/devices/$iface/driver ]]; then
 printf '%s' "$iface" > "/sys/bus/usb/devices/$iface/driver/unbind"
fi
printf '%s' "$iface" > /sys/bus/usb/drivers/btusb_mt7902/bind
sleep 8
journalctl -k --since '-2 minutes' --no-pager > test-kernel.log
cat test-kernel.log
hciconfig -a
timeout 10 bluetoothctl list || true
echo '测试完成。请查看是否出现有效蓝牙地址与控制器。此脚本不配置开机加载。'
echo '恢复原驱动和原固件：sudo bash ./rollback.sh'
