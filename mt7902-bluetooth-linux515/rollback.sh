#!/bin/bash
set -euo pipefail
cd -- "$(dirname -- "${BASH_SOURCE[0]}")"
[[ $EUID == 0 ]] || { echo '请运行 sudo bash ./rollback.sh'; exit 1; }
[[ ! -f /etc/systemd/system/mt7902-bluetooth.service ]] || { echo "已永久安装，请使用 sudo bash /var/lib/mt7902-bluetooth/uninstall.sh"; exit 1; }
[[ -f state/interface ]] || { echo '没有待回滚的测试状态'; exit 0; }
iface=$(cat state/interface)
if [[ -L /sys/bus/usb/devices/$iface/driver ]] && [[ $(basename "$(readlink -f "/sys/bus/usb/devices/$iface/driver")") == btusb_mt7902 ]]; then
 printf '%s' "$iface" > /sys/bus/usb/drivers/btusb_mt7902/unbind
fi
if [[ -d /sys/module/btusb_mt7902 ]]; then rmmod btusb_mt7902; fi
fw=/lib/firmware/mediatek/BT_RAM_CODE_MT7902_1_1_hdr.bin
if [[ -e state/firmware.original ]]; then
 cp -a state/firmware.original "$fw"
elif [[ -e state/firmware.absent ]]; then
 rm -f -- "$fw"
fi
modprobe btusb
if [[ -e /sys/bus/usb/devices/$iface && ! -L /sys/bus/usb/devices/$iface/driver ]]; then
 printf '%s' "$iface" > /sys/bus/usb/drivers/btusb/bind
fi
mv state "state-restored-$(date +%Y%m%d-%H%M%S)"
echo '已恢复原驱动和固件。若控制器仍无响应，可能需要完全关机断电后再启动。'
