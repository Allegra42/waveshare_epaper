#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

__visible struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x96978777, __VMLINUX_SYMBOL_STR(module_layout) },
	{ 0x73b6f6b7, __VMLINUX_SYMBOL_STR(cdev_del) },
	{ 0x405d9a68, __VMLINUX_SYMBOL_STR(class_destroy) },
	{ 0x65896a74, __VMLINUX_SYMBOL_STR(device_destroy) },
	{ 0x7485e15e, __VMLINUX_SYMBOL_STR(unregister_chrdev_region) },
	{ 0x2c5f18c8, __VMLINUX_SYMBOL_STR(platform_driver_unregister) },
	{ 0x8f59f1bd, __VMLINUX_SYMBOL_STR(uart_unregister_driver) },
	{ 0x68abb700, __VMLINUX_SYMBOL_STR(kobject_put) },
	{ 0x567648d2, __VMLINUX_SYMBOL_STR(gpiod_set_raw_value) },
	{ 0x2ff82fae, __VMLINUX_SYMBOL_STR(gpiod_direction_output_raw) },
	{ 0x60e34a36, __VMLINUX_SYMBOL_STR(gpio_to_desc) },
	{ 0x47229b5c, __VMLINUX_SYMBOL_STR(gpio_request) },
	{ 0xb84f5be9, __VMLINUX_SYMBOL_STR(device_create) },
	{ 0x8e865d3c, __VMLINUX_SYMBOL_STR(arm_delay_ops) },
	{ 0x6ffe713c, __VMLINUX_SYMBOL_STR(__class_create) },
	{ 0xf635c4e7, __VMLINUX_SYMBOL_STR(uart_register_driver) },
	{ 0x736808d7, __VMLINUX_SYMBOL_STR(__platform_driver_register) },
	{ 0xf5891015, __VMLINUX_SYMBOL_STR(cdev_add) },
	{ 0xbdbf795c, __VMLINUX_SYMBOL_STR(cdev_alloc) },
	{ 0x29537c9e, __VMLINUX_SYMBOL_STR(alloc_chrdev_region) },
	{ 0xfbc74f64, __VMLINUX_SYMBOL_STR(__copy_from_user) },
	{ 0xfa2a45e, __VMLINUX_SYMBOL_STR(__memzero) },
	{ 0xdb7305a1, __VMLINUX_SYMBOL_STR(__stack_chk_fail) },
	{ 0x67c2fa54, __VMLINUX_SYMBOL_STR(__copy_to_user) },
	{ 0x1cfb04fa, __VMLINUX_SYMBOL_STR(finish_wait) },
	{ 0x344b7739, __VMLINUX_SYMBOL_STR(prepare_to_wait_event) },
	{ 0x1000e51, __VMLINUX_SYMBOL_STR(schedule) },
	{ 0x8f678b07, __VMLINUX_SYMBOL_STR(__stack_chk_guard) },
	{ 0xf9dfa12b, __VMLINUX_SYMBOL_STR(uart_add_one_port) },
	{ 0x5f455ade, __VMLINUX_SYMBOL_STR(devm_ioremap_resource) },
	{ 0x424a45b, __VMLINUX_SYMBOL_STR(platform_get_resource) },
	{ 0x9051c611, __VMLINUX_SYMBOL_STR(devm_kmalloc) },
	{ 0x822137e2, __VMLINUX_SYMBOL_STR(arm_heavy_mb) },
	{ 0x27e1a049, __VMLINUX_SYMBOL_STR(printk) },
	{ 0x5fe52ae7, __VMLINUX_SYMBOL_STR(uart_remove_one_port) },
	{ 0x2e5810c6, __VMLINUX_SYMBOL_STR(__aeabi_unwind_cpp_pr1) },
	{ 0xb1ad28e0, __VMLINUX_SYMBOL_STR(__gnu_mcount_nc) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";

MODULE_ALIAS("of:N*T*Cti,omap3-uart*");
MODULE_ALIAS("of:N*T*Cti,am3352-uartti*");
MODULE_ALIAS("platform:omap3-uart");
MODULE_ALIAS("platform:am3352-uartti");
