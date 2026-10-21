#!/bin/bash
set -euo pipefail
cd -- "$(dirname -- "${BASH_SOURCE[0]}")"
[[ $EUID == 0 ]] || { echo '请运行 sudo bash ./install.sh'; exit 1; }
command -v dkms >/dev/null
[[ -e /lib/modules/$(uname -r)/build/Makefile ]] || { echo '缺少当前内核头文件'; exit 1; }
backup=/var/lib/mt7902-bluetooth
if [[ ! -e $backup ]]; then
 install -d -m 700 "$backup"
 fw=/lib/firmware/mediatek/BT_RAM_CODE_MT7902_1_1_hdr.bin
 if [[ -e state/firmware.original ]]; then
  cp -a state/firmware.original "$backup/firmware.original"
 elif [[ -e state/firmware.absent ]]; then
  touch "$backup/firmware.absent"
 elif [[ -e $fw ]]; then
  cp -a "$fw" "$backup/firmware.original"
 else
  touch "$backup/firmware.absent"
 fi
fi
install -d /usr/src/btusb-mt7902-0.1
install -m 644 src/btusb_mt7902.c src/btintel.h src/btbcm.h src/btrtl.h src/Makefile dkms.conf /usr/src/btusb-mt7902-0.1/
if ! dkms status -m btusb-mt7902 -v 0.1 | grep -q .; then
 dkms add -m btusb-mt7902 -v 0.1
fi
dkms install -m btusb-mt7902 -v 0.1 -k "$(uname -r)"
install -m 644 firmware/BT_RAM_CODE_MT7902_1_1_hdr.bin /lib/firmware/mediatek/
install -m 755 bind-mt7902.sh /usr/local/sbin/bind-mt7902-bluetooth
install -m 644 mt7902-bluetooth.service /etc/systemd/system/
install -m 644 71-mt7902-bluetooth.rules /etc/udev/rules.d/
printf '%s\n' 'options btusb_mt7902 enable_autosuspend=0' > /etc/modprobe.d/btusb-mt7902.conf
install -m 755 uninstall.sh "$backup/uninstall.sh"
depmod -a
systemctl daemon-reload
udevadm control --reload-rules
systemctl enable mt7902-bluetooth.service
systemctl start mt7902-bluetooth.service
systemctl enable bluetooth.service
echo '永久配置已安装；当前工作中的蓝牙不会重新绑定。'
echo '检查：dkms status; systemctl status mt7902-bluetooth.service; bluetoothctl show'
echo '卸载：sudo bash /var/lib/mt7902-bluetooth/uninstall.sh'
echo '内核升级时 DKMS 会尝试重编译；若新内核接口不兼容，需要更新适配源码。'
