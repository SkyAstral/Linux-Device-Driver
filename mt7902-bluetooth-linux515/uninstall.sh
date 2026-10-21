#!/bin/bash
set -euo pipefail
[[ $EUID == 0 ]] || { echo '请使用 sudo 运行'; exit 1; }
backup=/var/lib/mt7902-bluetooth
systemctl disable --now mt7902-bluetooth.service || true
rm -f /etc/systemd/system/mt7902-bluetooth.service /etc/udev/rules.d/71-mt7902-bluetooth.rules /etc/modprobe.d/btusb-mt7902.conf /usr/local/sbin/bind-mt7902-bluetooth
systemctl daemon-reload
udevadm control --reload-rules
for dev in /sys/bus/usb/devices/*; do
 [[ -f "$dev/idVendor" && -f "$dev/idProduct" ]] || continue
 [[ $(cat "$dev/idVendor") == 13d3 && $(cat "$dev/idProduct") == 3596 ]] || continue
 iface="$(basename "$dev"):1.0"
 if [[ -L /sys/bus/usb/devices/$iface/driver ]] && [[ $(basename "$(readlink -f "/sys/bus/usb/devices/$iface/driver")") == btusb_mt7902 ]]; then
  printf '%s' "$iface" > /sys/bus/usb/drivers/btusb_mt7902/unbind
 fi
done
if [[ -d /sys/module/btusb_mt7902 ]]; then rmmod btusb_mt7902; fi
dkms remove -m btusb-mt7902 -v 0.1 --all
fw=/lib/firmware/mediatek/BT_RAM_CODE_MT7902_1_1_hdr.bin
if [[ -e $backup/firmware.original ]]; then
 cp -a "$backup/firmware.original" "$fw"
elif [[ -e $backup/firmware.absent ]]; then
 rm -f -- "$fw"
fi
depmod -a
modprobe btusb
for dev in /sys/bus/usb/devices/*; do
 [[ -f "$dev/idVendor" && -f "$dev/idProduct" ]] || continue
 [[ $(cat "$dev/idVendor") == 13d3 && $(cat "$dev/idProduct") == 3596 ]] || continue
 iface="$(basename "$dev"):1.0"
 if [[ -e /sys/bus/usb/devices/$iface && ! -L /sys/bus/usb/devices/$iface/driver ]]; then
  printf '%s' "$iface" > /sys/bus/usb/drivers/btusb/bind
 fi
done
echo '永久配置已卸载，已恢复原固件和 btusb 绑定。备份仍保存在 /var/lib/mt7902-bluetooth。'
