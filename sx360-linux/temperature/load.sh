#!/bin/sh
set -eu
cd "$(dirname "$0")"
if [ "$(id -u)" -ne 0 ]; then
    echo '请使用 sudo sh load.sh' >&2
    exit 1
fi
expected=$(modinfo -F vermagic ./zen5temp.ko | cut -d ' ' -f 1)
if [ "$expected" != "$(uname -r)" ]; then
    echo '模块与当前内核不匹配，请先执行 make 重新编译。' >&2
    exit 1
fi
if [ ! -d /sys/module/zen5temp ]; then
    insmod ./zen5temp.ko
fi
for sensor in /sys/class/hwmon/hwmon*; do
    if [ -r "$sensor/name" ] && [ "$(cat "$sensor/name")" = zen5temp ]; then
        echo "CPU 温度传感器：$sensor/temp1_input"
        awk '{printf "当前 Tctl：%.3f°C\n", $1 / 1000}' "$sensor/temp1_input"
        exit 0
    fi
done
echo '模块未成功注册温度传感器，请提供以下命令的输出：' >&2
echo 'sudo dmesg | tail -50' >&2
exit 1
