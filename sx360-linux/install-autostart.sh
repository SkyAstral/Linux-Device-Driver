#!/bin/sh
set -eu
cd "$(dirname "$0")"
if [ "$(id -u)" -ne 0 ]; then
    echo '请执行 sudo sh install-autostart.sh' >&2
    exit 1
fi
for file in sx360.py sx360-screen.service temperature/load.sh temperature/zen5temp.ko; do
    if [ ! -f "$file" ]; then
        echo "缺少文件：$file" >&2
        exit 1
    fi
done
expected=$(modinfo -F vermagic temperature/zen5temp.ko | cut -d ' ' -f 1)
if [ "$expected" != "$(uname -r)" ]; then
    echo '温度模块与当前内核不一致，请先执行 make -C temperature。' >&2
    exit 1
fi
# Reject a failed sensor probe before enabling a boot service.
sh temperature/load.sh
/usr/bin/python3 sx360.py preview >/dev/null
if systemctl is-active --quiet sx360-screen.service; then
    systemctl stop sx360-screen.service
fi
install -d -o root -g root -m 0755 /opt/sx360-linux/temperature
install -o root -g root -m 0644 sx360.py /opt/sx360-linux/sx360.py
install -o root -g root -m 0644 temperature/load.sh /opt/sx360-linux/temperature/load.sh
install -o root -g root -m 0644 temperature/zen5temp.ko /opt/sx360-linux/temperature/zen5temp.ko
install -o root -g root -m 0644 sx360-screen.service /etc/systemd/system/sx360-screen.service
systemctl daemon-reload
systemctl enable --now sx360-screen.service
sleep 2
if ! systemctl is-active --quiet sx360-screen.service; then
    systemctl status sx360-screen.service --no-pager || true
    echo '服务未正常运行，请运行 journalctl -u sx360-screen.service -n 40 --no-pager 查看日志。' >&2
    exit 1
fi
echo 'SX360 开机自启动已启用，当前已启动。'
echo '查看状态：systemctl status sx360-screen.service --no-pager'
echo '查看日志：journalctl -u sx360-screen.service -n 30 --no-pager'
