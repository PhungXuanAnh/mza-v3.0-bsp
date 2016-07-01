/*
 * linux/arch/arm/mach-omap2/board-omap3beagle.c
 *
 * Copyright (C) 2008 Texas Instruments
 *
 * Modified from mach-omap2/board-3430sdp.c
 *
 * Initial code: Syed Mohammed Khasim
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/leds.h>
#include <linux/gpio.h>
#include <linux/input.h>
#include <linux/gpio_keys.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/mtd/nand.h>
#include <linux/mmc/host.h>

#include <linux/usb/android_composite.h>

#include <linux/regulator/machine.h>
#include <linux/i2c/twl.h>

#include <mach/hardware.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/flash.h>

#include <plat/board.h>
#include <plat/common.h>
#include <plat/display.h>
#include <plat/gpmc.h>
#include <plat/nand.h>
#include <plat/usb.h>

#include "mux.h"
#include "hsmmc.h"
#include "timer-gp.h"
#include "board-flash.h"
#ifdef CONFIG_MACH_OMAP3_PANTHER
#include "pm34xx.h"
#endif

#if defined(CONFIG_MACH_OMAP3_PANTHER) && defined(CONFIG_WL12XX_PLATFORM_DATA)
#include <linux/wl12xx.h>
#include <linux/regulator/fixed.h>
#define BEAGLE_WLAN_PMENA_GPIO       16
#define BEAGLE_WLAN_IRQ_GPIO         112
#endif

#if defined(CONFIG_MACH_OMAP3_PANTHER) && defined(CONFIG_TI_ST)
#include <linux/ti_wilink_st.h>
#define BEAGLE_BTEN_GPIO       15
#endif

#define NAND_BLOCK_SIZE		SZ_128K

#ifdef CONFIG_USB_ANDROID

#define GOOGLE_VENDOR_ID		0x18d1
#define GOOGLE_PRODUCT_ID		0x9018
#define GOOGLE_ADB_PRODUCT_ID		0x9015

//add by AnhPX
#define INT_TSC 115

static char *usb_functions_adb[] = {
	"adb",
};

static char *usb_functions_mass_storage[] = {
	"usb_mass_storage",
};
static char *usb_functions_ums_adb[] = {
	"usb_mass_storage",
	"adb",
};

static char *usb_functions_all[] = {
	"adb", "usb_mass_storage",
};

static struct android_usb_product usb_products[] = {
	{
		.product_id	= GOOGLE_PRODUCT_ID,
		.num_functions	= ARRAY_SIZE(usb_functions_adb),
		.functions	= usb_functions_adb,
	},
	{
		.product_id	= GOOGLE_PRODUCT_ID,
		.num_functions	= ARRAY_SIZE(usb_functions_mass_storage),
		.functions	= usb_functions_mass_storage,
	},
	{
		.product_id	= GOOGLE_PRODUCT_ID,
		.num_functions	= ARRAY_SIZE(usb_functions_ums_adb),
		.functions	= usb_functions_ums_adb,
	},
};

static struct usb_mass_storage_platform_data mass_storage_pdata = {
	.nluns		= 1,
	.vendor		= "rowboat",
	.product	= "rowboat gadget",
	.release	= 0x100,
};

static struct platform_device usb_mass_storage_device = {
	.name	= "usb_mass_storage",
	.id	= -1,
	.dev	= {
		.platform_data = &mass_storage_pdata,
	},
};

static struct android_usb_platform_data android_usb_pdata = {
	.vendor_id	= GOOGLE_VENDOR_ID,
	.product_id	= GOOGLE_PRODUCT_ID,
	.functions	= usb_functions_all,
	.products	= usb_products,
	.num_products	= ARRAY_SIZE(usb_products),
	.version	= 0x0100,
	.product_name	= "rowboat gadget",
	.manufacturer_name	= "rowboat",
	.serial_number	= "20100720",
	.num_functions	= ARRAY_SIZE(usb_functions_all),
};

static struct platform_device androidusb_device = {
	.name	= "android_usb",
	.id	= -1,
	.dev	= {
		.platform_data = &android_usb_pdata,
	},
};

static void omap3beagle_android_gadget_init(void)
{
	platform_device_register(&androidusb_device);
}
#endif
/*
 * OMAP3 Beagle revision
 * Run time detection of Beagle revision is done by reading GPIO.
 * GPIO ID -
 *	AXBX	= GPIO173, GPIO172, GPIO171: 1 1 1
 *	C1_3	= GPIO173, GPIO172, GPIO171: 1 1 0
 *	C4	= GPIO173, GPIO172, GPIO171: 1 0 1
 *	XM	= GPIO173, GPIO172, GPIO171: 0 0 0
 */
enum {
	OMAP3BEAGLE_BOARD_UNKN = 0,
	OMAP3BEAGLE_BOARD_AXBX,
	OMAP3BEAGLE_BOARD_C1_3,
	OMAP3BEAGLE_BOARD_C4,
	OMAP3BEAGLE_BOARD_XM,
	OMAP3BEAGLE_BOARD_XMC,
};

extern void omap_pm_sys_offmode_select(int);
extern void omap_pm_sys_offmode_pol(int);
extern void omap_pm_sys_clkreq_pol(int);
extern void omap_pm_auto_off(int);
extern void omap_pm_auto_ret(int);

static u8 omap3_beagle_version;

static u8 omap3_beagle_get_rev(void)
{
	return omap3_beagle_version;
}

/**
 * Board specific initialization of PM components
 */
static void __init omap3_beagle_pm_init(void)
{
	/* Use sys_offmode signal */
	omap_pm_sys_offmode_select(1);

	/* sys_clkreq - active high */
	omap_pm_sys_clkreq_pol(1);

	/* sys_offmode - active low */
	omap_pm_sys_offmode_pol(0);

	/* Automatically send OFF command */
	omap_pm_auto_off(1);

	/* Automatically send RET command */
	omap_pm_auto_ret(1);

#ifdef CONFIG_MACH_OMAP3_PANTHER
    /*initialize sleep relational register for PM components*/
    omap3_pm_prm_polctrl_set(1,0);
    omap3_pm_prm_voltctrl_set(1,1,1);
#endif
}

