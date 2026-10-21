/* PCIe-only compatibility: MT7902 has no MediaTek SoC WED engine. */
#ifndef MT7902_COMPAT515_H
#define MT7902_COMPAT515_H
#include <linux/version.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 15, 0) || LINUX_VERSION_CODE >= KERNEL_VERSION(5, 16, 0)
#error "This experimental port targets Linux 5.15 only"
#endif
#define ieee80211_link_sta ieee80211_sta
#ifndef __counted_by
#define __counted_by(member)
#endif
#include <net/page_pool.h>
#if IS_ENABLED(CONFIG_NET_MEDIATEK_SOC_WED)
#error "This 5.15 port only supports PCIe without SoC WED"
#endif
struct mtk_wed_device { struct { unsigned int token_start; } wlan; };
static inline bool mtk_wed_device_active(struct mtk_wed_device *wed) { return false; }
static inline bool mtk_wed_get_rx_capa(struct mtk_wed_device *wed) { return false; }
static inline void mtk_wed_device_detach(struct mtk_wed_device *wed) {}
static inline u32 mtk_wed_device_reg_read(struct mtk_wed_device *wed, u32 reg) { return 0; }
static inline void mtk_wed_device_reg_write(struct mtk_wed_device *wed, u32 reg, u32 val) {}
static inline void mtk_wed_device_irq_set_mask(struct mtk_wed_device *wed, u32 mask) {}
#ifndef IEEE80211_MLD_MAX_NUM_LINKS
#define IEEE80211_MLD_MAX_NUM_LINKS 1
#endif
#endif
