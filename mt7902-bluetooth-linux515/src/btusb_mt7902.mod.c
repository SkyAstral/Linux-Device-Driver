#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;
BUILD_LTO_INFO;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0x2c635209, "module_layout" },
	{ 0xe7a578c, "param_ops_bool" },
	{ 0xe180ce57, "usb_deregister" },
	{ 0x942410fb, "usb_register_driver" },
	{ 0x3ce4ca6f, "disable_irq" },
	{ 0xe2dc6d97, "usb_get_from_anchor" },
	{ 0xfcec0987, "enable_irq" },
	{ 0xce2840e7, "irq_set_irq_wake" },
	{ 0x71038ac7, "pv_ops" },
	{ 0x8427cc7b, "_raw_spin_lock_irq" },
	{ 0x87a21cb3, "__ubsan_handle_out_of_bounds" },
	{ 0xd4835ef8, "dmi_check_system" },
	{ 0x4116d286, "btrtl_shutdown_realtek" },
	{ 0x9355bcff, "btrtl_setup_realtek" },
	{ 0x3398f7af, "usb_set_interface" },
	{ 0x5374f3c5, "btintel_configure_setup" },
	{ 0xe7be0263, "btbcm_setup_apple" },
	{ 0x2aff387f, "usb_ifnum_to_if" },
	{ 0x577aba3d, "btbcm_set_bdaddr" },
	{ 0x71d05582, "btbcm_setup_patchram" },
	{ 0xce74f78d, "usb_match_id" },
	{ 0x31b360b9, "hci_register_dev" },
	{ 0x9cfbb611, "usb_driver_claim_interface" },
	{ 0xd09ecfb0, "gpiod_get_optional" },
	{ 0x4b4891f1, "hci_alloc_dev_priv" },
	{ 0xd9a5ea54, "__init_waitqueue_head" },
	{ 0xc3fcadfd, "devm_kmalloc" },
	{ 0xa0fbac79, "wake_up_bit" },
	{ 0xe65aecb1, "skb_clone" },
	{ 0xeae3dfd6, "__const_udelay" },
	{ 0xb43f9365, "ktime_get" },
	{ 0xc3055d20, "usleep_range_state" },
	{ 0x4071b517, "out_of_line_wait_on_bit_timeout" },
	{ 0x44bae227, "bit_wait_timeout" },
	{ 0x800473f, "__cond_resched" },
	{ 0xed221b0b, "__hci_cmd_send" },
	{ 0x69acdf38, "memcpy" },
	{ 0x28d384c6, "skb_put" },
	{ 0x10473ee, "__alloc_skb" },
	{ 0xc6d09aa9, "release_firmware" },
	{ 0x718b8b7, "bt_info" },
	{ 0x6aa31da7, "request_firmware" },
	{ 0x656e4a6e, "snprintf" },
	{ 0xa7b6ca7d, "usb_bulk_msg" },
	{ 0x6bd8bb45, "usb_enable_autosuspend" },
	{ 0x924eee22, "device_set_wakeup_capable" },
	{ 0x5abed3bb, "pm_runtime_forbid" },
	{ 0x56d7310c, "__pm_runtime_suspend" },
	{ 0xc97f87e5, "pm_runtime_allow" },
	{ 0x214e4265, "bt_warn" },
	{ 0x54b1fac6, "__ubsan_handle_load_invalid_value" },
	{ 0x3942155, "btintel_bootup" },
	{ 0x59706e47, "btintel_secure_send_result" },
	{ 0x807766ea, "usb_scuttle_anchored_urbs" },
	{ 0x2a8ca7d2, "usb_autopm_put_interface" },
	{ 0x3c12dfe, "cancel_work_sync" },
	{ 0xa3bb6e89, "hci_recv_frame" },
	{ 0xec5a5049, "hci_recv_diag" },
	{ 0xce947466, "usb_disable_autosuspend" },
	{ 0xf0311d7, "_dev_err" },
	{ 0x3d7e65af, "usb_control_msg" },
	{ 0x8d74cc4f, "usb_queue_reset_device" },
	{ 0xe9de6a72, "usb_autopm_get_interface" },
	{ 0xf437e5cc, "__hci_cmd_sync_ev" },
	{ 0xf9a482f9, "msleep" },
	{ 0x25fb07f8, "gpiod_set_value_cansleep" },
	{ 0xd0da656b, "__stack_chk_fail" },
	{ 0x6cea8936, "__hci_cmd_sync" },
	{ 0xf621d045, "usb_driver_release_interface" },
	{ 0xefb9f05f, "device_init_wakeup" },
	{ 0x6f72a4bb, "hci_free_dev" },
	{ 0xc90f248d, "gpiod_put" },
	{ 0x25a69299, "hci_unregister_dev" },
	{ 0x962c8ae1, "usb_kill_anchored_urbs" },
	{ 0xc5b6f236, "queue_work_on" },
	{ 0x2d3385d3, "system_wq" },
	{ 0x10bc6bea, "kfree_skb_reason" },
	{ 0x37a0cba, "kfree" },
	{ 0xd35cce70, "_raw_spin_unlock_irqrestore" },
	{ 0x34db050b, "_raw_spin_lock_irqsave" },
	{ 0xeb233a45, "__kmalloc" },
	{ 0x2f886acb, "usb_free_urb" },
	{ 0xaf88e69b, "kmem_cache_alloc_trace" },
	{ 0x30a93ed, "kmalloc_caches" },
	{ 0x56baf789, "usb_unanchor_urb" },
	{ 0x7b8c32f1, "bt_err" },
	{ 0x65487097, "__x86_indirect_thunk_rax" },
	{ 0x837b7b09, "__dynamic_pr_debug" },
	{ 0xb20c3797, "usb_submit_urb" },
	{ 0x6ebe366f, "ktime_get_mono_fast_ns" },
	{ 0x2cf05418, "usb_anchor_urb" },
	{ 0x437b7175, "usb_alloc_urb" },
	{ 0x5b8239ca, "__x86_return_thunk" },
	{ 0xbdfb6dbb, "__fentry__" },
};

MODULE_INFO(depends, "btrtl,btintel,btbcm,bluetooth");

MODULE_ALIAS("of:N*T*Cusb1286,204e");
MODULE_ALIAS("of:N*T*Cusb1286,204eC*");
MODULE_ALIAS("of:N*T*Cusbcf3,e300");
MODULE_ALIAS("of:N*T*Cusbcf3,e300C*");
MODULE_ALIAS("of:N*T*Cusb4ca,301a");
MODULE_ALIAS("of:N*T*Cusb4ca,301aC*");
MODULE_ALIAS("usb:v13D3p3596d*dc*dsc*dp*ic*isc*ip*in*");

MODULE_INFO(srcversion, "B0398F39429C8EC773CCB66");