static void __init omap3_beagle_init_rev(void)
{
	int ret;
	u16 beagle_rev = 0;

	omap_mux_init_gpio(171, OMAP_PIN_INPUT_PULLUP);
	omap_mux_init_gpio(172, OMAP_PIN_INPUT_PULLUP);
	omap_mux_init_gpio(173, OMAP_PIN_INPUT_PULLUP);

	ret = gpio_request(171, "rev_id_0");
	if (ret < 0)
		goto fail0;

	ret = gpio_request(172, "rev_id_1");
	if (ret < 0)
		goto fail1;

	ret = gpio_request(173, "rev_id_2");
	if (ret < 0)
		goto fail2;

	gpio_direction_input(171);
	gpio_direction_input(172);
	gpio_direction_input(173);

	beagle_rev = gpio_get_value(171) | (gpio_get_value(172) << 1)
			| (gpio_get_value(173) << 2);

	switch (beagle_rev) {
	case 7:
		printk(KERN_INFO "OMAP3 Beagle Rev: Ax/Bx\n");
		omap3_beagle_version = OMAP3BEAGLE_BOARD_AXBX;
		break;
	case 6:
		printk(KERN_INFO "OMAP3 Beagle Rev: C1/C2/C3\n");
		omap3_beagle_version = OMAP3BEAGLE_BOARD_C1_3;
		break;
	case 5:
		printk(KERN_INFO "OMAP3 Beagle Rev: C4\n");
		omap3_beagle_version = OMAP3BEAGLE_BOARD_C4;
		break;
	case 2:
		printk(KERN_INFO "OMAP3 Beagle Rev: xM C\n");
		omap3_beagle_version = OMAP3BEAGLE_BOARD_XMC;
		break;
	case 0:
		printk(KERN_INFO "OMAP3 Beagle Rev: xM\n");
		omap3_beagle_version = OMAP3BEAGLE_BOARD_XM;
		break;
	default:
		printk(KERN_INFO "OMAP3 Beagle Rev: unknown %hd\n", beagle_rev);
		omap3_beagle_version = OMAP3BEAGLE_BOARD_UNKN;
	}

	return;

fail2:
	gpio_free(172);
fail1:
	gpio_free(171);
fail0:
	printk(KERN_ERR "Unable to get revision detection GPIO pins\n");
	omap3_beagle_version = OMAP3BEAGLE_BOARD_UNKN;

	return;
}

static struct mtd_partition omap3beagle_nand_partitions[] = {
	/* All the partition sizes are listed in terms of NAND block size */
	{
		.name		= "X-Loader",
		.offset		= 0,
		.size		= 4 * NAND_BLOCK_SIZE,
		.mask_flags	= MTD_WRITEABLE,	/* force read-only */
	},
	{
		.name		= "U-Boot",
		.offset		= MTDPART_OFS_APPEND,	/* Offset = 0x80000 */
#ifndef CONFIG_MACH_OMAP3_PANTHER
		.size		= 15 * NAND_BLOCK_SIZE,
#else
		.size		= 10 * NAND_BLOCK_SIZE,
#endif
		.mask_flags	= MTD_WRITEABLE,	/* force read-only */
	},
	{
		.name		= "U-Boot Env",
		.offset		= MTDPART_OFS_APPEND,	/* Offset = 0x260000 */
#ifndef CONFIG_MACH_OMAP3_PANTHER
		.size		= 1 * NAND_BLOCK_SIZE,
#else
		.size		= 6 * NAND_BLOCK_SIZE,
#endif
	},
	{
		.name		= "Kernel",
		.offset		= MTDPART_OFS_APPEND,	/* Offset = 0x280000 */
#ifndef CONFIG_MACH_OMAP3_PANTHER
		.size		= 32 * NAND_BLOCK_SIZE,
#else
		.size		= 40 * NAND_BLOCK_SIZE,
#endif
	},
	{
		.name		= "File System",
		.offset		= MTDPART_OFS_APPEND,	/* Offset = 0x680000 */
		.size		= MTDPART_SIZ_FULL,
	},
};

/* DSS */

static int beagle_enable_dvi(struct omap_dss_device *dssdev)
{
	if (gpio_is_valid(dssdev->reset_gpio))
		gpio_set_value(dssdev->reset_gpio, 1);

	return 0;
}

static void beagle_disable_dvi(struct omap_dss_device *dssdev)
{
	if (gpio_is_valid(dssdev->reset_gpio))
		gpio_set_value(dssdev->reset_gpio, 0);
}

static struct omap_dss_device beagle_dvi_device = {
	.type = OMAP_DISPLAY_TYPE_DPI,
	.name = "dvi",
	.driver_name = "generic_panel",
	.phy.dpi.data_lines = 24,
	.reset_gpio = -EINVAL,
	.platform_enable = beagle_enable_dvi,
	.platform_disable = beagle_disable_dvi,
};

static struct omap_dss_device beagle_tv_device = {
	.name = "tv",
	.driver_name = "venc",
	.type = OMAP_DISPLAY_TYPE_VENC,
	.phy.venc.type = OMAP_DSS_VENC_TYPE_SVIDEO,
};

static struct omap_dss_device *beagle_dss_devices[] = {
	&beagle_dvi_device,
	&beagle_tv_device,
};

static struct omap_dss_board_info beagle_dss_data = {
	.num_devices = ARRAY_SIZE(beagle_dss_devices),
	.devices = beagle_dss_devices,
	.default_device = &beagle_dvi_device,
};

static struct platform_device beagle_dss_device = {
	.name          = "omapdss",
	.id            = -1,
	.dev            = {
		.platform_data = &beagle_dss_data,
	},
};

static struct regulator_consumer_supply beagle_vdac_supply =
	REGULATOR_SUPPLY("vdda_dac", "omapdss");

static struct regulator_consumer_supply beagle_vdvi_supply =
	REGULATOR_SUPPLY("vdds_dsi", "omapdss");

static void __init beagle_display_init(void)
{
	int r;

#ifdef CONFIG_MACH_OMAP3_PANTHER
    beagle_dvi_device.reset_gpio = 129;
    omap_mux_init_gpio(beagle_dvi_device.reset_gpio, OMAP_PIN_INPUT);
#endif

	r = gpio_request(beagle_dvi_device.reset_gpio, "DVI reset");
	if (r < 0) {
		printk(KERN_ERR "Unable to get DVI reset GPIO\n");
		return;
	}

	gpio_direction_output(beagle_dvi_device.reset_gpio, 0);
}


