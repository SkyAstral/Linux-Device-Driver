#!/bin/bash
set -euo pipefail
exec 9>/run/lock/mt7902-bluetooth.lock
flock 9
modprobe btusb_mt7902
for dev in /sys/bus/usb/devices/*; do
 [[ -f "$dev/idVendor" && -f "$dev/idProduct" ]] || continue
 [[ $(cat "$dev/idVendor") == 13d3 && $(cat "$dev/idProduct") == 3596 ]] || continue
 iface="$(basename "$dev"):1.0"
 [[ -e /sys/bus/usb/devices/$iface ]] || continue
 if [[ -L /sys/bus/usb/devices/$iface/driver ]]; then
  driver=$(basename "$(readlink -f "/sys/bus/usb/devices/$iface/driver")")
  [[ $driver == btusb_mt7902 ]] && continue
  [[ $driver == btusb ]] || { echo "Unexpected driver: $driver" >&2; exit 1; }
  printf '%s' "$iface" > /sys/bus/usb/drivers/btusb/unbind
 fi
 if ! printf '%s' "$iface" > /sys/bus/usb/drivers/btusb_mt7902/bind; then
  printf '%s' "$iface" > /sys/bus/usb/drivers/btusb/bind || true
  exit 1
 fi
done
