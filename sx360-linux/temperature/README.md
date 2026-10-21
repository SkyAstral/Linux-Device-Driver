# 9800X3D 温度桥接模块

目标为当前机器：AMD Family 1Ah / Model 44h，PCI root 1022:14d8，DF function 1022:14e3，单路主机 0000:00:18.3。
已在本机 Linux 5.15.0-139-generic 头文件下编译通过；**尚未加载或验证真实温度读数**，属于实验性模块。

## 依据

- 温度地址 0x59800 与 Tctl 换算逻辑依据 Linux v6.12 `drivers/hwmon/k10temp.c`：https://raw.githubusercontent.com/torvalds/linux/v6.12/drivers/hwmon/k10temp.c
- PCI 间接 SMN 访问依据 Linux v5.15 `arch/x86/kernel/amd_nb.c`：https://raw.githubusercontent.com/torvalds/linux/v5.15/arch/x86/kernel/amd_nb.c

当前内核的 AMD northbridge 缓存可能不识别此平台，因此仅给旧 k10temp 加设备 ID 不一定有效。本模块在已有匹配 northbridge 时使用内核 amd_smn_read 及其锁；无 northbridge 缓存时使用局部互斥锁访问已核对的 root PCI 地址选择寄存器 0x60，并从数据寄存器 0x64 读取温度、恢复原选择器。若已有不匹配的 northbridge 缓存则拒绝读取。

局部锁不能与其他直接访问 SMN 的用户态工具或外部驱动同步；测试时不要同时运行此类工具。它不写 SMN 数据、温度阈值、电压或风扇控制寄存器；但读取温度需要写 PCI 地址选择器。系统原装 k10temp 文件不被覆盖。

## 编译和测试加载

```bash
make
sudo sh load.sh
```

加载后应输出真实温度传感器路径及 Tctl。首次请对照 BIOS 或已知可信监控结果核对合理性。模块拒绝全零、全一或不在 (0,125]°C 的结果，不把失败伪装为 0°C。

如果提示 `Key was rejected by service`，是 Secure Boot 签名限制，需按本机已有 MOK 签名流程处理，不能强制加载。如果没有传感器，运行 `sudo dmesg | tail -50` 查看 probe 失败原因。

## 驱动屏幕

使用已经更新的 sx360.py：

```bash
cd ..
python3 sx360.py watch --dry-run --count 3
sudo python3 sx360.py watch --experimental-write
```

真实温度、CPU 使用率和平均频率每秒更新。Ctrl+C 停止屏幕刷新。

模块是临时加载，重启后不会自动加载；尚未安装 DKMS 或开机服务。当前内核版本以外需要重新编译。

## 卸载

先停止屏幕刷新，再执行：

```bash
sudo rmmod zen5temp
```

无需删除或恢复系统驱动文件。