#include "sdram-micron-mt46h32m32lf-6.h"

static struct omap2_hsmmc_info mmc[] = {
	{
		.mmc		= 1,
		.caps		= MMC_CAP_4_BIT_DATA | MMC_CAP_8_BIT_DATA,
		.gpio_wp	= 29,
	},
#if defined(CONFIG_MACH_OMAP3_PANTHER) && defined(CONFIG_WL12XX_PLATFORM_DATA)
    {
        .name           = "wl1271",
        .mmc            = 2,
        .caps           = MMC_CAP_4_BIT_DATA | MMC_CAP_POWER_OFF_CARD,
        .gpio_wp        = -EINVAL,
        .gpio_cd        = -EINVAL,
        .nonremovable   = true,
    },
#endif
	{}	/* Terminator */
};

static struct regulator_consumer_supply beagle_vmmc1_supply = {
	.supply			= "vmmc",
};

#ifndef CONFIG_MACH_OMAP3_PANTHER
static struct regulator_consumer_supply beagle_vsim_supply = {
	.supply			= "vmmc_aux",
};
#endif

static struct regulator_consumer_supply beagle_vaux3_supply = {
	.supply         = "cam_1v8",
};

static struct regulator_consumer_supply beagle_vaux4_supply = {
	.supply         = "cam_2v8",
};

static struct gpio_led gpio_leds[];

static int beagle_twl_gpio_setup(struct device *dev,
		unsigned gpio, unsigned ngpio)
{
#ifndef CONFIG_MACH_OMAP3_PANTHER
	if (omap3_beagle_get_rev() == OMAP3BEAGLE_BOARD_XM || omap3_beagle_get_rev() == OMAP3BEAGLE_BOARD_XMC) {
		mmc[0].gpio_wp = -EINVAL;
	} else if ((omap3_beagle_get_rev() == OMAP3BEAGLE_BOARD_C1_3) ||
		(omap3_beagle_get_rev() == OMAP3BEAGLE_BOARD_C4)) {
		omap_mux_init_gpio(23, OMAP_PIN_INPUT);
		mmc[0].gpio_wp = 23;
	} else {
		omap_mux_init_gpio(29, OMAP_PIN_INPUT);
	}
#else
    mmc[0].gpio_wp = -EINVAL;
#endif

	/* gpio + 0 is "mmc0_cd" (input/IRQ) */
	mmc[0].gpio_cd = gpio + 0;
	omap2_hsmmc_init(mmc);

	/* link regulators to MMC adapters */
	beagle_vmmc1_supply.dev = mmc[0].dev;
#ifndef CONFIG_MACH_OMAP3_PANTHER
	beagle_vsim_supply.dev = mmc[0].dev;

	/* REVISIT: need ehci-omap hooks for external VBUS
	 * power switch and overcurrent detect
	 */
	if (omap3_beagle_get_rev() != OMAP3BEAGLE_BOARD_XM || omap3_beagle_get_rev() == OMAP3BEAGLE_BOARD_XMC) {
		gpio_request(gpio + 1, "EHCI_nOC");
		gpio_direction_input(gpio + 1);
	}
#endif

	/*
	 * TWL4030_GPIO_MAX + 0 == ledA, EHCI nEN_USB_PWR (out, XM active
	 * high / others active low)
	 */
	gpio_request(gpio + TWL4030_GPIO_MAX, "nEN_USB_PWR");
#ifndef CONFIG_MACH_OMAP3_PANTHER
	gpio_direction_output(gpio + TWL4030_GPIO_MAX, 0);
	if (omap3_beagle_get_rev() == OMAP3BEAGLE_BOARD_XM)
		gpio_direction_output(gpio + TWL4030_GPIO_MAX, 1);
	else
		gpio_direction_output(gpio + TWL4030_GPIO_MAX, 0);
#else
	gpio_direction_output(gpio + TWL4030_GPIO_MAX, 1);
#endif

#ifndef CONFIG_MACH_OMAP3_PANTHER
	gpio_direction_output(gpio + TWL4030_GPIO_MAX, 0);
	/* DVI reset GPIO is different between beagle revisions */
	if (omap3_beagle_get_rev() == OMAP3BEAGLE_BOARD_XM || omap3_beagle_get_rev() == OMAP3BEAGLE_BOARD_XMC)
		beagle_dvi_device.reset_gpio = 129;
	else
		beagle_dvi_device.reset_gpio = 170;
#else
    gpio_request(gpio + 1, "nDVI_PWR_EN");
    gpio_direction_output(gpio + 1, 0);
#endif

#ifndef CONFIG_MACH_OMAP3_PANTHER
	if (omap3_beagle_get_rev() == OMAP3BEAGLE_BOARD_XM) {
		/* Power on camera interface */
		gpio_request(gpio + 2, "CAM_EN");
		gpio_direction_output(gpio + 2, 1);

		/* TWL4030_GPIO_MAX + 0 == ledA, EHCI nEN_USB_PWR (out, active low) */
		gpio_request(gpio + TWL4030_GPIO_MAX, "nEN_USB_PWR");
		gpio_direction_output(gpio + TWL4030_GPIO_MAX, 1);
	} else {
		gpio_request(gpio + 1, "EHCI_nOC");
		gpio_direction_input(gpio + 1);

		/* TWL4030_GPIO_MAX + 0 == ledA, EHCI nEN_USB_PWR (out, active low) */
		gpio_request(gpio + TWL4030_GPIO_MAX, "nEN_USB_PWR");
		gpio_direction_output(gpio + TWL4030_GPIO_MAX, 0);
	}
	/* TWL4030_GPIO_MAX + 1 == ledB, PMU_STAT (out, active low LED) */
	gpio_leds[2].gpio = gpio + TWL4030_GPIO_MAX + 1;

	/*
	 * gpio + 1 on Xm controls the TFP410's enable line (active low)
	 * gpio + 2 control varies depending on the board rev as follows:
	 * P7/P8 revisions(prototype): Camera EN
	 * A2+ revisions (production): LDO (supplies DVI, serial, led blocks)
	 */
	if (omap3_beagle_get_rev() == OMAP3BEAGLE_BOARD_XM || omap3_beagle_get_rev() == OMAP3BEAGLE_BOARD_XMC) {
		gpio_request(gpio + 1, "nDVI_PWR_EN");
		gpio_direction_output(gpio + 1, 0);
		gpio_request(gpio + 2, "DVI_LDO_EN");
		gpio_direction_output(gpio + 2, 1);
	}
#endif

	return 0;
}

