#!/bin/sh
set -eu
if [ "$(id -u)" -ne 0 ]; then
    echo '请执行 sudo sh uninstall-autostart.sh' >&2
    exit 1
fi
if [ -f /etc/systemd/system/sx360-screen.service ]; then
    systemctl disable --now sx360-screen.service
    rm -f /etc/systemd/system/sx360-screen.service
    systemctl daemon-reload
fi
echo '开机自启动已移除。/opt/sx360-linux 保留；温度模块保留至卸载或重启。'
echo '如不再需要温度模块：sudo rmmod zen5temp'
