# MT7902 / Ubuntu 20.04 / Linux 5.15 实验驱动

目标电脑：Ubuntu 20.04.6 LTS，x86_64，5.15.0-139-generic；PCI 14c3:7902，子系统 1a3b:6040。

状态：源码已针对本机内核完成编译兼容移植。完整重编译（KCFLAGS=-Werror）及 MODPOST 符号版本检查通过。v0.1 实机已确认固件启动、接口出现，后续日志显示认证尝试，但添加 AP 的 STA 记录失败（-22）。v0.2 修复单 PHY 查找路径，尚待重新加载验证连接、传输和休眠。不能称为已验证可用驱动。

源自用户提供的 ../mt7902-backport，原始源码未修改。本目录保留其 Linux MT7902 实现，并适配 Linux 5.15 API；不是将 Windows .sys 转换成 Linux 模块。固件使用该 Linux 源码包自带的两份文件，未混用 Windows 包固件。

## 临时测试

先保存正在编辑的工作。在普通终端执行：

```sh
cd /home/starry/下载/mt7902-ubuntu2004-linux515
sudo sh test-load.sh
```

脚本只为 5.15.0-139-generic 和目标 MT7902 设计。拒绝与已加载的 MediaTek 无线模块混用，拒绝已绑定的设备，校验模块和固件。仅在对应系统固件缺失时补入；遇到已有不同固件或压缩固件就停止，不覆盖。

脚本临时加载 mt7902e.ko，测试关闭 PCI ASPM，等待目标接口出现并尝试列出热点，保存 test-日期-时间.log。不会安装模块到系统目录，不设置开机加载。新增的固件文件会保留。Secure Boot 若拒绝未签名模块，insmod 会报错；不要把签名失败当成硬件初始化失败。

出现无线接口不等于已连接。随后通过系统网络菜单选择热点，验证实际网络访问。请将测试日志交回以继续检查；日志可能包含热点名称及网络接口信息。

卸载：

```sh
sudo sh test-unload.sh
```

重启也会卸载临时模块。若此前实验留下设备状态且再次握手超时，可在保存日志后完整关机断电再测。

## 兼容范围与修改

- 只支持 Linux 5.15、PCIe MT7902，不包含蓝牙。
- 适配旧 mac80211 的站点/BSS 数据、单链路模型、回调签名、TX/RX 状态、NAPI 和 beacon API。
- 保留新源码的 MT7902 固件/MCU/DMA 实现。移除未编入模块的 Connac3 MAC 对象；MT7902 属于 Connac2。
- SoC WED 加速不可用；兼容层明确拒绝启用 CONFIG_NET_MEDIATEK_SOC_WED 的构建。
- 5.15 page-pool 使用 PP_FLAG_PAGE_FRAG，保留分片缓冲区回收与 DMA 映射。
- 使用旧版 HE capability 位定义，修复位值已编码导致的重复移位；扫描探测帧使用旧版 off-channel 标志。
- 初始版本仅开放 2.4/5 GHz，关闭 6 GHz 广播能力；不启用 EHT/MLO，不支持多 BSSID AP。
- 不自动进行 DKMS 安装。持久安装需先完成实机验证。

## 重编译和审查

```sh
make JOBS=4 KCFLAGS=-Werror
```

build-5.15.log：完整构建日志。
linux515.patch：相对原始 backport 的源码补丁。
SHA256SUMS：模块与固件校验。
README.upstream.md：上游原始说明，所述内核范围针对原始代码。

本次运行环境只允许工作目录写入，无法写系统模块/固件目录或加载内核模块，所以实机验证需由用户在本机终端执行以上命令。本目录尚未替换任何系统驱动。

## v0.2 修复与重新测试

已发现 mt76_vif_phy 在旧内核错误依赖新版多 radio 的信道上下文，可能返回 NULL 并导致 mt76_sta_state 返回 -EINVAL（-22），与实机失败日志一致。5.15 的本机驱动仅有一个 PHY，改为直接返回 hw->priv，也修复同一路径的发射功率查询。

旧模块仍在内存中，磁盘上的新模块不会自动替换它。执行：

```sh
cd /home/starry/下载/mt7902-ubuntu2004-linux515
sudo sh test-unload.sh
sudo sh test-load.sh
```

然后在系统网络菜单重新连接热点。旧版模块和日志备份在 previous-v0.1/。

## v0.3 卸载路径修复

v0.1 实机卸载出现 Oops，模块进入 Unloading/going，使用计数 -1。回溯为 mt76_dma_cleanup -> free_netdev -> dev_addr_flush -> __hw_addr_flush。
定位到原始 backport 中 alloc_netdev 的 setup 回调调用 init_dummy_netdev；Linux 5.15 的后者会 memset 整个 net_device，破坏前者已经初始化的地址链表、引用和分配偏移，也设置了与 free_netdev 不匹配的注册状态。
修正为保留 alloc_netdev 初始化，仅设置 NAPI 所需的 PRESENT/START 状态，保留 NETREG_UNINITIALIZED。v0.3 同时包含 v0.2 的单 PHY 修复。构建通过仍不等于卸载实测通过。
源码依据：https://raw.githubusercontent.com/torvalds/linux/v5.15/net/core/dev.c

当前内存中的 v0.1 已因 Oops 卡住，重复 rmmod 不能修复；不要强制卸载。保存工作后正常重启，然后只运行 test-load.sh 加载 v0.3，再验证扫描、连接。不要再次加载 previous-v0.1 或 previous-v0.2 的旧模块。