static struct twl4030_gpio_platform_data beagle_gpio_data = {
	.gpio_base	= OMAP_MAX_GPIO_LINES,
	.irq_base	= TWL4030_GPIO_IRQ_BASE,
	.irq_end	= TWL4030_GPIO_IRQ_END,
	.use_leds	= true,
	.pullups	= BIT(1),
	.pulldowns	= BIT(2) | BIT(6) | BIT(7) | BIT(8) | BIT(13)
				| BIT(15) | BIT(16) | BIT(17),
	.setup		= beagle_twl_gpio_setup,
};

/* VMMC1 for MMC1 pins CMD, CLK, DAT0..DAT3 (20 mA, plus card == max 220 mA) */
static struct regulator_init_data beagle_vmmc1 = {
	.constraints = {
		.min_uV			= 1850000,
		.max_uV			= 3150000,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL
					| REGULATOR_MODE_STANDBY,
		.valid_ops_mask		= REGULATOR_CHANGE_VOLTAGE
					| REGULATOR_CHANGE_MODE
					| REGULATOR_CHANGE_STATUS,
	},
	.num_consumer_supplies	= 1,
	.consumer_supplies	= &beagle_vmmc1_supply,
};

#ifndef CONFIG_MACH_OMAP3_PANTHER
/* VSIM for MMC1 pins DAT4..DAT7 (2 mA, plus card == max 50 mA) */
static struct regulator_init_data beagle_vsim = {
	.constraints = {
		.min_uV			= 1800000,
		.max_uV			= 3000000,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL
					| REGULATOR_MODE_STANDBY,
		.valid_ops_mask		= REGULATOR_CHANGE_VOLTAGE
					| REGULATOR_CHANGE_MODE
					| REGULATOR_CHANGE_STATUS,
	},
	.num_consumer_supplies	= 1,
	.consumer_supplies	= &beagle_vsim_supply,
};
#endif

/* VDAC for DSS driving S-Video (8 mA unloaded, max 65 mA) */
static struct regulator_init_data beagle_vdac = {
	.constraints = {
		.min_uV			= 1800000,
		.max_uV			= 1800000,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL
					| REGULATOR_MODE_STANDBY,
		.valid_ops_mask		= REGULATOR_CHANGE_MODE
					| REGULATOR_CHANGE_STATUS,
	},
	.num_consumer_supplies	= 1,
	.consumer_supplies	= &beagle_vdac_supply,
};

/* VPLL2 for digital video outputs */
static struct regulator_init_data beagle_vpll2 = {
	.constraints = {
		.name			= "VDVI",
		.min_uV			= 1800000,
		.max_uV			= 1800000,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL
					| REGULATOR_MODE_STANDBY,
		.valid_ops_mask		= REGULATOR_CHANGE_MODE
					| REGULATOR_CHANGE_STATUS,
	},
	.num_consumer_supplies	= 1,
	.consumer_supplies	= &beagle_vdvi_supply,
};

/* VAUX3 for CAM_1V8 */
static struct regulator_init_data beagle_vaux3 = {
	.constraints = {
		.min_uV                 = 1800000,
		.max_uV                 = 1800000,
		.apply_uV               = true,
		.valid_modes_mask       = REGULATOR_MODE_NORMAL
					| REGULATOR_MODE_STANDBY,
		.valid_ops_mask         = REGULATOR_CHANGE_MODE
					| REGULATOR_CHANGE_STATUS,
	},
	.num_consumer_supplies  = 1,
	.consumer_supplies      = &beagle_vaux3_supply,
};

 /* VAUX4 for CAM_2V8 */
static struct regulator_init_data beagle_vaux4 = {
	.constraints = {
		.min_uV                 = 1800000,
		.max_uV                 = 1800000,
		.apply_uV               = true,
		.valid_modes_mask       = REGULATOR_MODE_NORMAL
			| REGULATOR_MODE_STANDBY,
		.valid_ops_mask         = REGULATOR_CHANGE_MODE
			| REGULATOR_CHANGE_STATUS,
	},
	.num_consumer_supplies  = 1,
	.consumer_supplies      = &beagle_vaux4_supply,
};

static struct twl4030_usb_data beagle_usb_data = {
	.usb_mode	= T2_USB_MODE_ULPI,
};

/**
 * Macro to configure resources
 */
#define TWL4030_RESCONFIG(res,grp,typ1,typ2,state)	\
	{						\
		.resource	= res,			\
		.devgroup	= grp,			\
		.type		= typ1,			\
		.type2		= typ2,			\
		.remap_sleep	= state			\
	}

static struct twl4030_resconfig  __initdata board_twl4030_rconfig[] = {
	TWL4030_RESCONFIG(RES_VPLL1, DEV_GRP_P1, 3, 1, RES_STATE_OFF),		/* ? */
	TWL4030_RESCONFIG(RES_VINTANA1, DEV_GRP_ALL, 1, 2, RES_STATE_SLEEP),
	TWL4030_RESCONFIG(RES_VINTANA2, DEV_GRP_ALL, 0, 2, RES_STATE_SLEEP),
	TWL4030_RESCONFIG(RES_VINTDIG, DEV_GRP_ALL, 1, 2, RES_STATE_SLEEP),
	TWL4030_RESCONFIG(RES_VIO, DEV_GRP_ALL, 2, 2, RES_STATE_SLEEP),
	TWL4030_RESCONFIG(RES_VDD1, DEV_GRP_P1, 4, 1, RES_STATE_OFF),		/* ? */
	TWL4030_RESCONFIG(RES_VDD2, DEV_GRP_P1, 3, 1, RES_STATE_OFF),		/* ? */
	TWL4030_RESCONFIG(RES_REGEN, DEV_GRP_ALL, 2, 1, RES_STATE_SLEEP),
	TWL4030_RESCONFIG(RES_NRES_PWRON, DEV_GRP_ALL, 0, 1, RES_STATE_SLEEP),
	TWL4030_RESCONFIG(RES_CLKEN, DEV_GRP_ALL, 3, 2, RES_STATE_SLEEP),
	TWL4030_RESCONFIG(RES_SYSEN, DEV_GRP_ALL, 6, 1, RES_STATE_SLEEP),
	TWL4030_RESCONFIG(RES_HFCLKOUT, DEV_GRP_P3, 0, 2, RES_STATE_SLEEP),	/* ? */
	TWL4030_RESCONFIG(0, 0, 0, 0, 0),
};

