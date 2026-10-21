# 本机 MT7902 蓝牙实验适配包

目标：Ubuntu 20.04.6 / Linux 5.15.0-139-generic，USB 13d3:3596。

状态：编译通过、符号解析通过、内核 vermagic 匹配；尚未加载到硬件，不能认定蓝牙已修复。

## 分析

Windows 包 INF 版本 1.1038.0.440，日期 2024-06-28，明确将 13d3:3596 映射到 MTKBT_A2_1。该映射本身不能单独证明芯片型号；MT7902 判断还结合本机既有 MT7902 Wi-Fi 适配环境。测试驱动通过 MediaTek 寄存器读取实际芯片 ID，若不是支持的型号则拒绝初始化。

Windows .sys 不能直接转换为 Linux .ko。本包使用官方 Linux v5.15 btusb 源码，复用其 MediaTek WMT 下载协议，并携带 Windows 包中的 MT7902 固件，未执行任何 Windows 程序。

原 Linux v5.15 源码缺少 13d3:3596 专用 MediaTek 匹配和 0x7902 芯片分支。实验改动加入这两处支持；保留读取实际芯片/固件版本；加入固件分段边界检查、固件加载失败处理、下载状态及重试超时处理，并修正段描述复制长度。模块 USB 驱动名称独立，只匹配目标设备。协议复用是否适合实际 MT7902 硬件仍需测试。

固件头时间：20240629120951，大小 512642 字节，3 个段，两个非空段均通过文件边界校验。系统原有同名固件 SHA256：4f53b5e02fbd933172e18caf952bda410a877eaad00561781528cb4aff58dc38。本包固件 SHA256：782f6927ea520535013606dcac7b423e4ed28a52de56bd741ca91b96f11c25f8。

## 本机测试

在本目录运行：

```bash
sudo bash ./test.sh
```

脚本备份系统同名固件，临时加载模块，仅重新绑定目标 USB 接口，并输出控制器和内核日志；日志保存在 test-kernel.log。会中断目标蓝牙接口，不会设置开机加载。sudo 密码仅在你自己的终端输入。

成功标准：日志中实际芯片 ID 与固件加载成功，bluetoothctl list 出现控制器，hciconfig 显示有效蓝牙地址；随后还需开启蓝牙、实际扫描与连接验证。编译成功不代表这些检查已通过。

## 回滚

```bash
sudo bash ./rollback.sh
```

恢复原固件，解除实验驱动绑定并重新绑定原 btusb。固件已发送到设备 RAM 时，必要时完全关机断电后再启动。

## 重新编译

```bash
make -C src -j4
cp src/btusb_mt7902.ko ./btusb_mt7902.ko
```

本包绑定当前内核；升级内核后须重新评估接口与编译。Secure Boot 当前关闭。本包不是经过厂家认证的驱动。

官方源码来源：https://github.com/torvalds/linux/tree/v5.15/drivers/bluetooth
upstream/ 为原始文件，src/ 为修改后的源码，linux515.patch 为补丁。源码沿用原 GPL 许可；固件来自用户本机 Windows 驱动包，未改变其授权。

## 永久安装（2026-10-13）

用户已报告实验驱动成功。读取本机确认芯片 ID 00007902、控制器 50:2E:91:56:36:77 且 Powered=yes；Wi-Fi 当前在信道 149 / 5745 MHz。

运行 `sudo bash ./install-permanent.sh` 完成管理员安装。脚本注册 DKMS，配置 systemd 开机绑定与 udev 热插拔触发，仅操作 13d3:3596 的接口 0；不屏蔽通用 btusb，不修改 Wi-Fi 驱动。当前已绑定实验驱动时不重新绑定。DKMS 在内核升级时尝试重编译，但不保证兼容未来内核 API。

永久安装后使用 `sudo bash /var/lib/mt7902-bluetooth/uninstall.sh` 卸载，勿使用早期临时测试的 rollback.sh。原固件备份保存在 /var/lib/mt7902-bluetooth。
