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
	{ 0x1c937e3f, "pci_unregister_driver" },
	{ 0x2554c13b, "__pci_register_driver" },
	{ 0xf0311d7, "_dev_err" },
	{ 0xec2dcbc3, "_dev_info" },
	{ 0x720b33b3, "devm_hwmon_device_register_with_info" },
	{ 0x81221cad, "amd_nb_num" },
	{ 0xbaceca03, "node_to_amd_nb" },
	{ 0xf5837f0a, "devm_add_action" },
	{ 0x7a71f7f, "pci_get_domain_bus_and_slot" },
	{ 0xcefb0c9f, "__mutex_init" },
	{ 0xc3fcadfd, "devm_kmalloc" },
	{ 0xd033aa56, "boot_cpu_data" },
	{ 0x54b1fac6, "__ubsan_handle_load_invalid_value" },
	{ 0xd0da656b, "__stack_chk_fail" },
	{ 0x65cd4d1a, "pci_write_config_dword" },
	{ 0x3213f038, "mutex_unlock" },
	{ 0xead99a33, "pci_read_config_dword" },
	{ 0x4dfa8d4b, "mutex_lock" },
	{ 0xfc37562, "amd_smn_read" },
	{ 0x9b565c56, "pci_dev_put" },
	{ 0x5b8239ca, "__x86_return_thunk" },
	{ 0xbdfb6dbb, "__fentry__" },
};

MODULE_INFO(depends, "");

MODULE_ALIAS("pci:v00001022d000014E3sv*sd*bc*sc*i*");

MODULE_INFO(srcversion, "94FE2169AF00F42AF90F25A");