/**
 * Optimized 'Active to Sleep' sequence
 */
static struct twl4030_ins omap3beagle_sleep_seq[] __initdata = {
	{ MSG_SINGULAR(DEV_GRP_NULL, RES_HFCLKOUT, RES_STATE_SLEEP), 20},
	{ MSG_BROADCAST(DEV_GRP_NULL, RES_GRP_ALL, RES_TYPE_R0, RES_TYPE2_R1, RES_STATE_SLEEP), 2 },
	{ MSG_BROADCAST(DEV_GRP_NULL, RES_GRP_ALL, RES_TYPE_R0, RES_TYPE2_R2, RES_STATE_SLEEP), 2 },
};

static struct twl4030_script omap3beagle_sleep_script __initdata = {
	.script	= omap3beagle_sleep_seq,
	.size	= ARRAY_SIZE(omap3beagle_sleep_seq),
	.flags	= TWL4030_SLEEP_SCRIPT,
};

/**
 * Optimized 'Sleep to Active (P12)' sequence
 */
static struct twl4030_ins omap3beagle_wake_p12_seq[] __initdata = {
	{ MSG_BROADCAST(DEV_GRP_NULL, RES_GRP_ALL, RES_TYPE_R0, RES_TYPE2_R1, RES_STATE_ACTIVE), 2 }
};

static struct twl4030_script omap3beagle_wake_p12_script __initdata = {
	.script = omap3beagle_wake_p12_seq,
	.size   = ARRAY_SIZE(omap3beagle_wake_p12_seq),
	.flags  = TWL4030_WAKEUP12_SCRIPT,
};

/**
 * Optimized 'Sleep to Active' (P3) sequence
 */
static struct twl4030_ins omap3beagle_wake_p3_seq[] __initdata = {
	{ MSG_BROADCAST(DEV_GRP_NULL, RES_GRP_ALL, RES_TYPE_R0, RES_TYPE2_R2, RES_STATE_ACTIVE), 2 }
};

static struct twl4030_script omap3beagle_wake_p3_script __initdata = {
	.script = omap3beagle_wake_p3_seq,
	.size   = ARRAY_SIZE(omap3beagle_wake_p3_seq),
	.flags  = TWL4030_WAKEUP3_SCRIPT,
};

/**
 * Optimized warm reset sequence (for less power surge)
 */
static struct twl4030_ins omap3beagle_wrst_seq[] __initdata = {
	{ MSG_SINGULAR(DEV_GRP_NULL, RES_RESET, RES_STATE_OFF), 0x2 },
	{ MSG_SINGULAR(DEV_GRP_NULL, RES_MAIN_REF, RES_STATE_WRST), 2 },
	{ MSG_BROADCAST(DEV_GRP_NULL, RES_GRP_ALL, RES_TYPE_R0, RES_TYPE2_R2, RES_STATE_WRST), 0x2},
	{ MSG_SINGULAR(DEV_GRP_NULL, RES_VUSB_3V1, RES_STATE_WRST), 0x2 },
	{ MSG_SINGULAR(DEV_GRP_NULL, RES_VPLL1, RES_STATE_WRST), 0x2 },
	{ MSG_SINGULAR(DEV_GRP_NULL, RES_VDD2, RES_STATE_WRST), 0x7 },
	{ MSG_SINGULAR(DEV_GRP_NULL, RES_VDD1, RES_STATE_WRST), 0x25 },
	{ MSG_BROADCAST(DEV_GRP_NULL, RES_GRP_RC, RES_TYPE_ALL, RES_TYPE2_R0, RES_STATE_WRST), 0x2 },
	{ MSG_SINGULAR(DEV_GRP_NULL, RES_RESET, RES_STATE_ACTIVE), 0x2 },

};

static struct twl4030_script omap3beagle_wrst_script __initdata = {
	.script = omap3beagle_wrst_seq,
	.size   = ARRAY_SIZE(omap3beagle_wrst_seq),
	.flags  = TWL4030_WRST_SCRIPT,
};

static struct twl4030_script __initdata *board_twl4030_scripts[] = {
	&omap3beagle_wake_p12_script,
	&omap3beagle_wake_p3_script,
	&omap3beagle_sleep_script,
	&omap3beagle_wrst_script
};

static struct twl4030_power_data __initdata omap3beagle_script_data = {
	.scripts		= board_twl4030_scripts,
	.num			= ARRAY_SIZE(board_twl4030_scripts),
	.resource_config	= board_twl4030_rconfig,
};

static struct twl4030_codec_audio_data beagle_audio_data = {
	.audio_mclk = 26000000,
	.digimic_delay = 1,
	.ramp_delay_value = 1,
	.offset_cncl_path = 1,
	.check_defaults = false,
	.reset_registers = false,
	.reset_registers = false,
};

static struct twl4030_codec_data beagle_codec_data = {
	.audio_mclk = 26000000,
	.audio = &beagle_audio_data,
};

static struct twl4030_platform_data beagle_twldata = {
	.irq_base	= TWL4030_IRQ_BASE,
	.irq_end	= TWL4030_IRQ_END,

	/* platform_data for children goes here */
	.usb		= &beagle_usb_data,
	.gpio		= &beagle_gpio_data,
	.codec		= &beagle_codec_data,
	.vmmc1		= &beagle_vmmc1,
#ifndef CONFIG_MACH_OMAP3_PANTHER
	.vsim		= &beagle_vsim,
#endif
	.vdac		= &beagle_vdac,
	.vpll2		= &beagle_vpll2,
	.vaux3		= &beagle_vaux3,
	.vaux4		= &beagle_vaux4,
	.power		= &omap3beagle_script_data,
};

