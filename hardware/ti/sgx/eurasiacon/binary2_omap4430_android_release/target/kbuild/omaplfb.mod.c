#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
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
	{ 0x4d10e47d, "module_layout" },
	{ 0x12da5bb2, "__kmalloc" },
	{ 0x5934392b, "fb_register_client" },
	{ 0x4806518e, "__alloc_workqueue_key" },
	{ 0x632657bb, "fb_pan_display" },
	{ 0xf7802486, "__aeabi_uidivmod" },
	{ 0xc04af4ce, "queue_work" },
	{ 0x62b72b0d, "mutex_unlock" },
	{ 0xf69276c, "PVRGetDisplayClassJTable" },
	{ 0xe2d5255a, "strcmp" },
	{ 0xe707d823, "__aeabi_uidiv" },
	{ 0xfa2a45e, "__memzero" },
	{ 0xb5eeb329, "register_early_suspend" },
	{ 0xdc798d37, "__mutex_init" },
	{ 0xea147363, "printk" },
	{ 0x94d32a88, "__tracepoint_module_get" },
	{ 0x328a05f1, "strncpy" },
	{ 0x16305289, "warn_slowpath_null" },
	{ 0xe16b893b, "mutex_lock" },
	{ 0x8a32fc7, "destroy_workqueue" },
	{ 0x5fd00832, "module_put" },
	{ 0x434fa55c, "release_console_sem" },
	{ 0xf174ed48, "acquire_console_sem" },
	{ 0xdcacfbe7, "registered_fb" },
	{ 0xc27487dd, "__bug" },
	{ 0x97a85afb, "fb_set_var" },
	{ 0x37a0cba, "kfree" },
	{ 0x9d669763, "memcpy" },
	{ 0xcc36f32e, "fb_unregister_client" },
	{ 0xb227ae83, "unregister_early_suspend" },
	{ 0xe7b3665d, "fb_blank" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=pvrsrvkm";


MODULE_INFO(srcversion, "566464E1393A2FD2D67C4BF");
