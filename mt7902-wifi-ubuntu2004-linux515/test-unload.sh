#!/bin/sh
set -eu
[ "$(id -u)" = 0 ] || { echo '请运行 sudo sh test-unload.sh'; exit 1; }
if [ -d /sys/module/mt7902e ]; then
 state=$(cat /sys/module/mt7902e/initstate)
 if [ "$state" != live ]; then
  echo "模块状态为 $state，不能继续卸载。请保存工作后正常重启。"
  exit 1
 fi
 version=$(cat /sys/module/mt7902e/version)
 case "$version" in
  0.1-linux515|0.2-linux515)
   echo "旧版 $version 的卸载兼容路径存在缺陷，请保存工作后正常重启，不再尝试 rmmod。"
   exit 1
   ;;
 esac
 rmmod mt7902e
fi
echo '实验模块已卸载；固件保留。'