static struct i2c_board_info __initdata beagle_i2c_boardinfo[] = {
	{
		I2C_BOARD_INFO("twl4030", 0x48),
		.flags = I2C_CLIENT_WAKE,
		.irq = INT_34XX_SYS_NIRQ,
		.platform_data = &beagle_twldata,
	},
};

static struct i2c_board_info __initdata beagle_i2c_eeprom[] = {
       {
               I2C_BOARD_INFO("eeprom", 0x50),
       },
};

/*
static struct i2c_board_info __initdata beagle_i2c_tsc[] = {
		{
				I2C_BOARD_INFO("NuTouch", 0x55),
				.irq = OMAP_GPIO_IRQ(INT_TSC),
		},
};
*/

static struct i2c_board_info __initdata beagle_i2c_tsc[] = {
		{
	          	I2C_BOARD_INFO("ft5x0x_ts", 0x38), 
				.irq = OMAP_GPIO_IRQ(INT_TSC),
		},
};

static struct i2c_board_info __initdata cbc34803_i2c_boardinfo[] = {
		{
		        I2C_BOARD_INFO("hibk", 0x69),
		},
};

static int __init omap3_beagle_i2c_init(void)
{
	omap_register_i2c_bus(1, 2600, beagle_i2c_boardinfo,
			ARRAY_SIZE(beagle_i2c_boardinfo));
//	omap_register_i2c_bus(2, 400, beagle_i2c_rtc, ARRAY_SIZE(beagle_i2c_rtc));
	omap_register_i2c_bus(2, 100, cbc34803_i2c_boardinfo, ARRAY_SIZE(cbc34803_i2c_boardinfo));

	/* Bus 2 is used for Camera/Sensor interface */
	//omap_register_i2c_bus(2, 400, NULL, 0);

	/* Bus 3 is attached to the DVI port where devices like the pico DLP
	 * projector don't work reliably with 400kHz */
	omap_register_i2c_bus(3, 100, beagle_i2c_tsc, ARRAY_SIZE(beagle_i2c_tsc));

	return 0;
}

static struct gpio_led gpio_leds[] = {
	{
		.name			= "beagleboard::usr0",
		.default_trigger	= "heartbeat",
		.gpio			= 150,
	},
	{
		.name			= "beagleboard::usr1",
		.default_trigger	= "mmc0",
		.gpio			= 149,
	},
	{
		.name			= "beagleboard::pmu_stat",
		.gpio			= -EINVAL,	/* gets replaced */
		.active_low		= true,
	},
};

static struct gpio_led_platform_data gpio_led_info = {
	.leds		= gpio_leds,
	.num_leds	= ARRAY_SIZE(gpio_leds),
};

static struct platform_device leds_gpio = {
	.name	= "leds-gpio",
	.id	= -1,
	.dev	= {
		.platform_data	= &gpio_led_info,
	},
};

static struct gpio_keys_button gpio_buttons[] = {
	{
		.code			= KEY_POWER,
		.gpio			= 4,
		.desc			= "user",
		.wakeup			= 1,
	},
};

static struct gpio_keys_platform_data gpio_key_info = {
	.buttons	= gpio_buttons,
	.nbuttons	= ARRAY_SIZE(gpio_buttons),
};

static struct platform_device keys_gpio = {
	.name	= "gpio-keys",
	.id	= -1,
	.dev	= {
		.platform_data	= &gpio_key_info,
	},
};

static void __init omap3_beagle_init_irq(void)
{
	omap2_init_common_infrastructure();
	omap2_init_common_devices(mt46h32m32lf6_sdrc_params,
				  mt46h32m32lf6_sdrc_params);
	omap_init_irq();
	gpmc_init();
#ifdef CONFIG_OMAP_32K_TIMER
#ifdef CONFIG_MACH_OMAP3_PANTHER
	omap2_gp_clockevent_set_gptimer(1);
#else
	if (omap3_beagle_version == OMAP3BEAGLE_BOARD_AXBX)
		omap2_gp_clockevent_set_gptimer(12);
	else
		omap2_gp_clockevent_set_gptimer(1);
#endif
#endif
}

#if defined(CONFIG_MACH_OMAP3_PANTHER) && defined(CONFIG_WL12XX_PLATFORM_DATA)
static struct regulator_consumer_supply beagle_vmmc2_supply =
   REGULATOR_SUPPLY("vmmc", "mmci-omap-hs.1");

/* VMMC2 for driving the WL12xx module */
static struct regulator_init_data beagle_vmmc2 = {
       .constraints = {
               .valid_ops_mask = REGULATOR_CHANGE_STATUS,
           },
       .num_consumer_supplies  = 1,
       .consumer_supplies = &beagle_vmmc2_supply,
    };

static struct fixed_voltage_config beagle_vwlan = {
       .supply_name            = "vwl1271",
       .microvolts             = 1800000, /* 1.80V */
       .gpio                   = BEAGLE_WLAN_PMENA_GPIO,
       .startup_delay          = 70000, /* 70ms */
       .enable_high            = 1,
       .enabled_at_boot        = 0,
       .init_data              = &beagle_vmmc2,
    };

static struct platform_device beagle_wlan_regulator = {
       .name           = "reg-fixed-voltage",
       .id             = 1,
       .dev = {
               .platform_data  = &beagle_vwlan,
           },
    };
struct wl12xx_platform_data beagle_wlan_data __initdata = {
        .irq = OMAP_GPIO_IRQ(BEAGLE_WLAN_IRQ_GPIO),
        .board_ref_clock = WL12XX_REFCLOCK_38, /* 38.4 MHz */
    };
#endif

#if defined(CONFIG_MACH_OMAP3_PANTHER) && defined(CONFIG_TI_ST)
int plat_kim_suspend(struct platform_device *pdev, pm_message_t state)
{
    /* TODO: wait for HCI-LL sleep */
    return 0;
}

int plat_kim_resume(struct platform_device *pdev)
{
    return 0;
}

/* TI BT, FM, GPS connectivity chip */
struct ti_st_plat_data wilink_pdata = {
    .nshutdown_gpio = BEAGLE_BTEN_GPIO,
    .dev_name = "/dev/ttyO1",
    .flow_cntrl = 1,
    .baud_rate = 3000000,
    .suspend = plat_kim_suspend,
    .resume = plat_kim_resume,
};
static struct platform_device kim_device = {
    .name           = "kim",
    .id             = -1,
    .dev.platform_data = &wilink_pdata,
};
static struct platform_device btwilink_device = {
    .name = "btwilink",
    .id = -1,
};
#endif

