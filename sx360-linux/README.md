# SX360 Linux 屏显实验程序

## 开机自启动

用户已实机确认温度模块可用。新增 systemd 服务：开机先加载 zen5temp，再每秒刷新真实 CPU 温度、使用率和平均频率。USB 未就绪或意外断开时，失败后每 10 秒重试；无需登录桌面。

请停止之前终端里手动运行的 watch（Ctrl+C），然后执行：

```bash
sudo sh install-autostart.sh
```

安装脚本先验证内核版本和真实温度，再将程序及温度模块复制到 root 所有的 `/opt/sx360-linux`，服务写入 `/etc/systemd/system/sx360-screen.service`，启用并立即启动。以后无需保留“文档”目录中的运行副本，也不依赖 hidraw 编号。

```bash
systemctl status sx360-screen.service --no-pager
journalctl -u sx360-screen.service -n 30 --no-pager
sudo systemctl restart sx360-screen.service
sudo sh uninstall-autostart.sh
```

更新程序后重新执行安装脚本即可。温度模块仅针对编译时内核；若以后升级内核，需要重新编译 `make -C temperature` 并重新安装自启动。安装脚本和 unit 已静态检查，当前自动化环境没有管理员权限，未实际安装/启动服务。

适用于 Python 3.8+，无第三方依赖，不替换内核、系统库或 CoolerControl。
根据用户提供的“冰魄SX360水冷驱动.exe”中 TempComm.dll 静态分析实现。

**状态：用户已确认发送 45°C 后屏幕正确显示。** 支持识别设备、读取 CPU 温度、预览状态包、发送一次状态包和持续刷新。完整握手未还原，但此用户的设备已经验证直接发送状态包可生效。没有实现图片/GIF 上传、开机服务或风扇转速控制。其他型号/固件仍需独立验证。

## 定时刷新

已提供针对本机 5.15.0-139 内核编译的实验性温度模块，见 [temperature/README.md](temperature/README.md)。先在本机执行 `sudo sh temperature/load.sh` 并确认读数，再运行真实温度刷新。编译验证已完成，模块尚未实机加载验证。

默认每秒读取 CPU 温度、CPU 使用率和平均 CPU 频率，Ctrl+C 停止。持续模式自动寻找唯一匹配的 USB 屏幕，避免依赖易变的 hidraw 编号。

```bash
# 只读检查，真实温度传感器必须存在
python3 sx360.py watch --dry-run --count 3
# 每秒更新屏幕，真实温度传感器必须存在
sudo python3 sx360.py watch --experimental-write
# 固定值测试，45°C 并非实时温度
sudo python3 sx360.py watch --cpu-temp 45 --experimental-write
```

可通过 `--interval 2` 调整刷新间隔；`--count 10` 限定次数。可用 `--sensor /sys/class/hwmon/hwmonN/temp1_input` 指定已经确认属于 CPU 的毫摄氏度传感器文件，不能用 NVMe 温度代替 CPU 温度。设备断开、读取失败或数据异常时程序停止并报错。

当前机器为 Ryzen 7 9800X3D，5.15.0-139 内核的现有 k10temp 模块别名没有对应 PCI 1022:14e3，sysfs 只暴露 NVMe 和无温度输入的 asus 设备。自动真实温度刷新目前因此受阻；需要另行补充兼容的 CPU 温度驱动，不能仅安装 lm-sensors 解决。当前没有替换/加载任何内核模块。GPU 温度仍需手工指定。

已离线验证定时循环、读取温度单位转换、无效数值拒绝、短写异常及缺少传感器时不发送数据；新持续模式尚未实机运行。最新检查时执行环境没有暴露 /dev/hidraw4，硬件写入仍需在本机终端启动。

## 使用

在本目录执行：

```bash
python3 sx360.py probe
python3 sx360.py preview
python3 sx360.py preview --cpu-temp 45 --gpu-temp 40
```

`probe` 通过 sysfs 读取设备信息，不打开设备、不写入数据。`preview` 只生成数据包。
GPU 温度、使用率、频率和转速默认显示 0，可以显式传入；CPU 温度默认从 k10temp/coretemp/zenpower 获取。

确认实际硬件型号后，使用 probe 输出的设备路径进行一次实验：

```bash
sudo python3 sx360.py send --device /dev/hidrawN --cpu-temp 45 --experimental-write
```

将 `/dev/hidrawN` 换成实际路径。程序强制检查 VID/PID=0145:1001 和完整 HID 描述符。写入成功仅代表系统接受了数据，不代表屏幕生效；如果无变化，应继续分析握手，而不是盲目切换设备 ID 或模式。

此前只读测试识别到 `/dev/hidraw4`，描述符匹配；用户随后在本机终端验证了固定温度 45°C 可显示。当前系统未暴露 k10temp/coretemp CPU 温度传感器，需暂时使用 `--cpu-temp` 指定测试温度。数据包长度、大端频率及 RPM 编码已通过离线检查。日期字段的星期约定仍需实机核对。

程序无需安装。退出后没有后台进程；删除目录即可移除。一次实验发送的屏显状态可能会保留到设备重启或下一次更新。

## 复制到文档

当前自动化工具仅允许写入工作目录和 /tmp，所以项目创建于工作目录。请在本机终端执行：

```bash
cp -r /home/starry/下载/mt76/sx360-linux /home/starry/文档/
cd /home/starry/文档/sx360-linux
python3 sx360.py probe
```

协议证据见相邻 `screen-analysis/分析报告.md`。尚未安装系统服务或修改设备权限。
