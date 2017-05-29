/* linux/arch/arm/mach-msm/board-flyer-keypad.c
 *
 * Copyright (C) 2010-2011 HTC Corporation.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/gpio_event.h>
#include <linux/keyreset.h>
#include <asm/mach-types.h>
#include <linux/gpio.h>
#include <mach/gpio.h>
#include <mach/board_htc.h>

#include "board-flyer.h"
#include "proc_comm.h"
#include <linux/mfd/pmic8058.h>

static char *keycaps = "--qwerty";
#undef MODULE_PARAM_PREFIX
#define MODULE_PARAM_PREFIX "board_flyer."
module_param_named(keycaps, keycaps, charp, 0);

static void config_gpio_table(uint32_t *table, int len)
{
	int n, rc;
	for (n = 0; n < len; n++) {
		rc = gpio_tlmm_config(table[n], GPIO_CFG_ENABLE);
		if (rc) {
			pr_err("[KEY] %s: gpio_tlmm_config(%#x)=%d\n",
				__func__, table[n], rc);
			break;
		}
	}
}

static struct gpio_event_direct_entry flyer_keypad_input_map_xc[] = {
	{
		.gpio = FLYER_GPIO_KEYPAD_POWER_KEY,
		.code = KEY_POWER,
	},
	{
		.gpio = PM8058_GPIO_PM_TO_SYS(FLYER_VOL_UP_XC),
		.code = KEY_VOLUMEUP,
	},
	{
		.gpio = PM8058_GPIO_PM_TO_SYS(FLYER_VOL_DN_XC),
		.code = KEY_VOLUMEDOWN,
	},
};

static struct gpio_event_direct_entry flyer_keypad_input_map[] = {
	{
		.gpio = FLYER_GPIO_KEYPAD_POWER_KEY,
		.code = KEY_POWER,
	},
	{
		.gpio = PM8058_GPIO_PM_TO_SYS(FLYER_VOL_UP),
		.code = KEY_VOLUMEUP,
	},
	{
		.gpio = PM8058_GPIO_PM_TO_SYS(FLYER_VOL_DN),
		.code = KEY_VOLUMEDOWN,
	},
};

uint32_t inputs_gpio_table[] = {
	PCOM_GPIO_CFG(FLYER_GPIO_KEYPAD_POWER_KEY, 0, GPIO_INPUT, GPIO_PULL_UP, GPIO_4MA),
};

static void flyer_setup_input_gpio(void)
{
	config_gpio_table(inputs_gpio_table, ARRAY_SIZE(inputs_gpio_table));
}

static struct gpio_event_input_info flyer_keypad_input_info = {
	.info.func = gpio_event_input_func,
	.flags = GPIOEDF_PRINT_KEYS,
	.type = EV_KEY,
#if BITS_PER_LONG != 64 && !defined(CONFIG_KTIME_SCALAR)
	.debounce_time.tv.nsec = 5 * NSEC_PER_MSEC,
# else
	.debounce_time.tv64 = 5 * NSEC_PER_MSEC,
# endif
	.keymap = flyer_keypad_input_map,
	.keymap_size = ARRAY_SIZE(flyer_keypad_input_map),
	.setup_input_gpio = flyer_setup_input_gpio,
};

static struct gpio_event_info *flyer_keypad_info[] = {
	&flyer_keypad_input_info.info,
};

static struct gpio_event_platform_data flyer_keypad_data = {
	.name = "flyer-keypad-v0",
	.info = flyer_keypad_info,
	.info_count = ARRAY_SIZE(flyer_keypad_info),
};

static struct platform_device flyer_keypad_input_device = {
	.name = GPIO_EVENT_DEV_NAME,
	.id = 0,
	.dev		= {
		.platform_data	= &flyer_keypad_data,
	},
};

int __init flyer_init_keypad(void)
{
	pr_info("[KEY] %s\n", __func__);

	if (system_rev >= 2) {
		flyer_keypad_input_info.keymap = flyer_keypad_input_map_xc;
		flyer_keypad_input_info.keymap_size =
				ARRAY_SIZE(flyer_keypad_input_map_xc);
	}
	return platform_device_register(&flyer_keypad_input_device);
}