static struct platform_device *omap3_beagle_devices[] __initdata = {
	&leds_gpio,
	&keys_gpio,
	&beagle_dss_device,
	&usb_mass_storage_device,
#if defined(CONFIG_MACH_OMAP3_PANTHER) && defined(CONFIG_TI_ST)
    &kim_device,
    &btwilink_device,
#endif
};

static void __init omap3beagle_flash_init(void)
{
	u8 cs = 0;
	u8 nandcs = GPMC_CS_NUM + 1;

	/* find out the chip-select on which NAND exists */
	while (cs < GPMC_CS_NUM) {
		u32 ret = 0;
		ret = gpmc_cs_read_reg(cs, GPMC_CS_CONFIG1);

		if ((ret & 0xC00) == 0x800) {
			printk(KERN_INFO "Found NAND on CS%d\n", cs);
			if (nandcs > GPMC_CS_NUM)
				nandcs = cs;
		}
		cs++;
	}

	if (nandcs > GPMC_CS_NUM) {
		printk(KERN_INFO "NAND: Unable to find configuration "
				 "in GPMC\n ");
		return;
	}

	if (nandcs < GPMC_CS_NUM) {
		printk(KERN_INFO "Registering NAND on CS%d\n", nandcs);
		board_nand_init(omap3beagle_nand_partitions,
			ARRAY_SIZE(omap3beagle_nand_partitions),
			nandcs, NAND_BUSWIDTH_16);
	}
}

static const struct ehci_hcd_omap_platform_data ehci_pdata __initconst = {

#ifdef CONFIG_MACH_OMAP3_PANTHER
	.port_mode[0] = EHCI_HCD_OMAP_MODE_UNKNOWN,
#else
	.port_mode[0] = EHCI_HCD_OMAP_MODE_PHY,
#endif
	.port_mode[1] = EHCI_HCD_OMAP_MODE_PHY,
	.port_mode[2] = EHCI_HCD_OMAP_MODE_UNKNOWN,

	.phy_reset  = true,
	.reset_gpio_port[0]  = -EINVAL,
#ifdef CONFIG_MACH_OMAP3_PANTHER
	.reset_gpio_port[1]  = 39,
#else
	.reset_gpio_port[1]  = 147,
#endif
	.reset_gpio_port[2]  = -EINVAL
};

#ifdef CONFIG_OMAP_MUX
static struct omap_board_mux board_mux[] __initdata = {
    OMAP3_MUX(SYS_NIRQ, OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLUP |
                OMAP_PIN_OFF_INPUT_PULLUP | OMAP_PIN_OFF_OUTPUT_LOW |
                OMAP_PIN_OFF_WAKEUPENABLE),
#ifdef CONFIG_MACH_OMAP3_PANTHER
    OMAP3_MUX(CAM_D0,OMAP_MUX_MODE0|OMAP_PIN_OFF_INPUT_PULLDOWN),
    OMAP3_MUX(CAM_D1,OMAP_MUX_MODE0|OMAP_PIN_OFF_INPUT_PULLDOWN),
    OMAP3_MUX(CAM_D2,OMAP_MUX_MODE0|OMAP_PIN_OFF_INPUT_PULLDOWN),
    OMAP3_MUX(CAM_D3,OMAP_MUX_MODE0|OMAP_PIN_OFF_INPUT_PULLDOWN),
    OMAP3_MUX(CAM_D4,OMAP_MUX_MODE0|OMAP_PIN_OFF_INPUT_PULLDOWN),
    OMAP3_MUX(CAM_D5,OMAP_MUX_MODE0|OMAP_PIN_OFF_INPUT_PULLDOWN),
    OMAP3_MUX(CAM_D6,OMAP_MUX_MODE0|OMAP_PIN_OFF_INPUT_PULLDOWN),
    OMAP3_MUX(CAM_D7,OMAP_MUX_MODE0|OMAP_PIN_OFF_INPUT_PULLDOWN),

    OMAP3_MUX(CAM_D8,OMAP_MUX_MODE0|OMAP_PIN_OFF_INPUT_PULLDOWN),
    OMAP3_MUX(CAM_D9,OMAP_MUX_MODE0|OMAP_PIN_OFF_INPUT_PULLDOWN),
    OMAP3_MUX(CAM_D10,OMAP_MUX_MODE0|OMAP_PIN_OFF_INPUT_PULLDOWN),
    OMAP3_MUX(CAM_D11,OMAP_MUX_MODE0|OMAP_PIN_OFF_INPUT_PULLDOWN),


    OMAP3_MUX(DSS_DATA0,OMAP_MUX_MODE0|OMAP_PIN_OFF_INPUT_PULLDOWN),
    OMAP3_MUX(DSS_DATA1,OMAP_MUX_MODE0|OMAP_PIN_OFF_INPUT_PULLDOWN),
    OMAP3_MUX(DSS_DATA2,OMAP_MUX_MODE0|OMAP_PIN_OFF_INPUT_PULLDOWN),
    OMAP3_MUX(DSS_DATA3,OMAP_MUX_MODE0|OMAP_PIN_OFF_INPUT_PULLDOWN),
    OMAP3_MUX(DSS_DATA4,OMAP_MUX_MODE0|OMAP_PIN_OFF_INPUT_PULLDOWN),
    OMAP3_MUX(DSS_DATA5,OMAP_MUX_MODE0|OMAP_PIN_OFF_INPUT_PULLDOWN),

    //  OMAP3_MUX(UART2_RX,OMAP_MUX_MODE1|OMAP_PIN_OFF_INPUT_PULLDOWN),
    OMAP3_MUX(CAM_XCLKB,OMAP_MUX_MODE0|OMAP_PIN_OFF_INPUT_PULLDOWN),

    OMAP3_MUX(CSI2_DY0,OMAP_MUX_MODE4|OMAP_PIN_OFF_INPUT_PULLDOWN),
    OMAP3_MUX(CSI2_DX1,OMAP_MUX_MODE4|OMAP_PIN_OFF_INPUT_PULLDOWN),
    OMAP3_MUX(CSI2_DY1,OMAP_MUX_MODE4|OMAP_PIN_INPUT_PULLUP), //HaoNC

    OMAP3_MUX(MCBSP1_CLKX,OMAP_MUX_MODE0|OMAP_PIN_OFF_INPUT_PULLDOWN),
    OMAP3_MUX(MCBSP1_DX,OMAP_MUX_MODE0|OMAP_PIN_OFF_INPUT_PULLDOWN),
    OMAP3_MUX(MCBSP4_CLKX,OMAP_MUX_MODE0|OMAP_PIN_OFF_INPUT_PULLDOWN),
    OMAP3_MUX(MCBSP4_DR,OMAP_MUX_MODE0|OMAP_PIN_OFF_INPUT_PULLDOWN),

    OMAP3_MUX(MCBSP4_DX,OMAP_MUX_MODE0|OMAP_PIN_OFF_INPUT_PULLDOWN),
    OMAP3_MUX(MCBSP4_FSX,OMAP_MUX_MODE0|OMAP_PIN_OFF_INPUT_PULLDOWN),
    OMAP3_MUX(MCBSP_CLKS,OMAP_MUX_MODE0|OMAP_PIN_OFF_INPUT_PULLDOWN),
    OMAP3_MUX(ETK_D9,OMAP_MUX_MODE2|OMAP_PIN_OFF_INPUT_PULLDOWN),
    OMAP3_MUX(MCBSP3_DX,OMAP_MUX_MODE0|OMAP_PIN_OFF_INPUT_PULLDOWN),
    OMAP3_MUX(MCBSP3_DR,OMAP_MUX_MODE0|OMAP_PIN_OFF_INPUT_PULLDOWN),
    OMAP3_MUX(MCBSP3_CLKX,OMAP_MUX_MODE0|OMAP_PIN_OFF_INPUT_PULLDOWN),
    OMAP3_MUX(MCSPI1_SIMO,OMAP_MUX_MODE0|OMAP_PIN_OFF_INPUT_PULLDOWN),

    OMAP3_MUX(MCSPI1_SOMI,OMAP_MUX_MODE0|OMAP_PIN_OFF_INPUT_PULLDOWN),
    OMAP3_MUX(ETK_D4,OMAP_MUX_MODE2|OMAP_PIN_OFF_INPUT_PULLDOWN),
    OMAP3_MUX(ETK_D5,OMAP_MUX_MODE2|OMAP_PIN_OFF_INPUT_PULLDOWN),
    OMAP3_MUX(ETK_D6,OMAP_MUX_MODE2|OMAP_PIN_OFF_INPUT_PULLDOWN),
    OMAP3_MUX(ETK_D7,OMAP_MUX_MODE2|OMAP_PIN_OFF_INPUT_PULLDOWN),
#endif
#if defined(CONFIG_MACH_OMAP3_PANTHER) && defined(CONFIG_WL12XX_PLATFORM_DATA)
    /* WLAN IRQ - GPIO 112 */
    OMAP3_MUX(CSI2_DX0, OMAP_MUX_MODE4 | OMAP_PIN_INPUT),
    /* WLAN POWER ENABLE - GPIO 16 */
    OMAP3_MUX(ETK_D2, OMAP_MUX_MODE4 | OMAP_PIN_OUTPUT),
    /* MMC2 SDIO pin muxes for WL12xx */
    OMAP3_MUX(SDMMC2_CLK, OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLUP),
    OMAP3_MUX(SDMMC2_CMD, OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLUP),
    OMAP3_MUX(SDMMC2_DAT0, OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLUP),
    OMAP3_MUX(SDMMC2_DAT1, OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLUP),
    OMAP3_MUX(SDMMC2_DAT2, OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLUP),
    OMAP3_MUX(SDMMC2_DAT3, OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLUP),
#endif
	{ .reg_offset = OMAP_MUX_TERMINATOR },
};
#endif

static struct omap_musb_board_data musb_board_data = {
	.interface_type		= MUSB_INTERFACE_ULPI,
	.mode			= MUSB_OTG,
	.power			= 100,
};

static void __init omap3_beagle_init(void)
{
	omap3_mux_init(board_mux, OMAP_PACKAGE_CBB);
#ifndef CONFIG_MACH_OMAP3_PANTHER
	omap3_beagle_init_rev();
#endif
	omap3_beagle_i2c_init();
	platform_add_devices(omap3_beagle_devices,
			ARRAY_SIZE(omap3_beagle_devices));
	omap_serial_init();

#ifndef CONFIG_MACH_OMAP3_PANTHER
	omap_mux_init_gpio(170, OMAP_PIN_INPUT);
	gpio_request(170, "DVI_nPD");
	/* REVISIT leave DVI powered down until it's needed ... */
	gpio_direction_output(170, true);
#else
    omap_mux_init_gpio(129, OMAP_PIN_INPUT);
    gpio_request(129, "DVI_nPD");
    /* REVISIT leave DVI powered down until it's needed ... */
    gpio_direction_output(129, true);
#endif

	usb_musb_init(&musb_board_data);
	usb_ehci_init(&ehci_pdata);
	omap3beagle_flash_init();

	/* Ensure SDRC pins are mux'd for self-refresh */
	omap_mux_init_signal("sdrc_cke0", OMAP_PIN_OUTPUT);
	omap_mux_init_signal("sdrc_cke1", OMAP_PIN_OUTPUT);

#if defined(CONFIG_MACH_OMAP3_PANTHER) && defined(CONFIG_WL12XX_PLATFORM_DATA)
    /* wl12xx WLAN Init */
    if (wl12xx_set_platform_data(&beagle_wlan_data))
        pr_err("error setting wl12xx data\n");
    platform_device_register(&beagle_wlan_regulator);
#endif

	beagle_display_init();
#ifdef CONFIG_USB_ANDROID
	omap3beagle_android_gadget_init();
#endif
	omap3_beagle_pm_init();
}

MACHINE_START(OMAP3_BEAGLE, "OMAP3 Beagle Board")
	/* Maintainer: Syed Mohammed Khasim - http://beagleboard.org */
	.boot_params	= 0x80000100,
	.map_io		= omap3_map_io,
	.reserve	= omap_reserve,
	.init_irq	= omap3_beagle_init_irq,
	.init_machine	= omap3_beagle_init,
	.timer		= &omap_timer,
MACHINE_END
