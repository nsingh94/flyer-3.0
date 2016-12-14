/* Copyright (c) 2009-2011, Code Aurora Forum. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/kernel.h>
#include <linux/irq.h>
#include <linux/gpio.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/bootmem.h>
#include <linux/io.h>
#ifdef CONFIG_SPI_QSD
#include <linux/spi/spi.h>
#endif
#include <linux/mfd/pmic8058.h>
#include <linux/leds.h>
#include <linux/mfd/marimba.h>
#include <linux/i2c.h>
#include <linux/akm8975.h>
#include <linux/bma150.h>
#include <linux/lightsensor.h>
#include <linux/input.h>
#include <linux/ntrig.h>
#include <linux/cap_sense.h>
#include <linux/smsc911x.h>
#include <linux/power_supply.h>
#include <linux/i2c/isa1200.h>
#include <linux/input/kp_flip_switch.h>
#include <linux/leds-pm8058.h>
#include <linux/msm_adc.h>
#include <linux/dma-mapping.h>
#include <linux/proc_fs.h>
#include <linux/htc_flashlight.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/setup.h>

#include <mach/system.h>
#include <mach/mpp.h>
#include <mach/board.h>
#include <mach/camera-7x30.h>
#include <mach/memory.h>
#include <mach/msm_iomap.h>
#ifdef CONFIG_USB_MSM_OTG_72K
#include <mach/msm_hsusb.h>
#else
#include <linux/usb/msm_hsusb.h>
#endif
#include <mach/msm_spi.h>
#include <mach/qdsp5v2_2x/msm_lpa.h>
#include <mach/dma.h>
#include <linux/android_pmem.h>
#include <linux/input/msm_ts.h>
#include <mach/pmic.h>
#include <mach/rpc_pmapp.h>
#include <mach/qdsp5v2_2x/aux_pcm.h>
#include <mach/qdsp5v2_2x/mi2s.h>
#include <mach/qdsp5v2_2x/audio_dev_ctl.h>
#include <mach/htc_battery.h>
#include <linux/ds2746_battery.h>
#include <linux/tps65200.h>
#include <mach/rpc_server_handset.h>
#include <mach/msm_tsif.h>
#include <mach/socinfo.h>
#include <mach/msm_memtypes.h>
#include <asm/mach/mmc.h>
#include <asm/mach/flash.h>
#include <mach/vreg.h>
#include <linux/platform_data/qcom_crypto_device.h>
#include <mach/atmega_microp.h>
#include <mach/htc_headset_mgr.h>
#include <mach/htc_headset_gpio.h>
#include <mach/htc_headset_pmic.h>
#include <mach/htc_headset_misc.h>
#include <mach/htc_headset_microp.h>

#include "devices.h"
#include "timer.h"
#ifdef CONFIG_USB_G_ANDROID
#include <mach/htc_usb.h>
#include <linux/usb/android_composite.h>
#include <linux/usb/android.h>
#include <mach/usbdiag.h>
#endif
#include "pm.h"
#include "pm-boot.h"
#include "spm.h"
#include "acpuclock.h"
#include <mach/dal_axi.h>
#include <mach/msm_serial_hs.h>
#ifdef CONFIG_SERIAL_BCM_BT_LPM
#include <mach/bcm_bt_lpm.h>
#endif
#include <mach/qdsp5v2_2x/mi2s.h>
#include <mach/qdsp5v2_2x/audio_dev_ctl.h>
#include <mach/sdio_al.h>
#include "smd_private.h"
#include "board-flyer.h"
#include "board-msm7x30-regulator.h"
#include <mach/board_htc.h>

#ifdef CONFIG_PERFLOCK
#include <mach/perflock.h>
#endif

#ifdef CONFIG_BT
#include <mach/htc_bdaddress.h>
#endif

#include <mach/cable_detect.h>

#define XC 2
#define XD 3

#define GPIO_2MA	0
#define GPIO_4MA	1
#define GPIO_6MA	2
#define GPIO_8MA	3
#define GPIO_10MA	4
#define GPIO_12MA	5
#define GPIO_14MA	6
#define GPIO_16MA	7

#define GPIO_INPUT      0
#define GPIO_OUTPUT     1

#define GPIO_NO_PULL    0
#define GPIO_PULL_DOWN  1
#define GPIO_PULL_UP    3

#define PCOM_GPIO_CFG(gpio, func, dir, pull, drvstr) \
		((((gpio) & 0x3FF) << 4)        | \
		((func) & 0xf)                  | \
		(((dir) & 0x1) << 14)           | \
		(((pull) & 0x3) << 15)          | \
		(((drvstr) & 0xF) << 17))

struct pm8xxx_gpio_init_info {
	unsigned			gpio;
	struct pm_gpio			config;
};

int __init flyer_init_panel(void);

static int board_ver;
static unsigned int engineerid;
unsigned long msm_fb_base;

static int get_thermal_id(void)
{
	return THERMAL_300_47_3440;
}

static int get_battery_id(void)
{
	if (system_rev >= XD)
		return -1; /* means need battery id detection */
	else
		return 3;
}

static int flyer_ts_ntrig_power(int on)
{
	pr_info("[ts]%s(%d):\n", __func__, on);

	if (on) {
		gpio_set_value(FLYER_GPIO_TP_3V3_ENABLE, 1);
		mdelay(1);
		pm8xxx_gpio_cfg(FLYER_TP_ATT_PMIC, PM_GPIO_DIR_IN, 0, 0, PM_GPIO_PULL_UP_31P5, PM8058_GPIO_VIN_L6, 0, PM_GPIO_FUNC_PAIRED, 0);
	} else {
		gpio_set_value(FLYER_GPIO_TP_3V3_ENABLE, 0);
		system_rev >= XC ? gpio_set_value(FLYER_GPIO_SPI_ENABLE_XC, 0) :
							gpio_set_value(FLYER_GPIO_SPI_ENABLE, 0);

		pm8xxx_gpio_cfg(FLYER_TP_ATT_PMIC, PM_GPIO_DIR_IN, 0, 0, PM_GPIO_PULL_DN, 0, PM_GPIO_STRENGTH_NO, PM_GPIO_FUNC_NORMAL, 0);
	}

	return 0;
}

static struct ntrig_spi_platform_data flyer_ts_ntrig_data[] = {
	{
		.abs_x_min = 600,
		.abs_x_max = 7200,
		.abs_y_min = 0,
		.abs_y_max = 9150,
		.fwtwozero = 0x1620,
		.abs_width_min = 0,
		.abs_width_max = 100,
		.orientate = 0x03,
		.abs_pressure_min = 0,
		.abs_pressure_max = 255,
		.spi_enable = FLYER_GPIO_SPI_ENABLE,
		.irq_gpio = FLYER_GPIO_TP_ATT_N,
		.power = flyer_ts_ntrig_power,
		.esdFlag = true,
	},
};

static struct microp_function_config microp_functions[] = {
	{
		.name   = "microp_intrrupt",
		.category = MICROP_FUNCTION_INTR,
	},
	{
		.name   = "reset-int",
		.category = MICROP_FUNCTION_RESET_INT,
		.int_pin = 1 << 8,
	},
};

static int __capella_cm3602_power(int on)
{
	uint8_t data[3];
	int ret;

	data[0] = 0;
	data[1] = 0;
	data[2] = 0x10;

	printk(KERN_DEBUG "%s: :Pull the ALS_SHDN %s\n",
		__func__, (on) ? "LOW" : "HIGH");
	if (on) {
		ret = microp_i2c_write(MICROP_I2C_WCMD_GPO_LED_STATUS_EN,
			data, 3);
		if (ret)
			printk(KERN_WARNING "%s: Pull PD4 DOWN fail!\n",
				__func__);
	} else {
		ret = microp_i2c_write(MICROP_I2C_WCMD_GPO_LED_STATUS_DIS,
			data, 3);
		if (ret)
			printk(KERN_WARNING "%s: Pull PD4 UP fail!\n",
				__func__);
	}
	return 0;
}

static DEFINE_MUTEX(capella_cm3602_lock);
static int als_power_control;

static int capella_cm3602_power(int pwr_device, uint8_t enable)
{
	/* TODO eolsen Add Voltage reg control */
	unsigned int old_status = 0;
	int ret = 0, on = 0;
	mutex_lock(&capella_cm3602_lock);

	old_status = als_power_control;
	if (enable)
		als_power_control |= pwr_device;
	else
		als_power_control &= ~pwr_device;

	on = als_power_control ? 1 : 0;
	if (old_status == 0 && on)
		ret = __capella_cm3602_power(1);
	else if (!on)
		ret = __capella_cm3602_power(0);

	mutex_unlock(&capella_cm3602_lock);
	return ret;
}

static struct microp_function_config microp_lightsensor_function = {
	.name = "light_sensor",
	.category = MICROP_FUNCTION_LSENSOR,
	.levels = { 5, 22, 41, 275, 366, 427, 488, 549,610, 4095 },
	.channel = 3,
	.int_pin = 1 << 9,
	.golden_adc = 0xB1,
	.ls_power = capella_cm3602_power,
};

static struct lightsensor_platform_data lightsensor_data = {
	.config = &microp_lightsensor_function,
	.irq = MSM_uP_TO_INT(9),
};

static struct microp_led_config up_led_config[] = {
	{
		.name = "amber",
		.type = LED_RGB,
	},
	{
		.name = "green",
		.type = LED_RGB,
	},
	{
		.name = "button-backlight-portrait",
		.type = LED_PWM,
		.led_pin = 1 << 0,
		.init_value = 0xFF,
		.fade_time = 5,
	},
	{
		.name = "button-backlight-landscape",
		.type = LED_PWM,
		.led_pin = 1 << 1,
		.init_value = 0xFF,
		.fade_time = 5,
	},
};

static struct microp_led_config up_led_config_XD2[] = {
	{
		.name = "amber",
		.type = LED_RGB,
	},
	{
		.name = "green",
		.type = LED_RGB,
	},
	{
		.name = "button-backlight-portrait",
		.type = LED_PWM,
		.led_pin = 1 << 0,
		.fade_time = 5,
	},
	{
		.name = "button-backlight-landscape",
		.type = LED_PWM,
		.led_pin = 1 << 1,
		.fade_time = 5,
	},
};

static struct microp_led_platform_data microp_leds_data = {
	.num_leds	= ARRAY_SIZE(up_led_config),
	.led_config	= up_led_config,
};

static struct bma150_platform_data flyer_g_sensor_pdata = {
	.microp_new_cmd = 1,
	.chip_layout = 1,
};

/* HTC_HEADSET_MICROP Driver */
static struct htc_headset_microp_platform_data htc_headset_microp_data = {
	.remote_int		= 1 << 13,
	.remote_irq		= MSM_uP_TO_INT(13),
	.remote_enable_pin	= 0,
	.adc_channel		= 0x01,
	.adc_remote		= {0, 33, 43, 73, 110, 172},
};

static struct platform_device htc_headset_microp = {
	.name	= "HTC_HEADSET_MICROP",
	.id	= -1,
	.dev	= {
		.platform_data	= &htc_headset_microp_data,
	},
};

/* HTC_HEADSET_GPIO Driver */
static struct htc_headset_gpio_platform_data htc_headset_gpio_data = {
    .hpin_gpio		= PM8058_GPIO_PM_TO_SYS(FLYER_AUD_HP_DETz),
	.key_enable_gpio	= 0,
	.mic_select_gpio	= FLYER_AUD_MICPATH_SEL,
};

static struct platform_device htc_headset_gpio = {
	.name	= "HTC_HEADSET_GPIO",
	.id	= -1,
	.dev	= {
		.platform_data	= &htc_headset_gpio_data,
	},
};

/* HTC_HEADSET_PMIC Driver */
static struct htc_headset_pmic_platform_data htc_headset_pmic_data = {
	.hpin_gpio	= 0,
	.hpin_irq	= MSM_GPIO_TO_INT(PM8058_GPIO_PM_TO_SYS(FLYER_AUD_HP_DETz)),
};

static struct platform_device htc_headset_pmic = {
	.name	= "HTC_HEADSET_PMIC",
	.id	= -1,
	.dev	= {
		.platform_data	= &htc_headset_pmic_data,
	},
};

/* HTC_HEADSET_MISC Driver */
static struct htc_headset_misc_platform_data htc_headset_misc_data = {
	.driver_flag		= DRIVER_HS_MISC_EXT_HP_DET,
	.ext_hpin_gpio		= PM8058_GPIO_PM_TO_SYS(FLYER_H2W_CABLE_IN1),
	.ext_accessory_type	= USB_AUDIO_OUT,
};

static struct platform_device htc_headset_misc = {
	.name	= "HTC_HEADSET_MISC",
	.id	= -1,
	.dev	= {
		.platform_data	= &htc_headset_misc_data,
	},
};

/* HTC_HEADSET_MGR Driver */
static struct platform_device *headset_devices[] = {
	&htc_headset_microp,
	&htc_headset_gpio,
	&htc_headset_misc,
	&htc_headset_pmic,
	/* Please put the headset detection driver on the last */
};

static struct headset_adc_config htc_headset_mgr_config[] = {
	{
		.type = HEADSET_MIC,
		.adc_max = 1023,
		.adc_min = 512,
	},
	{
		.type = HEADSET_BEATS,
		.adc_max = 512,
		.adc_min = 200,
	},
	{
		.type = HEADSET_NO_MIC,
		.adc_max = 200,
		.adc_min = 0,
	},
};

static struct htc_headset_mgr_platform_data htc_headset_mgr_data = {
	.headset_devices_num	= ARRAY_SIZE(headset_devices),
	.headset_devices	= headset_devices,
	.headset_config_num	= ARRAY_SIZE(htc_headset_mgr_config),
	.headset_config		= htc_headset_mgr_config,
};

static struct platform_device microp_devices[] = {
	{
		.name = "lightsensor_microp",
		.dev = {
			.platform_data = &lightsensor_data,
		},
	},
	{
		.name = "leds-microp",
		.id = -1,
		.dev = {
			.platform_data = &microp_leds_data,
		},
	},
	{
		.name = BMA150_G_SENSOR_NAME,
		.dev = {
			.platform_data = &flyer_g_sensor_pdata,
		},
	},
	{
		.name	= "HTC_HEADSET_MGR",
		.id	= -1,
		.dev	= {
			.platform_data	= &htc_headset_mgr_data,
		},
	},
};

static struct microp_i2c_platform_data microp_data = {
	.num_functions   = ARRAY_SIZE(microp_functions),
	.microp_function = microp_functions,
	.num_devices = ARRAY_SIZE(microp_devices),
	.microp_devices = microp_devices,
	.gpio_reset = FLYER_GPIO_UP_RESET_N,
	.spi_devices = SPI_GSENSOR,
};

static struct gpio_led gpio_led_config[] = {
	{
		.name = "amber-portrait",
		.gpio = PM8058_GPIO_PM_TO_SYS(FLYER_PEN_LED1),
		.active_low = 1,
	},
	{
		.name = "green-portrait",
		.gpio = PM8058_GPIO_PM_TO_SYS(FLYER_PEN_LED2),
		.active_low = 1,
	},
	{
		.name = "blue-portrait",
		.gpio = PM8058_GPIO_PM_TO_SYS(FLYER_PEN_LED3),
		.active_low = 1,
	},
	{
		.name = "amber-landscape",
		.gpio = PM8058_GPIO_PM_TO_SYS(FLYER_PEN_LED4),
		.active_low = 1,
	},
	{
		.name = "green-landscape",
		.gpio = PM8058_GPIO_PM_TO_SYS(FLYER_PEN_LED5),
		.active_low = 1,
	},
	{
		.name = "blue-landscape",
		.gpio = PM8058_GPIO_PM_TO_SYS(FLYER_PEN_LED6),
		.active_low = 1,
	},
};

static struct gpio_led gpio_led_config_XC[] = {
	{
		.name = "amber-portrait",
		.gpio = PM8058_GPIO_PM_TO_SYS(FLYER_PEN_LED1),
		.active_low = 1,
	},
	{
		.name = "green-portrait",
		.gpio = PM8058_GPIO_PM_TO_SYS(FLYER_PEN_LED2),
		.active_low = 1,
	},
	{
		.name = "blue-portrait",
		.gpio = PM8058_GPIO_PM_TO_SYS(FLYER_PEN_LED3),
		.active_low = 1,
	},
	{
		.name = "amber-landscape",
		.gpio = PM8058_GPIO_PM_TO_SYS(FLYER_PEN_LED1),
		.active_low = 1,
	},
	{
		.name = "green-landscape",
		.gpio = PM8058_GPIO_PM_TO_SYS(FLYER_PEN_LED2),
		.active_low = 1,
	},
	{
		.name = "blue-landscape",
		.gpio = PM8058_GPIO_PM_TO_SYS(FLYER_PEN_LED3),
		.active_low = 1,
	},
	{
		.name = "green-camera",
		.gpio = PM8058_GPIO_PM_TO_SYS(FLYER_2ND_CAM_LED),
		.active_low = 0,
	},
};


static struct gpio_led gpio_led_config_XD[] = {
	{
		.name = "amber-portrait",
		.gpio = PM8058_GPIO_PM_TO_SYS(FLYER_PEN_LED1),
		.active_low = 1,
	},
	{
		.name = "green-portrait",
		.gpio = PM8058_GPIO_PM_TO_SYS(FLYER_PEN_LED2),
		.active_low = 1,
	},
	{
		.name = "blue-portrait",
		.gpio = PM8058_GPIO_PM_TO_SYS(FLYER_PEN_LED3),
		.active_low = 1,
	},
	{
		.name = "amber-landscape",
		.gpio = PM8058_GPIO_PM_TO_SYS(FLYER_PEN_LED1),
		.active_low = 1,
	},
	{
		.name = "green-landscape",
		.gpio = PM8058_GPIO_PM_TO_SYS(FLYER_PEN_LED2),
		.active_low = 1,
	},
	{
		.name = "blue-landscape",
		.gpio = PM8058_GPIO_PM_TO_SYS(FLYER_PEN_LED3),
		.active_low = 1,
	},
	{
		.name = "green-camera",
		.gpio = FLYER_2ND_CAM_LED_XD,
		.active_low = 0,
	},
};


static struct gpio_led gpio_led_config_XD2[] = {
	{
		.name = "amber-portrait",
		.gpio = PM8058_GPIO_PM_TO_SYS(FLYER_PEN_LED1),
		.active_low = 0,
	},
	{
		.name = "green-portrait",
		.gpio = PM8058_GPIO_PM_TO_SYS(FLYER_PEN_LED2),
		.active_low = 0,
	},
	{
		.name = "white-portrait",
		.gpio = PM8058_GPIO_PM_TO_SYS(FLYER_PEN_LED3),
		.active_low = 0,
	},
	{
		.name = "amber-landscape",
		.gpio = PM8058_GPIO_PM_TO_SYS(FLYER_PEN_LED1),
		.active_low = 0,
	},
	{
		.name = "green-landscape",
		.gpio = PM8058_GPIO_PM_TO_SYS(FLYER_PEN_LED2),
		.active_low = 0,
	},
	{
		.name = "white-landscape",
		.gpio = PM8058_GPIO_PM_TO_SYS(FLYER_PEN_LED3),
		.active_low = 0,
	},
	{
		.name = "green-camera",
		.gpio = FLYER_2ND_CAM_LED_XD,
		.active_low = 0,
	},
};

static struct gpio_led_platform_data gpio_leds_data = {
	.num_leds = ARRAY_SIZE(gpio_led_config),
	.leds = gpio_led_config,
};

static struct platform_device gpio_leds = {
	.name = "leds-gpio",
	.id = -1,
	.dev = {
		.platform_data = &gpio_leds_data,
	},
};

/* Compass platform data */

static struct akm8975_platform_data compass_platform_data_XA_XB = {
	.layouts = FLYER_COMPASS_LAYOUTS_XA_XB,
	.irq_trigger = 1,
};

static struct akm8975_platform_data compass_platform_data_XC = {
	.layouts = FLYER_COMPASS_LAYOUTS_XC,
	.irq_trigger = 1,
};

static struct akm8975_platform_data compass_platform_data_XD = {
	.layouts = FLYER_COMPASS_LAYOUTS_XC,
	.irq_trigger = 1,
};

static struct akm8975_platform_data compass_platform_data_VER_A = {
	.layouts = FLYER_COMPASS_LAYOUTS_VER_A,
	.irq_trigger = 1,
};

#ifdef CONFIG_SENSORS_CAPSENSE
/* CapSense platform data */
static struct capsense_platform_data capsense_data = {
	.intr = PM8058_GPIO_PM_TO_SYS(FLYER_CSA_INTz_XC),
};

static struct i2c_board_info i2c_capsense_devices_XC[] = {
	{
		I2C_BOARD_INFO(CAPSENSE_NAME, 0x40 >> 1),
		.platform_data = &capsense_data,
		.irq = MSM_GPIO_TO_INT(PM8058_GPIO_PM_TO_SYS(FLYER_CSA_INTz_XC))
	},
};
#endif

static ssize_t flyer_virtual_keys_pen_show(struct kobject *kobj,
				       struct kobj_attribute *attr, char *buf)
{
	return sprintf(buf,
		__stringify(EV_KEY) ":" __stringify(KEY_RIGHTCTRL)":550:1050:90:70"
		":" __stringify(EV_KEY) ":" __stringify(KEY_RIGHTCTRL)":-35:1000:90:70"
		"\n");
}

static struct kobj_attribute flyer_virtual_key_pen_attr = {
	.attr = {
		.name = "virtualkeys.Ntrig-Pen-touchscreen",
		.mode = S_IRUGO,
	},
	.show = flyer_virtual_keys_pen_show,
};

static struct attribute *flyer_properties_attrs[] = {
	&flyer_virtual_key_pen_attr.attr,
	NULL
};

static struct attribute_group flyer_properties_attr_group = {
	.attrs = flyer_properties_attrs,
};

static struct i2c_board_info i2c_devices_microp[] = {
	{
		I2C_BOARD_INFO(MICROP_I2C_NAME, 0xCC >> 1),
		.platform_data = &microp_data,
		.irq = MSM_GPIO_TO_INT(FLYER_GPIO_UP_INT_N)
	},
};

static struct i2c_board_info i2c_compass_devices_XA_XB[] = {
	{
		I2C_BOARD_INFO(AKM8975_I2C_NAME, 0x1A >> 1),
		.platform_data = &compass_platform_data_XA_XB,
		.irq = MSM_GPIO_TO_INT(FLYER_GPIO_COMPASS_INT_XA_XB),
	},
};

static struct i2c_board_info i2c_compass_devices_XC[] = {
	{
		I2C_BOARD_INFO(AKM8975_I2C_NAME, 0x1A >> 1),
		.platform_data = &compass_platform_data_XC,
		.irq = MSM_GPIO_TO_INT(FLYER_GPIO_COMPASS_INT_XC),
	},
};

static struct i2c_board_info i2c_compass_devices_XD[] = {
	{
		I2C_BOARD_INFO(AKM8975_I2C_NAME, 0x1A >> 1),
		.platform_data = &compass_platform_data_XD,
		.irq = MSM_GPIO_TO_INT(PM8058_GPIO_PM_TO_SYS(FLYER_GPIO_COMPASS_INT_XD)),
	},
};

static struct i2c_board_info i2c_compass_devices_VER_A[] = {
	{
		I2C_BOARD_INFO(AKM8975_I2C_NAME, 0x1A >> 1),
		.platform_data = &compass_platform_data_VER_A,
		.irq = MSM_GPIO_TO_INT(PM8058_GPIO_PM_TO_SYS(FLYER_GPIO_COMPASS_INT_XD)),
	},
};

static int pm8058_gpios_init(void)
{
	int rc;
	struct pm8xxx_gpio_init_info csa_intz_xc = {
		PM8058_GPIO_PM_TO_SYS(FLYER_CSA_INTz_XC),
		{
			.direction      = PM_GPIO_DIR_IN,
			.output_buffer  = 0,
			.output_value   = 0,
			.pull           = PM_GPIO_PULL_UP_31P5,
			.vin_sel        = PM8058_GPIO_VIN_L5,
			.out_strength   = 0,
			.function       = PM_GPIO_FUNC_NORMAL,
			.inv_int_pol    = 0,
		},
	};

	struct pm8xxx_gpio_init_info tp_rstz = {
		PM8058_GPIO_PM_TO_SYS(FLYER_TP_RSTz),
		{
			.direction      = PM_GPIO_DIR_IN,
			.output_buffer  = 0,
			.output_value   = 0,
			.pull           = PM_GPIO_PULL_DN,
			.vin_sel        = PM8058_GPIO_VIN_BB,
			.out_strength   = 0,
			.function       = PM_GPIO_FUNC_NORMAL,
			.inv_int_pol    = 0,
		},
	};
	
#ifdef CONFIG_MMC_MSM_CARD_HW_DETECTION
	struct pm8xxx_gpio_init_info sdcc_det = {
		PM8058_GPIO_PM_TO_SYS(FLYER_SDMC_CD_N),
		{
			.direction      = PM_GPIO_DIR_IN,
			.pull           = PM_GPIO_PULL_UP_31P5,
			.vin_sel        = PM8058_GPIO_VIN_L5,
			.function       = PM_GPIO_FUNC_NORMAL,
			.inv_int_pol    = 0,
		},
	};
#endif

/*
	struct pm8xxx_gpio_init_info vol_up = {
		PM8058_GPIO_PM_TO_SYS(FLYER_VOL_UP),
		{
			.direction      = PM_GPIO_DIR_IN,
			.output_buffer  = 0,
			.output_value   = 0,
			.pull           = PM_GPIO_PULL_UP_31P5,
			.vin_sel        = PM8058_GPIO_VIN_S3,
			.out_strength   = 0,
			.function       = PM_GPIO_FUNC_NORMAL,
			.inv_int_pol    = 0,
		},
	};
	
	struct pm8xxx_gpio_init_info vol_dn = {
		PM8058_GPIO_PM_TO_SYS(FLYER_VOL_DN),
		{
			.direction      = PM_GPIO_DIR_IN,
			.output_buffer  = 0,
			.output_value   = 0,
			.pull           = PM_GPIO_PULL_UP_31P5,
			.vin_sel        = PM8058_GPIO_VIN_S3,
			.out_strength   = 0,
			.function       = PM_GPIO_FUNC_NORMAL,
			.inv_int_pol    = 0,
		},
	};
*/
	
	struct pm8xxx_gpio_init_info headset = {
		PM8058_GPIO_PM_TO_SYS(FLYER_AUD_HP_DETz),
		{
			.direction      = PM_GPIO_DIR_IN,
			.output_buffer  = 0,
			.output_value   = 0,
			.pull           = PM_GPIO_PULL_UP_31P5,
			.vin_sel        = PM8058_GPIO_VIN_S3,
			.out_strength   = 0,
			.function       = PM_GPIO_FUNC_NORMAL,
			.inv_int_pol    = 0,
		},
	};
	
	struct pm8xxx_gpio_init_info h2w_cable_in1 = {
		PM8058_GPIO_PM_TO_SYS(FLYER_H2W_CABLE_IN1),
		{
			.direction      = PM_GPIO_DIR_IN,
			.output_buffer  = 0,
			.output_value   = 0,
			.pull           = PM_GPIO_PULL_NO,
			.vin_sel        = PM8058_GPIO_VIN_S3,
			.out_strength   = 0,
			.function       = PM_GPIO_FUNC_NORMAL,
			.inv_int_pol    = 0,
		},
	};
	
	struct pm8xxx_gpio_init_info h2w_cable_in2 = {
		PM8058_GPIO_PM_TO_SYS(FLYER_H2W_CABLE_IN2),
		{
			.direction      = PM_GPIO_DIR_IN,
			.output_buffer  = 0,
			.output_value   = 0,
			.pull           = PM_GPIO_PULL_NO,
			.vin_sel        = PM8058_GPIO_VIN_S3,
			.out_strength   = 0,
			.function       = PM_GPIO_FUNC_NORMAL,
			.inv_int_pol    = 0,
		},
	};
	
	struct pm8xxx_gpio_init_info h2w_io1_clk = {
		PM8058_GPIO_PM_TO_SYS(FLYER_H2W_IO1_CLK),
		{
			.direction      = PM_GPIO_DIR_IN,
			.output_buffer  = 0,
			.output_value   = 0,
			.pull           = PM_GPIO_PULL_NO,
			.vin_sel        = PM8058_GPIO_VIN_S3,
			.out_strength   = 0,
			.function       = PM_GPIO_FUNC_NORMAL,
			.inv_int_pol    = 0,
		},
	};
	
	struct pm8xxx_gpio_init_info h2w_io2_dat = {
		PM8058_GPIO_PM_TO_SYS(FLYER_H2W_IO2_DAT),
		{
			.direction      = PM_GPIO_DIR_IN,
			.output_buffer  = 0,
			.output_value   = 0,
			.pull           = PM_GPIO_PULL_NO,
			.vin_sel        = PM8058_GPIO_VIN_S3,
			.out_strength   = 0,
			.function       = PM_GPIO_FUNC_NORMAL,
			.inv_int_pol    = 0,
		},
	};
	
	struct pm8xxx_gpio_init_info uart_en = {
		PM8058_GPIO_PM_TO_SYS(FLYER_UART_EN),
		{
			.direction      = PM_GPIO_DIR_IN,
			.output_buffer  = 0,
			.output_value   = 0,
			.pull           = PM_GPIO_PULL_NO,
			.vin_sel        = PM8058_GPIO_VIN_S3,
			.out_strength   = 0,
			.function       = PM_GPIO_FUNC_NORMAL,
			.inv_int_pol    = 0,
			},
	};

	struct pm8xxx_gpio_init_info pen_led3 = {
		PM8058_GPIO_PM_TO_SYS(FLYER_PEN_LED3),
		{
			.direction      = PM_GPIO_DIR_IN,
			.output_buffer  = 0,
			.output_value   = 0,
			.pull           = PM_GPIO_PULL_NO,
			.vin_sel        = PM8058_GPIO_VIN_L6,
			.out_strength   = 0,
			.function       = PM_GPIO_FUNC_NORMAL,
			.inv_int_pol    = 0,
		},
	};

	struct pm8xxx_gpio_init_info ac_detect = {
		PM8058_GPIO_PM_TO_SYS(FLYER_9V_AC_DETECT),
		{
			.direction      = PM_GPIO_DIR_OUT,
			.output_buffer  = PM_GPIO_OUT_BUF_CMOS,
			.output_value   = 0,
			.pull           = PM_GPIO_PULL_DN,
			.vin_sel        = PM8058_GPIO_VIN_L6,
			.out_strength   = 0,
			.function       = PM_GPIO_FUNC_NORMAL,
			.inv_int_pol    = 0,
		},
	};

if (system_rev >= 2) {
	rc = pm8xxx_gpio_config(csa_intz_xc.gpio, &csa_intz_xc.config); // 1
	if (rc) {
		printk(KERN_ERR "%s CSA_INTz_XC config failed\n", __func__);
		return rc;
	} else
		printk(KERN_ERR "%s CSA_INTz_XC config ok\n", __func__);
}

	rc = pm8xxx_gpio_config(tp_rstz.gpio, &tp_rstz.config); // 2
	if (rc) {
		printk(KERN_ERR "%s TP_RSTz config failed\n", __func__);
		return rc;
	} else
	  printk(KERN_ERR "%s TP_RSTz config ok\n", __func__);

#ifdef CONFIG_MMC_MSM_CARD_HW_DETECTION
	rc = pm8xxx_gpio_config(sdcc_det.gpio, &sdcc_det.config); // 3
	if (rc) {
		printk(KERN_ERR "%s SDMC_CD_N config failed\n", __func__);
		return rc;
	} else
	  printk(KERN_ERR "%s SDMC_CD_N config ok\n", __func__);
#endif

/*
	rc = pm8xxx_gpio_config(vol_up.gpio, &vol_up.config); // 4
	if (rc) {
		printk(KERN_ERR "%s VOL_UP config failed\n", __func__);
		return rc;
	} else
	  printk(KERN_ERR "%s VOL_UP config ok\n", __func__);

	rc = pm8xxx_gpio_config(vol_dn.gpio, &vol_dn.config); // 5
	if (rc) {
		printk(KERN_ERR "%s VOL_DN config failed\n", __func__);
		return rc;
	} else
	  printk(KERN_ERR "%s VOL_DN config ok\n", __func__);
*/

	rc = pm8xxx_gpio_config(headset.gpio, &headset.config); // 6
	if (rc) {
		printk(KERN_ERR "%s AUD_HP_DETz config failed\n", __func__);
		return rc;
	} else
	  printk(KERN_ERR "%s AUD_HP_DETz config ok\n", __func__);

	rc = pm8xxx_gpio_config(h2w_cable_in1.gpio, &h2w_cable_in1.config); // 7
	if (rc) {
		printk(KERN_ERR "%s H2W_CABLE_IN1 config failed\n", __func__);
		return rc;
	} else
	  printk(KERN_ERR "%s H2W_CABLE_IN1 config ok\n", __func__);

	rc = pm8xxx_gpio_config(h2w_cable_in2.gpio, &h2w_cable_in2.config); // 8
	if (rc) {
		printk(KERN_ERR "%s H2W_CABLE_IN2 config failed\n", __func__);
		return rc;
	} else
	  printk(KERN_ERR "%s H2W_CABLE_IN2 config ok\n", __func__);

	rc = pm8xxx_gpio_config(h2w_io1_clk.gpio, &h2w_io1_clk.config); // 9
	if (rc) {
		printk(KERN_ERR "%s H2W_IO1_CLK config failed\n", __func__);
		return rc;
	} else
	  printk(KERN_ERR "%s H2W_IO1_CLK config ok\n", __func__);

	rc = pm8xxx_gpio_config(h2w_io2_dat.gpio, &h2w_io2_dat.config); // 10
	if (rc) {
		printk(KERN_ERR "%s H2W_IO2_DAT config failed\n", __func__);
		return rc;
	} else
	  printk(KERN_ERR "%s H2W_IO2_DAT config ok\n", __func__);

if (system_rev >= 2) {
	rc = pm8xxx_gpio_config(uart_en.gpio, &uart_en.config); // 11
	if (rc) {
		printk(KERN_ERR "%s UART_EN config failed\n", __func__);
		return rc;
	} else
	printk(KERN_ERR "%s UART_EN config ok\n", __func__);
}

if (system_rev >= 3) {/* for Led XD board */	
	rc = pm8xxx_gpio_config(pen_led3.gpio, &pen_led3.config); // 12
	if (rc) {
		printk(KERN_ERR "%s PEN_LED3 config failed\n", __func__);
		return rc;
	} else
	printk(KERN_ERR "%s PEN_LED3 config ok\n", __func__);
}
	
	rc = pm8xxx_gpio_config(ac_detect.gpio, &ac_detect.config); // 13
	if (rc) {
		printk(KERN_ERR "%s 9V_AC_DETECT config failed\n", __func__);
		return rc;
	} else
	  printk(KERN_ERR "%s 9V_AC_DETECT config ok\n", __func__);

	return 0;
}
/* Regulator API support */

#ifdef CONFIG_MSM_PROC_COMM_REGULATOR
static struct platform_device msm_proccomm_regulator_dev = {
	.name = PROCCOMM_REGULATOR_DEV_NAME,
	.id   = -1,
	.dev  = {
		.platform_data = &msm7x30_proccomm_regulator_data
	}
};
#endif

static int pm8058_pwm_config(struct pwm_device *pwm, int ch, int on)
{
	struct pm_gpio pwm_gpio_config = {
		.direction      = PM_GPIO_DIR_OUT,
		.output_buffer  = PM_GPIO_OUT_BUF_CMOS,
		.output_value   = 0,
		.pull           = PM_GPIO_PULL_NO,
		.vin_sel        = PM8058_GPIO_VIN_S3,
		.out_strength   = PM_GPIO_STRENGTH_HIGH,
		.function       = PM_GPIO_FUNC_2,
	};
	int	rc = -EINVAL;
	int	id, mode, max_mA;

	id = mode = max_mA = 0;
	switch (ch) {
	case 0:
	case 1:
	case 2:
		if (on) {
			id = 24 + ch;
			rc = pm8xxx_gpio_config(PM8058_GPIO_PM_TO_SYS(id - 1),
							&pwm_gpio_config);
			if (rc)
				pr_err("%s: pm8xxx_gpio_config(%d): rc=%d\n",
				       __func__, id, rc);
		}
		break;

	case 3:
		id = PM_PWM_LED_KPD;
		mode = PM_PWM_CONF_DTEST3;
		max_mA = 200;
		break;

	case 4:
		id = PM_PWM_LED_0;
		mode = PM_PWM_CONF_PWM1;
		max_mA = 40;
		break;

	case 5:
		id = PM_PWM_LED_2;
		mode = PM_PWM_CONF_PWM2;
		max_mA = 40;
		break;

	case 6:
		id = PM_PWM_LED_FLASH;
		mode = PM_PWM_CONF_DTEST3;
		max_mA = 200;
		break;

	default:
		break;
	}

	if (ch >= 3 && ch <= 6) {
		if (!on) {
			mode = PM_PWM_CONF_NONE;
			max_mA = 0;
		}
		rc = pm8058_pwm_config_led(pwm, id, mode, max_mA);
		if (rc)
			pr_err("%s: pm8058_pwm_config_led(ch=%d): rc=%d\n",
			       __func__, ch, rc);
	}

	return rc;
}

static int pm8058_pwm_enable(struct pwm_device *pwm, int ch, int on)
{
	int	rc;

	switch (ch) {
	case 7:
		rc = pm8058_pwm_set_dtest(pwm, on);
		if (rc)
			pr_err("%s: pwm_set_dtest(%d): rc=%d\n",
			       __func__, on, rc);
		break;
	default:
		rc = -EINVAL;
		break;
	}
	return rc;
}

static struct pm8xxx_vibrator_platform_data pm8058_vib_pdata = {
       .initial_vibrate_ms  = 0,
       .level_mV = 3000,
       .max_timeout_ms = 15000,
};
static struct pm8058_pwm_pdata pm8058_pwm_data = {
	.config         = pm8058_pwm_config,
	.enable         = pm8058_pwm_enable,
};

static struct pm8xxx_irq_platform_data pm8xxx_irq_pdata = {
	.irq_base		= PMIC8058_IRQ_BASE,
	.devirq			= MSM_GPIO_TO_INT(PMIC_GPIO_INT),
	.irq_trigger_flag       = IRQF_TRIGGER_LOW,
};

static struct pm8xxx_gpio_platform_data pm8xxx_gpio_pdata = {
	.gpio_base		= PM8058_GPIO_PM_TO_SYS(0),
};

static struct pm8xxx_mpp_platform_data pm8xxx_mpp_pdata = {
	.mpp_base	= PM8058_MPP_PM_TO_SYS(0),
};

static struct pm8058_platform_data pm8058_7x30_data = {
	.irq_pdata		= &pm8xxx_irq_pdata,
	.gpio_pdata		= &pm8xxx_gpio_pdata,
	.mpp_pdata		= &pm8xxx_mpp_pdata,
	.pwm_pdata		= &pm8058_pwm_data,
	.vibrator_pdata		= &pm8058_vib_pdata,
};

#ifdef CONFIG_MSM_SSBI
static struct msm_ssbi_platform_data msm7x30_ssbi_pm8058_pdata __devinitdata = {
	.controller_type = MSM_SBI_CTRL_PMIC_ARBITER,
	.slave	= {
		.name			= "pm8058-core",
		.irq = MSM_GPIO_TO_INT(PMIC_GPIO_INT),
		.platform_data		= &pm8058_7x30_data,
	},
	.rspinlock_name	= "D:PMIC_SSBI",
};
#endif

static struct i2c_board_info msm_camera_boardinfo[] __initdata = {
#ifdef CONFIG_S5K4E1GX
	{
		I2C_BOARD_INFO("s5k4e1gx", 0x20 >> 1),
	},
#endif
#ifdef CONFIG_S5K6AAFX
	{
		I2C_BOARD_INFO("s5k6aafx", 0x78 >> 1),
	},
	{
		I2C_BOARD_INFO("s5k6aafx", 0x5A >> 1),/*for cob,*/
	},
#endif
};

/* default GPIO setting of Camera */
static int gCAM_RST_GPIO = FLYER_CAM_RST;
static int gCAM2_PWN_GPIO = FLYER_CAM2_PWD;
static int gCAM_GPIO_SEL = FLYER_CLK_SWITCH;

#ifdef CONFIG_MSM_CAMERA
static uint32_t camera_off_gpio_table[] = {
	/* parallel CAMERA interfaces */
PCOM_GPIO_CFG(FLYER_CAM_RST,  0, GPIO_OUTPUT, GPIO_PULL_DOWN, GPIO_2MA),
PCOM_GPIO_CFG(FLYER_CAM_PWD,  0, GPIO_OUTPUT, GPIO_PULL_DOWN, GPIO_2MA),
PCOM_GPIO_CFG(FLYER_CAM2_RST,  0, GPIO_OUTPUT, GPIO_PULL_DOWN, GPIO_2MA),
PCOM_GPIO_CFG(FLYER_CAM2_PWD,  0, GPIO_OUTPUT, GPIO_PULL_DOWN, GPIO_2MA),

	PCOM_GPIO_CFG(2,  0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_4MA), /* DAT2 */
	PCOM_GPIO_CFG(3,  0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_4MA), /* DAT3 */
	PCOM_GPIO_CFG(4,  0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_4MA), /* DAT4 */
	PCOM_GPIO_CFG(5,  0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_4MA), /* DAT5 */
	PCOM_GPIO_CFG(6,  0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_4MA), /* DAT6 */
	PCOM_GPIO_CFG(7,  0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_4MA), /* DAT7 */
	PCOM_GPIO_CFG(8,  0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_4MA), /* DAT8 */
	PCOM_GPIO_CFG(9,  0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_4MA), /* DAT9 */
	PCOM_GPIO_CFG(10, 0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_4MA), /* DAT10 */
	PCOM_GPIO_CFG(11, 0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_4MA), /* DAT11 */
	PCOM_GPIO_CFG(12, 0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_4MA), /* PCLK */
	PCOM_GPIO_CFG(13, 0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_4MA), /* HSYNC_IN */
	PCOM_GPIO_CFG(14, 0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_4MA), /* VSYNC_IN */
	PCOM_GPIO_CFG(15, 0, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA), /* MCLK */

};

static uint32_t camera_on_gpio_table[] = {
	/* parallel CAMERA interfaces */
PCOM_GPIO_CFG(FLYER_CAM_RST,   0, GPIO_OUTPUT, GPIO_PULL_DOWN, GPIO_2MA),
PCOM_GPIO_CFG(FLYER_CAM_PWD,   0, GPIO_OUTPUT, GPIO_PULL_DOWN, GPIO_2MA),
PCOM_GPIO_CFG(FLYER_CAM2_RST,  0, GPIO_OUTPUT, GPIO_PULL_DOWN, GPIO_2MA),
PCOM_GPIO_CFG(FLYER_CAM2_PWD,  0, GPIO_OUTPUT, GPIO_PULL_DOWN, GPIO_2MA),

	PCOM_GPIO_CFG(2,  1, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), /* DAT2 */
	PCOM_GPIO_CFG(3,  1, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), /* DAT3 */
	PCOM_GPIO_CFG(4,  1, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), /* DAT4 */
	PCOM_GPIO_CFG(5,  1, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), /* DAT5 */
	PCOM_GPIO_CFG(6,  1, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), /* DAT6 */
	PCOM_GPIO_CFG(7,  1, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), /* DAT7 */
	PCOM_GPIO_CFG(8,  1, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), /* DAT8 */
	PCOM_GPIO_CFG(9,  1, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), /* DAT9 */
	PCOM_GPIO_CFG(10, 1, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), /* DAT10 */
	PCOM_GPIO_CFG(11, 1, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), /* DAT11 */
	PCOM_GPIO_CFG(12, 1, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), /* PCLK */
	PCOM_GPIO_CFG(13, 1, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), /* HSYNC_IN */
	PCOM_GPIO_CFG(14, 1, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), /* VSYNC_IN */
	PCOM_GPIO_CFG(15, 1, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_16MA), /* MCLK */

};

/* for XC board */
static uint32_t camera_off_gpio_table_XC[] = {
	/* parallel CAMERA interfaces */
PCOM_GPIO_CFG(FLYER_CAM_RST_XC,  0, GPIO_OUTPUT, GPIO_PULL_DOWN, GPIO_2MA),
PCOM_GPIO_CFG(FLYER_CAM_PWD,  0, GPIO_OUTPUT, GPIO_PULL_DOWN, GPIO_2MA),
PCOM_GPIO_CFG(FLYER_CAM2_RST,  0, GPIO_OUTPUT, GPIO_PULL_DOWN, GPIO_2MA),
PCOM_GPIO_CFG(FLYER_CAM2_PWD_XC,  0, GPIO_OUTPUT, GPIO_PULL_DOWN, GPIO_2MA),

	PCOM_GPIO_CFG(2,  0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_4MA), /* DAT2 */
	PCOM_GPIO_CFG(3,  0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_4MA), /* DAT3 */
	PCOM_GPIO_CFG(4,  0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_4MA), /* DAT4 */
	PCOM_GPIO_CFG(5,  0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_4MA), /* DAT5 */
	PCOM_GPIO_CFG(6,  0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_4MA), /* DAT6 */
	PCOM_GPIO_CFG(7,  0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_4MA), /* DAT7 */
	PCOM_GPIO_CFG(8,  0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_4MA), /* DAT8 */
	PCOM_GPIO_CFG(9,  0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_4MA), /* DAT9 */
	PCOM_GPIO_CFG(10, 0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_4MA), /* DAT10 */
	PCOM_GPIO_CFG(11, 0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_4MA), /* DAT11 */
	PCOM_GPIO_CFG(12, 0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_4MA), /* PCLK */
	PCOM_GPIO_CFG(13, 0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_4MA),
	PCOM_GPIO_CFG(14, 0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_4MA),
	PCOM_GPIO_CFG(15, 0, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA), /* MCLK */

};

/* for XC board */
static uint32_t camera_on_gpio_table_XC[] = {
	/* parallel CAMERA interfaces */
PCOM_GPIO_CFG(FLYER_CAM_RST_XC,   0, GPIO_OUTPUT, GPIO_PULL_DOWN, GPIO_2MA),
PCOM_GPIO_CFG(FLYER_CAM_PWD,   0, GPIO_OUTPUT, GPIO_PULL_DOWN, GPIO_2MA),
PCOM_GPIO_CFG(FLYER_CAM2_RST,  0, GPIO_OUTPUT, GPIO_PULL_DOWN, GPIO_2MA),
PCOM_GPIO_CFG(FLYER_CAM2_PWD_XC,  0, GPIO_OUTPUT, GPIO_PULL_DOWN, GPIO_2MA),

	PCOM_GPIO_CFG(2,  1, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), /* DAT2 */
	PCOM_GPIO_CFG(3,  1, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), /* DAT3 */
	PCOM_GPIO_CFG(4,  1, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), /* DAT4 */
	PCOM_GPIO_CFG(5,  1, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), /* DAT5 */
	PCOM_GPIO_CFG(6,  1, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), /* DAT6 */
	PCOM_GPIO_CFG(7,  1, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), /* DAT7 */
	PCOM_GPIO_CFG(8,  1, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), /* DAT8 */
	PCOM_GPIO_CFG(9,  1, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), /* DAT9 */
	PCOM_GPIO_CFG(10, 1, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), /* DAT10 */
	PCOM_GPIO_CFG(11, 1, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), /* DAT11 */
	PCOM_GPIO_CFG(12, 1, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), /* PCLK */
	PCOM_GPIO_CFG(13, 1, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA),
	PCOM_GPIO_CFG(14, 1, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA),
	PCOM_GPIO_CFG(15, 1, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_16MA), /* MCLK */

};

/* for XD board : change FLYER_CAM2_PWD*/
static uint32_t camera_off_gpio_table_XD[] = {
	/* parallel CAMERA interfaces */
PCOM_GPIO_CFG(FLYER_CAM_RST_XC,  0, GPIO_OUTPUT, GPIO_PULL_DOWN, GPIO_2MA),
PCOM_GPIO_CFG(FLYER_CAM_PWD,  0, GPIO_OUTPUT, GPIO_PULL_DOWN, GPIO_2MA),
PCOM_GPIO_CFG(FLYER_CAM2_RST,  0, GPIO_OUTPUT, GPIO_PULL_DOWN, GPIO_2MA),
PCOM_GPIO_CFG(FLYER_CAM2_PWD_XD,  0, GPIO_OUTPUT, GPIO_PULL_DOWN, GPIO_2MA),

	PCOM_GPIO_CFG(2,  0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_4MA), /* DAT2 */
	PCOM_GPIO_CFG(3,  0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_4MA), /* DAT3 */
	PCOM_GPIO_CFG(4,  0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_4MA), /* DAT4 */
	PCOM_GPIO_CFG(5,  0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_4MA), /* DAT5 */
	PCOM_GPIO_CFG(6,  0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_4MA), /* DAT6 */
	PCOM_GPIO_CFG(7,  0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_4MA), /* DAT7 */
	PCOM_GPIO_CFG(8,  0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_4MA), /* DAT8 */
	PCOM_GPIO_CFG(9,  0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_4MA), /* DAT9 */
	PCOM_GPIO_CFG(10, 0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_4MA), /* DAT10 */
	PCOM_GPIO_CFG(11, 0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_4MA), /* DAT11 */
	PCOM_GPIO_CFG(12, 0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_4MA), /* PCLK */
	PCOM_GPIO_CFG(13, 0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_4MA),
	PCOM_GPIO_CFG(14, 0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_4MA),
	PCOM_GPIO_CFG(15, 0, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA), /* MCLK */

};

/* for XD board  : change FLYER_CAM2_PWD*/
static uint32_t camera_on_gpio_table_XD[] = {
	/* parallel CAMERA interfaces */
PCOM_GPIO_CFG(FLYER_CAM_RST_XC,   0, GPIO_OUTPUT, GPIO_PULL_DOWN, GPIO_2MA),
PCOM_GPIO_CFG(FLYER_CAM_PWD,   0, GPIO_OUTPUT, GPIO_PULL_DOWN, GPIO_2MA),
PCOM_GPIO_CFG(FLYER_CAM2_RST,  0, GPIO_OUTPUT, GPIO_PULL_DOWN, GPIO_2MA),
PCOM_GPIO_CFG(FLYER_CAM2_PWD_XD,  0, GPIO_OUTPUT, GPIO_PULL_DOWN, GPIO_2MA),

	PCOM_GPIO_CFG(2,  1, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), /* DAT2 */
	PCOM_GPIO_CFG(3,  1, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), /* DAT3 */
	PCOM_GPIO_CFG(4,  1, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), /* DAT4 */
	PCOM_GPIO_CFG(5,  1, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), /* DAT5 */
	PCOM_GPIO_CFG(6,  1, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), /* DAT6 */
	PCOM_GPIO_CFG(7,  1, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), /* DAT7 */
	PCOM_GPIO_CFG(8,  1, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), /* DAT8 */
	PCOM_GPIO_CFG(9,  1, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), /* DAT9 */
	PCOM_GPIO_CFG(10, 1, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), /* DAT10 */
	PCOM_GPIO_CFG(11, 1, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), /* DAT11 */
	PCOM_GPIO_CFG(12, 1, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), /* PCLK */
	PCOM_GPIO_CFG(13, 1, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA),
	PCOM_GPIO_CFG(14, 1, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA),
	PCOM_GPIO_CFG(15, 1, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_16MA), /* MCLK */

};

static int sensor_power_enable(char *power, unsigned volt)
{
	struct vreg *vreg_gp;
	int rc;

	if (power == NULL)
		return EIO;

	vreg_gp = vreg_get(NULL, power);
	if (IS_ERR(vreg_gp)) {
		pr_err("[CAM] %s: vreg_get(%s) failed (%ld)\n",
			__func__, power, PTR_ERR(vreg_gp));
		return EIO;
	}

	rc = vreg_set_level(vreg_gp, volt);
	if (rc) {
		pr_err("[CAM] %s: vreg wlan set %s level failed (%d)\n",
			__func__, power, rc);
		return EIO;
	}

	rc = vreg_enable(vreg_gp);
	if (rc) {
		pr_err("[CAM] %s: vreg enable %s failed (%d)\n",
			__func__, power, rc);
		return EIO;
	}
	return rc;
}

static int sensor_power_disable(char *power)
{
	struct vreg *vreg_gp;
	int rc;
	vreg_gp = vreg_get(NULL, power);
	if (IS_ERR(vreg_gp)) {
		pr_err("[CAM] %s: vreg_get(%s) failed (%ld)\n",
			__func__, power, PTR_ERR(vreg_gp));
		return EIO;
	}

	rc = vreg_disable(vreg_gp);
	if (rc) {
		pr_err("[CAM] %s: vreg disable %s failed (%d)\n",
			__func__, power, rc);
		return EIO;
	}
	return rc;
}

#if defined(CONFIG_S5K4E1GX) || defined(CONFIG_S5K6AAFX)
static int sensor_vreg_on(void)
{
	int rc;
	pr_info("[CAM] %s camera vreg on\n", __func__);

	/*camera power down*/
#if 0
	gpio_set_value(CAM_PWD, 1);
	udelay(200);
#endif

	/*camera VCM power*/
	rc = sensor_power_enable("gp9", 2850);
	pr_info("[CAM] sensor_power_enable(\"gp9\", 2850) == %d\n", rc);

	udelay(150);

	/*camera analog power*/
	/* This delay is just temporary work-around,*/
	/*and will remove when HW power team fix */
	/*the power up two stage problem with pmic */

	if (system_rev >= 3) { /* for XD board */
		rc = sensor_power_enable("gp6", 2850);
		pr_info("[CAM] sensor_power_enable(\"gp6\", 2850) == %d\n", rc);
	} else {
		rc = sensor_power_enable("gp4", 2850);
		pr_info("[CAM] sensor_power_enable(\"gp4\", 2850) == %d\n", rc);
	}
	udelay(150);

	/*camera digital power*/
	rc = sensor_power_enable("wlan", 1800);
	pr_info("[CAM] sensor_power_enable(\"wlan\", 1800) == %d\n", rc);

	udelay(150);

	/*camera IO power*/
	rc = sensor_power_enable("gp2", 1800);
	pr_info("[CAM] sensor_power_enable(\"gp2\", 1800) == %d\n", rc);


	return rc;
}

static int sensor_vreg_off(void)
{
	int rc;
	pr_info("[CAM] %s camera vreg off\n", __func__);
	/*camera analog power*/
	if (system_rev >= 3) { /* for XD board */
		rc = sensor_power_disable("gp6");
		pr_info("[CAM] sensor_power_disable(\"gp6\") == %d\n", rc);
	} else {
		rc = sensor_power_disable("gp4");
		pr_info("[CAM] sensor_power_disable(\"gp4\") == %d\n", rc);
	}

	/*camera digital power*/
	rc = sensor_power_disable("wlan");
	pr_info("[CAM] sensor_power_disable(\"wlan\") == %d\n", rc);
	/*camera IO power*/
	rc = sensor_power_disable("gp2");
	pr_info("[CAM] sensor_power_disable(\"gp2\") == %d\n", rc);
	/*camera VCM power*/
	rc = sensor_power_disable("gp9");
	pr_info("[CAM] sensor_power_disable(\"gp9\") == %d\n", rc);
	return rc;
}
#endif

static int config_camera_on_gpios(void)
{
	pr_info("[CAM] config_camera_on_gpios\n");
	if (system_rev >= 3) /* for XD board */
	config_gpio_table(camera_on_gpio_table_XD,
		ARRAY_SIZE(camera_on_gpio_table_XD));
	else if (system_rev == 2) /* for XC board */
	config_gpio_table(camera_on_gpio_table_XC,
		ARRAY_SIZE(camera_on_gpio_table_XC));
	else
	config_gpio_table(camera_on_gpio_table,
		ARRAY_SIZE(camera_on_gpio_table));
	return 0;
}

static void config_camera_off_gpios(void)
{
	pr_info("[CAM] config_camera_off_gpios\n");
	if (system_rev >= 3) /* for XD board */
	config_gpio_table(camera_off_gpio_table_XD,
		ARRAY_SIZE(camera_off_gpio_table_XD));
	else if (system_rev == 2) /* for XC board */
	config_gpio_table(camera_off_gpio_table_XC,
		ARRAY_SIZE(camera_off_gpio_table_XC));
	else
	config_gpio_table(camera_off_gpio_table,
		ARRAY_SIZE(camera_off_gpio_table));
}

struct resource msm_camera_resources[] = {
	{
		.start	= 0xA6000000,
		.end	= 0xA6000000 + SZ_1M - 1,
		.flags	= IORESOURCE_MEM,
	},
	{
		.start	= INT_VFE,
		.end	= INT_VFE,
		.flags	= IORESOURCE_IRQ,
	},
};

struct msm_camera_device_platform_data camera_device_data = {
  .camera_gpio_on  = config_camera_on_gpios,
  .camera_gpio_off = config_camera_off_gpios,
  .ioext.mdcphy = MSM_MDC_PHYS,
  .ioext.mdcsz  = MSM_MDC_SIZE,
  .ioext.appphy = MSM_CLK_CTL_PHYS,
  .ioext.appsz  = MSM_CLK_CTL_SIZE,
  .ioext.camifpadphy = 0xAB000000,
  .ioext.camifpadsz  = 0x00000400,
  .ioext.csiphy = 0xA6100000,
  .ioext.csisz  = 0x00000400,
  .ioext.csiirq = INT_CSI,
};

#ifdef CONFIG_S5K4E1GX
static void flyer_maincam_clk_switch(void){
	int rc = 0;
	pr_info("[CAM] Doing clk switch (s5k4e1gx)\n");


	rc = gpio_request(gCAM_GPIO_SEL, "s5k4e1gx");

	if (rc < 0)
		pr_err("[CAM] GPIO (%d) request fail\n", gCAM_GPIO_SEL);
	else
		gpio_direction_output(gCAM_GPIO_SEL, 0);

	gpio_free(gCAM_GPIO_SEL);

	return;
}
#endif

#ifdef CONFIG_S5K6AAFX
static void flyer_seccam_clk_switch(void){

	int rc = 0;
	pr_info("[CAM] Doing clk switch (s5k6aafx)\n");
	rc = gpio_request(gCAM_GPIO_SEL, "s5k6aafx");

	if (rc < 0)
		pr_err("[CAM] GPIO (%d) request fail\n", gCAM_GPIO_SEL);
	else
		gpio_direction_output(gCAM_GPIO_SEL, 1);

	gpio_free(gCAM_GPIO_SEL);
	return;
}
#endif

/* S5K4E1GX */
#ifdef CONFIG_S5K4E1GX
static struct msm_camera_sensor_info msm_camera_sensor_s5k4e1gx_data = {
	.sensor_name    = "s5k4e1gx",
	.sensor_reset   = FLYER_CAM_RST,
	.vcm_pwd     = FLYER_CAM_PWD,
	.camera_power_on = sensor_vreg_on,
	.camera_power_off = sensor_vreg_off,
	.camera_clk_switch = flyer_maincam_clk_switch,
	.pdata          = &camera_device_data,
	.flash_type     = MSM_CAMERA_FLASH_NONE,
	.resource       = msm_camera_resources,
	.num_resources  = ARRAY_SIZE(msm_camera_resources),
	/* .flash_cfg	= &msm_camera_sensor_flash_cfg, */
	.cam_select_pin = FLYER_CLK_SWITCH,
	.sensor_lc_disable = true, /* disable sensor lens correction */
};

static struct platform_device msm_camera_sensor_s5k4e1gx = {
  .name = "msm_camera_s5k4e1gx",
  .dev = {
    .platform_data = &msm_camera_sensor_s5k4e1gx_data,
  },
};
#endif

#ifdef CONFIG_S5K6AAFX
static struct msm_camera_sensor_info msm_camera_sensor_s5k6aafx_data = {
	.sensor_name	= "s5k6aafx",
	.sensor_reset	= FLYER_CAM2_RST,
	.sensor_pwd		= FLYER_CAM2_PWD,
	.camera_power_on = sensor_vreg_on,
	.camera_power_off = sensor_vreg_off,
	.camera_clk_switch	= flyer_seccam_clk_switch,
	.pdata		= &camera_device_data,
	.flash_type     = MSM_CAMERA_FLASH_NONE,
	.resource = msm_camera_resources,
	.num_resources = ARRAY_SIZE(msm_camera_resources),
	.power_down_disable = true, /* true: disable pwd down function */
	.full_size_preview = false, /* true: use full size preview */
	.cam_select_pin = FLYER_CLK_SWITCH,
};

static struct platform_device msm_camera_sensor_s5k6aafx = {
	.name	   = "msm_camera_s5k6aafx",
	.dev	    = {
		.platform_data = &msm_camera_sensor_s5k6aafx_data,
	},
};
#endif

#ifdef CONFIG_MSM_GEMINI
static struct resource msm_gemini_resources[] = {
	{
		.start  = 0xA3A00000,
		.end    = 0xA3A00000 + 0x0150 - 1,
		.flags  = IORESOURCE_MEM,
	},
	{
		.start  = INT_JPEG,
		.end    = INT_JPEG,
		.flags  = IORESOURCE_IRQ,
	},
};

static struct platform_device msm_gemini_device = {
	.name           = "msm_gemini",
	.resource       = msm_gemini_resources,
	.num_resources  = ARRAY_SIZE(msm_gemini_resources),
};
#endif

#ifdef CONFIG_MSM_VPE
static struct resource msm_vpe_resources[] = {
	{
		.start	= 0xAD200000,
		.end	= 0xAD200000 + SZ_1M - 1,
		.flags	= IORESOURCE_MEM,
	},
	{
		.start	= INT_VPE,
		.end	= INT_VPE,
		.flags	= IORESOURCE_IRQ,
	},
};

static struct platform_device msm_vpe_device = {
       .name = "msm_vpe",
       .id   = 0,
       .num_resources = ARRAY_SIZE(msm_vpe_resources),
       .resource = msm_vpe_resources,
};
#endif

#endif /*CONFIG_MSM_CAMERA*/

#ifdef CONFIG_MSM7KV2_AUDIO

static unsigned aux_pcm_gpio_on[] = {
	GPIO_CFG(138, 1, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),   /* PCM_DOUT */
	GPIO_CFG(139, 1, GPIO_CFG_INPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_2MA),   /* PCM_DIN  */
	GPIO_CFG(140, 1, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),   /* PCM_SYNC */
	GPIO_CFG(141, 1, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),   /* PCM_CLK  */
};

static int __init aux_pcm_gpio_init(void)
{
	int pin, rc;

	pr_info("aux_pcm_gpio_init \n");
	for (pin = 0; pin < ARRAY_SIZE(aux_pcm_gpio_on); pin++) {
		rc = gpio_tlmm_config(aux_pcm_gpio_on[pin],
					GPIO_CFG_ENABLE);
		if (rc) {
			printk(KERN_ERR
				"%s: gpio_tlmm_config(%#x)=%d\n",
				__func__, aux_pcm_gpio_on[pin], rc);
		}
	}
	return rc;
}

static struct msm_gpio mi2s_clk_gpios[] = {
	{ GPIO_CFG(145, 1, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
	    "MI2S_SCLK"},
	{ GPIO_CFG(144, 1, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
	    "MI2S_WS"},
	{ GPIO_CFG(120, 1, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
	    "MI2S_MCLK_A"},
};

static struct msm_gpio mi2s_rx_data_lines_gpios[] = {
	{ GPIO_CFG(121, 1, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
	    "MI2S_DATA_SD0_A"},
	{ GPIO_CFG(122, 1, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
	    "MI2S_DATA_SD1_A"},
	{ GPIO_CFG(123, 1, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
	    "MI2S_DATA_SD2_A"},
	{ GPIO_CFG(146, 1, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
	    "MI2S_DATA_SD3"},
};

static struct msm_gpio mi2s_tx_data_lines_gpios[] = {
	{ GPIO_CFG(146, 1, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
	    "MI2S_DATA_SD3"},
};

int mi2s_config_clk_gpio(void)
{
	int rc = 0;

	rc = msm_gpios_request_enable(mi2s_clk_gpios,
			ARRAY_SIZE(mi2s_clk_gpios));
	if (rc) {
		pr_err("%s: enable mi2s clk gpios  failed\n",
					__func__);
		return rc;
	}
	return 0;
}

int  mi2s_unconfig_data_gpio(u32 direction, u8 sd_line_mask)
{
	int i, rc = 0;
	sd_line_mask &= MI2S_SD_LINE_MASK;

	switch (direction) {
	case DIR_TX:
		msm_gpios_disable_free(mi2s_tx_data_lines_gpios, 1);
		break;
	case DIR_RX:
		i = 0;
		while (sd_line_mask) {
			if (sd_line_mask & 0x1)
				msm_gpios_disable_free(
					mi2s_rx_data_lines_gpios + i , 1);
			sd_line_mask = sd_line_mask >> 1;
			i++;
		}
		break;
	default:
		pr_err("%s: Invaild direction  direction = %u\n",
						__func__, direction);
		rc = -EINVAL;
		break;
	}
	return rc;
}

int mi2s_config_data_gpio(u32 direction, u8 sd_line_mask)
{
	int i , rc = 0;
	u8 sd_config_done_mask = 0;

	sd_line_mask &= MI2S_SD_LINE_MASK;

	switch (direction) {
	case DIR_TX:
		if ((sd_line_mask & MI2S_SD_0) || (sd_line_mask & MI2S_SD_1) ||
		   (sd_line_mask & MI2S_SD_2) || !(sd_line_mask & MI2S_SD_3)) {
			pr_err("%s: can not use SD0 or SD1 or SD2 for TX"
				".only can use SD3. sd_line_mask = 0x%x\n",
				__func__ , sd_line_mask);
			rc = -EINVAL;
		} else {
			rc = msm_gpios_request_enable(mi2s_tx_data_lines_gpios,
							 1);
			if (rc)
				pr_err("%s: enable mi2s gpios for TX failed\n",
					   __func__);
		}
		break;
	case DIR_RX:
		i = 0;
		while (sd_line_mask && (rc == 0)) {
			if (sd_line_mask & 0x1) {
				rc = msm_gpios_request_enable(
					mi2s_rx_data_lines_gpios + i , 1);
				if (rc) {
					pr_err("%s: enable mi2s gpios for"
					 "RX failed.  SD line = %s\n",
					 __func__,
					 (mi2s_rx_data_lines_gpios + i)->label);
					mi2s_unconfig_data_gpio(DIR_RX,
						sd_config_done_mask);
				} else
					sd_config_done_mask |= (1 << i);
			}
			sd_line_mask = sd_line_mask >> 1;
			i++;
		}
		break;
	default:
		pr_err("%s: Invaild direction  direction = %u\n",
			__func__, direction);
		rc = -EINVAL;
		break;
	}
	return rc;
}

int mi2s_unconfig_clk_gpio(void)
{
	msm_gpios_disable_free(mi2s_clk_gpios, ARRAY_SIZE(mi2s_clk_gpios));
	return 0;
}

#endif /* CONFIG_MSM7KV2_AUDIO */

static int __init buses_init(void)
{
	if (gpio_tlmm_config(GPIO_CFG(PMIC_GPIO_INT, 1, GPIO_CFG_INPUT,
				  GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE))
		pr_err("%s: gpio_tlmm_config (gpio=%d) failed\n",
		       __func__, PMIC_GPIO_INT);
	return 0;
}

#define TIMPANI_RESET_GPIO	1

struct bahama_config_register{
	u8 reg;
	u8 value;
	u8 mask;
};

enum version{
	VER_1_0,
	VER_2_0,
	VER_UNSUPPORTED = 0xFF
};


/* static struct vreg *vreg_marimba_1; */
static struct vreg *vreg_marimba_2;

static struct msm_gpio timpani_reset_gpio_cfg[] = {
{ GPIO_CFG(TIMPANI_RESET_GPIO, 0, GPIO_CFG_OUTPUT,
	GPIO_CFG_NO_PULL, GPIO_CFG_2MA), "timpani_reset"} };

static int config_timpani_reset(void)
{
	int rc;

	rc = msm_gpios_request_enable(timpani_reset_gpio_cfg,
				ARRAY_SIZE(timpani_reset_gpio_cfg));
	if (rc < 0) {
		printk(KERN_ERR
			"%s: msm_gpios_request_enable failed (%d)\n",
				__func__, rc);
	}
	return rc;
}

static unsigned int msm_timpani_setup_power(void)
{
	int rc;

	rc = config_timpani_reset();
	if (rc < 0)
		goto out;
#if 0
	rc = vreg_enable(vreg_marimba_1);
	if (rc) {
		printk(KERN_ERR "%s: vreg_enable() = %d\n",
					__func__, rc);
		goto out;
	}
#endif
	rc = vreg_enable(vreg_marimba_2);
	if (rc) {
		printk(KERN_ERR "%s: vreg_enable() = %d\n",
					__func__, rc);
	/*	goto fail_disable_vreg_marimba_1; */
	}

	rc = gpio_direction_output(TIMPANI_RESET_GPIO, 1);
	if (rc < 0) {
		printk(KERN_ERR
			"%s: gpio_direction_output failed (%d)\n",
				__func__, rc);
		msm_gpios_free(timpani_reset_gpio_cfg,
				ARRAY_SIZE(timpani_reset_gpio_cfg));
		vreg_disable(vreg_marimba_2);
	} else
		goto out;

#if 0
fail_disable_vreg_marimba_1:
	vreg_disable(vreg_marimba_1);
#endif
out:
	return rc;
};

static void msm_timpani_shutdown_power(void)
{
	int rc;
#if 0
	rc = vreg_disable(vreg_marimba_1);
	if (rc) {
		printk(KERN_ERR "%s: return val: %d\n",
					__func__, rc);
	}
#endif
	rc = vreg_disable(vreg_marimba_2);
	if (rc) {
		printk(KERN_ERR "%s: return val: %d\n",
					__func__, rc);
	}

	rc = gpio_direction_output(TIMPANI_RESET_GPIO, 0);
	if (rc < 0) {
		printk(KERN_ERR
			"%s: gpio_direction_output failed (%d)\n",
				__func__, rc);
	}

	msm_gpios_free(timpani_reset_gpio_cfg,
				   ARRAY_SIZE(timpani_reset_gpio_cfg));
};

static struct msm_gpio marimba_svlte_config_clock[] = {
	{ GPIO_CFG(34, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
		"MARIMBA_SVLTE_CLOCK_ENABLE" },
};

static unsigned int msm_marimba_gpio_config_svlte(int gpio_cfg_marimba)
{
	if (machine_is_msm8x55_svlte_surf() ||
		machine_is_msm8x55_svlte_ffa()) {
		if (gpio_cfg_marimba)
			gpio_set_value(GPIO_PIN
				(marimba_svlte_config_clock->gpio_cfg), 1);
		else
			gpio_set_value(GPIO_PIN
				(marimba_svlte_config_clock->gpio_cfg), 0);
	}

	return 0;
};

static unsigned int msm_marimba_setup_power(void)
{
	int rc;
#if 0
	rc = vreg_enable(vreg_marimba_1);
	if (rc) {
		printk(KERN_ERR "%s: vreg_enable() = %d \n",
					__func__, rc);
		goto out;
	}
#endif
	rc = vreg_enable(vreg_marimba_2);
	if (rc) {
		printk(KERN_ERR "%s: vreg_enable() = %d \n",
					__func__, rc);
		goto out;
	}

	if (machine_is_msm8x55_svlte_surf() || machine_is_msm8x55_svlte_ffa()) {
		rc = msm_gpios_request_enable(marimba_svlte_config_clock,
				ARRAY_SIZE(marimba_svlte_config_clock));
		if (rc < 0) {
			printk(KERN_ERR
				"%s: msm_gpios_request_enable failed (%d)\n",
					__func__, rc);
			return rc;
		}

		rc = gpio_direction_output(GPIO_PIN
			(marimba_svlte_config_clock->gpio_cfg), 0);
		if (rc < 0) {
			printk(KERN_ERR
				"%s: gpio_direction_output failed (%d)\n",
					__func__, rc);
			return rc;
		}
	}

out:
	return rc;
};

static void msm_marimba_shutdown_power(void)
{
	int rc;
#if 0
	rc = vreg_disable(vreg_marimba_1);
	if (rc) {
		printk(KERN_ERR "%s: return val: %d\n",
					__func__, rc);
	}
#endif
	rc = vreg_disable(vreg_marimba_2);
	if (rc) {
		printk(KERN_ERR "%s: return val: %d \n",
					__func__, rc);
	}
};

static int bahama_present(void)
{
	int id;
	switch (id = adie_get_detected_connectivity_type()) {
	case BAHAMA_ID:
		return 1;

	case MARIMBA_ID:
		return 0;

	case TIMPANI_ID:
	default:
	printk(KERN_ERR "%s: unexpected adie connectivity type: %d\n",
			__func__, id);
	return -ENODEV;
	}
}

struct vreg *fm_regulator;
static int fm_radio_setup(struct marimba_fm_platform_data *pdata)
{
	int rc;
	uint32_t irqcfg;
	const char *id = "FMPW";

	int bahama_not_marimba = bahama_present();

	if (bahama_not_marimba == -1) {
		printk(KERN_WARNING "%s: bahama_present: %d\n",
				__func__, bahama_not_marimba);
		return -ENODEV;
	}
	if (bahama_not_marimba)
		fm_regulator = vreg_get(NULL, "s3");
	else
		fm_regulator = vreg_get(NULL, "s2");

	if (IS_ERR(fm_regulator)) {
		printk(KERN_ERR "%s: vreg get failed (%ld)\n",
			__func__, PTR_ERR(fm_regulator));
		return -1;
	}
	if (!bahama_not_marimba) {

		rc = pmapp_vreg_level_vote(id, PMAPP_VREG_S2, 1300);

		if (rc < 0) {
			printk(KERN_ERR "%s: voltage level vote failed (%d)\n",
				__func__, rc);
			return rc;
		}
	}
	rc = vreg_enable(fm_regulator);
	if (rc) {
		printk(KERN_ERR "%s: vreg_enable() = %d\n",
					__func__, rc);
		return rc;
	}

	rc = pmapp_clock_vote(id, PMAPP_CLOCK_ID_DO,
					  PMAPP_CLOCK_VOTE_ON);
	if (rc < 0) {
		printk(KERN_ERR "%s: clock vote failed (%d)\n",
			__func__, rc);
		goto fm_clock_vote_fail;
	}
	/*Request the Clock Using GPIO34/AP2MDM_MRMBCK_EN in case
	of svlte*/
	if (machine_is_msm8x55_svlte_surf() ||
			machine_is_msm8x55_svlte_ffa())	{
		rc = marimba_gpio_config(1);
		if (rc < 0)
			printk(KERN_ERR "%s: clock enable for svlte : %d\n",
						__func__, rc);
	}
	irqcfg = GPIO_CFG(147, 0, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL,
					GPIO_CFG_2MA);
	rc = gpio_tlmm_config(irqcfg, GPIO_CFG_ENABLE);
	if (rc) {
		printk(KERN_ERR "%s: gpio_tlmm_config(%#x)=%d\n",
				__func__, irqcfg, rc);
		rc = -EIO;
		goto fm_gpio_config_fail;

	}
	return 0;
fm_gpio_config_fail:
	pmapp_clock_vote(id, PMAPP_CLOCK_ID_DO,
				  PMAPP_CLOCK_VOTE_OFF);
fm_clock_vote_fail:
	vreg_disable(fm_regulator);
	return rc;

};

static void fm_radio_shutdown(struct marimba_fm_platform_data *pdata)
{
	int rc;
	const char *id = "FMPW";
	uint32_t irqcfg = GPIO_CFG(147, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_UP,
					GPIO_CFG_2MA);

	int bahama_not_marimba = bahama_present();
	if (bahama_not_marimba == -1) {
		printk(KERN_WARNING "%s: bahama_present: %d\n",
			__func__, bahama_not_marimba);
		return;
	}

	rc = gpio_tlmm_config(irqcfg, GPIO_CFG_ENABLE);
	if (rc) {
		printk(KERN_ERR "%s: gpio_tlmm_config(%#x)=%d\n",
				__func__, irqcfg, rc);
	}
	if (fm_regulator != NULL) {
		rc = vreg_disable(fm_regulator);

		if (rc) {
			printk(KERN_ERR "%s: return val: %d\n",
					__func__, rc);
		}
		fm_regulator = NULL;
	}
	rc = pmapp_clock_vote(id, PMAPP_CLOCK_ID_DO,
					  PMAPP_CLOCK_VOTE_OFF);
	if (rc < 0)
		printk(KERN_ERR "%s: clock_vote return val: %d\n",
						__func__, rc);

	/*Disable the Clock Using GPIO34/AP2MDM_MRMBCK_EN in case
	of svlte*/
	if (machine_is_msm8x55_svlte_surf() ||
			machine_is_msm8x55_svlte_ffa())	{
		rc = marimba_gpio_config(0);
		if (rc < 0)
			printk(KERN_ERR "%s: clock disable for svlte : %d\n",
						__func__, rc);
	}


	if (!bahama_not_marimba)	{
		rc = pmapp_vreg_level_vote(id, PMAPP_VREG_S2, 0);

		if (rc < 0)
			printk(KERN_ERR "%s: vreg level vote return val: %d\n",
						__func__, rc);
	}
}

static struct marimba_fm_platform_data marimba_fm_pdata = {
	.fm_setup =  fm_radio_setup,
	.fm_shutdown = fm_radio_shutdown,
	.irq = MSM_GPIO_TO_INT(147),
	.vreg_s2 = NULL,
	.vreg_xo_out = NULL,
	.is_fm_soc_i2s_master = false,
	.config_i2s_gpio = NULL,
};


/* Slave id address for FM/CDC/QMEMBIST
 * Values can be programmed using Marimba slave id 0
 * should there be a conflict with other I2C devices
 * */
#define MARIMBA_SLAVE_ID_FM_ADDR	0x2A
#define MARIMBA_SLAVE_ID_CDC_ADDR	0x77
#define MARIMBA_SLAVE_ID_QMEMBIST_ADDR	0X66

#define BAHAMA_SLAVE_ID_FM_ADDR         0x2A
#define BAHAMA_SLAVE_ID_QMEMBIST_ADDR   0x7B

static const char *tsadc_id = "MADC";
static const char *vregs_tsadc_name[] = {
	"gp12",
	"s2",
};
static struct vreg *vregs_tsadc[ARRAY_SIZE(vregs_tsadc_name)];

static const char *vregs_timpani_tsadc_name[] = {
	"s3",
	"gp12",
	"gp16"
};
static struct vreg *vregs_timpani_tsadc[ARRAY_SIZE(vregs_timpani_tsadc_name)];

static int marimba_tsadc_power(int vreg_on)
{
	int i, rc = 0;
	int tsadc_adie_type = adie_get_detected_codec_type();

	if (tsadc_adie_type == TIMPANI_ID) {
		for (i = 0; i < ARRAY_SIZE(vregs_timpani_tsadc_name); i++) {
			if (!vregs_timpani_tsadc[i]) {
				pr_err("%s: vreg_get %s failed(%d)\n",
				__func__, vregs_timpani_tsadc_name[i], rc);
				goto vreg_fail;
			}

			rc = vreg_on ? vreg_enable(vregs_timpani_tsadc[i]) :
				  vreg_disable(vregs_timpani_tsadc[i]);
			if (rc < 0) {
				pr_err("%s: vreg %s %s failed(%d)\n",
					__func__, vregs_timpani_tsadc_name[i],
				       vreg_on ? "enable" : "disable", rc);
				goto vreg_fail;
			}
		}
		/* Vote for D0 and D1 buffer */
		rc = pmapp_clock_vote(tsadc_id, PMAPP_CLOCK_ID_D1,
			vreg_on ? PMAPP_CLOCK_VOTE_ON : PMAPP_CLOCK_VOTE_OFF);
		if (rc)	{
			pr_err("%s: unable to %svote for d1 clk\n",
				__func__, vreg_on ? "" : "de-");
			goto do_vote_fail;
		}
		rc = pmapp_clock_vote(tsadc_id, PMAPP_CLOCK_ID_DO,
			vreg_on ? PMAPP_CLOCK_VOTE_ON : PMAPP_CLOCK_VOTE_OFF);
		if (rc)	{
			pr_err("%s: unable to %svote for d1 clk\n",
				__func__, vreg_on ? "" : "de-");
			goto do_vote_fail;
		}
	} else if (tsadc_adie_type == MARIMBA_ID) {
		for (i = 0; i < ARRAY_SIZE(vregs_tsadc_name); i++) {
			if (!vregs_tsadc[i]) {
				pr_err("%s: vreg_get %s failed (%d)\n",
					__func__, vregs_tsadc_name[i], rc);
				goto vreg_fail;
			}

			rc = vreg_on ? vreg_enable(vregs_tsadc[i]) :
				  vreg_disable(vregs_tsadc[i]);
			if (rc < 0) {
				pr_err("%s: vreg %s %s failed (%d)\n",
					__func__, vregs_tsadc_name[i],
				       vreg_on ? "enable" : "disable", rc);
				goto vreg_fail;
			}
		}
		/* If marimba vote for DO buffer */
		rc = pmapp_clock_vote(tsadc_id, PMAPP_CLOCK_ID_DO,
			vreg_on ? PMAPP_CLOCK_VOTE_ON : PMAPP_CLOCK_VOTE_OFF);
		if (rc)	{
			pr_err("%s: unable to %svote for d0 clk\n",
				__func__, vreg_on ? "" : "de-");
			goto do_vote_fail;
		}
	} else {
		pr_err("%s:Adie %d not supported\n",
				__func__, tsadc_adie_type);
		return -ENODEV;
	}

	msleep(5); /* ensure power is stable */

	return 0;

do_vote_fail:
vreg_fail:
	while (i) {
		if (vreg_on) {
			if (tsadc_adie_type == TIMPANI_ID)
				vreg_disable(vregs_timpani_tsadc[--i]);
			else if (tsadc_adie_type == MARIMBA_ID)
				vreg_disable(vregs_tsadc[--i]);
		} else {
			if (tsadc_adie_type == TIMPANI_ID)
				vreg_enable(vregs_timpani_tsadc[--i]);
			else if (tsadc_adie_type == MARIMBA_ID)
				vreg_enable(vregs_tsadc[--i]);
		}
	}

	return rc;
}

static int marimba_tsadc_vote(int vote_on)
{
	int rc = 0;

	if (adie_get_detected_codec_type() == MARIMBA_ID) {
		int level = vote_on ? 1300 : 0;
		rc = pmapp_vreg_level_vote(tsadc_id, PMAPP_VREG_S2, level);
		if (rc < 0)
			pr_err("%s: vreg level %s failed (%d)\n",
			__func__, vote_on ? "on" : "off", rc);
	}

	return rc;
}

static int marimba_tsadc_init(void)
{
	int i, rc = 0;
	int tsadc_adie_type = adie_get_detected_codec_type();

	if (tsadc_adie_type == TIMPANI_ID) {
		for (i = 0; i < ARRAY_SIZE(vregs_timpani_tsadc_name); i++) {
			vregs_timpani_tsadc[i] = vreg_get(NULL,
						vregs_timpani_tsadc_name[i]);
			if (IS_ERR(vregs_timpani_tsadc[i])) {
				pr_err("%s: vreg get %s failed (%ld)\n",
				       __func__, vregs_timpani_tsadc_name[i],
				       PTR_ERR(vregs_timpani_tsadc[i]));
				rc = PTR_ERR(vregs_timpani_tsadc[i]);
				goto vreg_get_fail;
			}
		}
	} else if (tsadc_adie_type == MARIMBA_ID) {
		for (i = 0; i < ARRAY_SIZE(vregs_tsadc_name); i++) {
			vregs_tsadc[i] = vreg_get(NULL, vregs_tsadc_name[i]);
			if (IS_ERR(vregs_tsadc[i])) {
				pr_err("%s: vreg get %s failed (%ld)\n",
				       __func__, vregs_tsadc_name[i],
				       PTR_ERR(vregs_tsadc[i]));
				rc = PTR_ERR(vregs_tsadc[i]);
				goto vreg_get_fail;
			}
		}
	} else {
		pr_err("%s:Adie %d not supported\n",
				__func__, tsadc_adie_type);
		return -ENODEV;
	}

	return 0;

vreg_get_fail:
	while (i) {
		if (tsadc_adie_type == TIMPANI_ID)
			vreg_put(vregs_timpani_tsadc[--i]);
		else if (tsadc_adie_type == MARIMBA_ID)
			vreg_put(vregs_tsadc[--i]);
	}
	return rc;
}

static int marimba_tsadc_exit(void)
{
	int i, rc = 0;
	int tsadc_adie_type = adie_get_detected_codec_type();

	if (tsadc_adie_type == TIMPANI_ID) {
		for (i = 0; i < ARRAY_SIZE(vregs_timpani_tsadc_name); i++) {
			if (vregs_tsadc[i])
				vreg_put(vregs_timpani_tsadc[i]);
		}
	} else if (tsadc_adie_type == MARIMBA_ID) {
		for (i = 0; i < ARRAY_SIZE(vregs_tsadc_name); i++) {
			if (vregs_tsadc[i])
				vreg_put(vregs_tsadc[i]);
		}
		rc = pmapp_vreg_level_vote(tsadc_id, PMAPP_VREG_S2, 0);
		if (rc < 0)
			pr_err("%s: vreg level off failed (%d)\n",
						__func__, rc);
	} else {
		pr_err("%s:Adie %d not supported\n",
				__func__, tsadc_adie_type);
		rc = -ENODEV;
	}

	return rc;
}


static struct msm_ts_platform_data msm_ts_data = {
	.min_x          = 0,
	.max_x          = 4096,
	.min_y          = 0,
	.max_y          = 4096,
	.min_press      = 0,
	.max_press      = 255,
	.inv_x          = 4096,
	.inv_y          = 4096,
	.can_wakeup	= false,
};

static struct marimba_tsadc_platform_data marimba_tsadc_pdata = {
	.marimba_tsadc_power =  marimba_tsadc_power,
	.init		     =  marimba_tsadc_init,
	.exit		     =  marimba_tsadc_exit,
	.level_vote	     =  marimba_tsadc_vote,
	.tsadc_prechg_en = true,
	.can_wakeup	= false,
	.setup = {
		.pen_irq_en	=	true,
		.tsadc_en	=	true,
	},
	.params2 = {
		.input_clk_khz		=	2400,
		.sample_prd		=	TSADC_CLK_3,
	},
	.params3 = {
		.prechg_time_nsecs	=	6400,
		.stable_time_nsecs	=	6400,
		.tsadc_test_mode	=	0,
	},
	.tssc_data = &msm_ts_data,
};

static struct vreg *vreg_codec_s4;
static int msm_marimba_codec_power(int vreg_on)
{
	int rc = 0;

	if (!vreg_codec_s4) {

		vreg_codec_s4 = vreg_get(NULL, "s4");

		if (IS_ERR(vreg_codec_s4)) {
			printk(KERN_ERR "%s: vreg_get() failed (%ld)\n",
				__func__, PTR_ERR(vreg_codec_s4));
			rc = PTR_ERR(vreg_codec_s4);
			goto  vreg_codec_s4_fail;
		}
	}

	if (vreg_on) {
		rc = vreg_enable(vreg_codec_s4);
		if (rc)
			printk(KERN_ERR "%s: vreg_enable() = %d \n",
					__func__, rc);
		goto vreg_codec_s4_fail;
	} else {
		rc = vreg_disable(vreg_codec_s4);
		if (rc)
			printk(KERN_ERR "%s: vreg_disable() = %d \n",
					__func__, rc);
		goto vreg_codec_s4_fail;
	}

vreg_codec_s4_fail:
	return rc;
}

static struct marimba_codec_platform_data mariba_codec_pdata = {
	.marimba_codec_power =  msm_marimba_codec_power,
};

static struct marimba_platform_data marimba_pdata = {
	.slave_id[MARIMBA_SLAVE_ID_FM]       = MARIMBA_SLAVE_ID_FM_ADDR,
	.slave_id[MARIMBA_SLAVE_ID_CDC]	     = MARIMBA_SLAVE_ID_CDC_ADDR,
	.slave_id[MARIMBA_SLAVE_ID_QMEMBIST] = MARIMBA_SLAVE_ID_QMEMBIST_ADDR,
	.slave_id[SLAVE_ID_BAHAMA_FM]        = BAHAMA_SLAVE_ID_FM_ADDR,
	.slave_id[SLAVE_ID_BAHAMA_QMEMBIST]  = BAHAMA_SLAVE_ID_QMEMBIST_ADDR,
	.marimba_setup = msm_marimba_setup_power,
	.marimba_shutdown = msm_marimba_shutdown_power,
	.marimba_gpio_config = msm_marimba_gpio_config_svlte,
	.fm = &marimba_fm_pdata,
	.codec = &mariba_codec_pdata,
	.tsadc_ssbi_adap = MARIMBA_SSBI_ADAP,
};

static void __init flyer_init_marimba(void)
{
#if 0
	vreg_marimba_1 = vreg_get(NULL, "s2");
	if (IS_ERR(vreg_marimba_1)) {
		printk(KERN_ERR "%s: vreg get failed (%ld)\n",
			__func__, PTR_ERR(vreg_marimba_1));
		return;
	}
#endif
	vreg_marimba_2 = vreg_get(NULL, "gp16");
	if (IS_ERR(vreg_marimba_2)) {
		printk(KERN_ERR "%s: vreg get failed (%ld)\n",
			__func__, PTR_ERR(vreg_marimba_2));
		return;
	}
}

static struct marimba_codec_platform_data timpani_codec_pdata = {
	.marimba_codec_power =  msm_marimba_codec_power,
};

static struct marimba_platform_data timpani_pdata = {
	.slave_id[MARIMBA_SLAVE_ID_CDC]	= MARIMBA_SLAVE_ID_CDC_ADDR,
	.slave_id[MARIMBA_SLAVE_ID_QMEMBIST] = MARIMBA_SLAVE_ID_QMEMBIST_ADDR,
	.marimba_setup = msm_timpani_setup_power,
	.marimba_shutdown = msm_timpani_shutdown_power,
	.codec = &timpani_codec_pdata,
	.tsadc = &marimba_tsadc_pdata,
	.tsadc_ssbi_adap = MARIMBA_SSBI_ADAP,
};

#define TIMPANI_I2C_SLAVE_ADDR	0xD

static struct i2c_board_info msm_i2c_gsbi7_timpani_info[] = {
	{
		I2C_BOARD_INFO("timpani", TIMPANI_I2C_SLAVE_ADDR),
		.platform_data = &timpani_pdata,
	},
};

#ifdef CONFIG_MSM7KV2_AUDIO
static struct resource msm_aictl_resources[] = {
	{
		.name = "aictl",
		.start = 0xa5000100,
		.end = 0xa5000100,
		.flags = IORESOURCE_MEM,
	}
};

static struct resource msm_mi2s_resources[] = {
	{
		.name = "hdmi",
		.start = 0xac900000,
		.end = 0xac900038,
		.flags = IORESOURCE_MEM,
	},
	{
		.name = "codec_rx",
		.start = 0xac940040,
		.end = 0xac940078,
		.flags = IORESOURCE_MEM,
	},
	{
		.name = "codec_tx",
		.start = 0xac980080,
		.end = 0xac9800B8,
		.flags = IORESOURCE_MEM,
	}

};

static struct msm_lpa_platform_data lpa_pdata = {
	.obuf_hlb_size = 0x2BFF8,
	.dsp_proc_id = 0,
	.app_proc_id = 2,
	.nosb_config = {
		.llb_min_addr = 0,
		.llb_max_addr = 0x3ff8,
		.sb_min_addr = 0,
		.sb_max_addr = 0,
	},
	.sb_config = {
		.llb_min_addr = 0,
		.llb_max_addr = 0x37f8,
		.sb_min_addr = 0x3800,
		.sb_max_addr = 0x3ff8,
	}
};

static struct resource msm_lpa_resources[] = {
	{
		.name = "lpa",
		.start = 0xa5000000,
		.end = 0xa50000a0,
		.flags = IORESOURCE_MEM,
	}
};

static struct resource msm_aux_pcm_resources[] = {

	{
		.name = "aux_codec_reg_addr",
		.start = 0xac9c00c0,
		.end = 0xac9c00c8,
		.flags = IORESOURCE_MEM,
	},
	{
		.name   = "aux_pcm_dout",
		.start  = 138,
		.end    = 138,
		.flags  = IORESOURCE_IO,
	},
	{
		.name   = "aux_pcm_din",
		.start  = 139,
		.end    = 139,
		.flags  = IORESOURCE_IO,
	},
	{
		.name   = "aux_pcm_syncout",
		.start  = 140,
		.end    = 140,
		.flags  = IORESOURCE_IO,
	},
	{
		.name   = "aux_pcm_clkin_a",
		.start  = 141,
		.end    = 141,
		.flags  = IORESOURCE_IO,
	},
};

static struct platform_device msm_aux_pcm_device = {
	.name   = "msm_aux_pcm",
	.id     = 0,
	.num_resources  = ARRAY_SIZE(msm_aux_pcm_resources),
	.resource       = msm_aux_pcm_resources,
};

struct platform_device msm_aictl_device = {
	.name = "audio_interct",
	.id   = 0,
	.num_resources = ARRAY_SIZE(msm_aictl_resources),
	.resource = msm_aictl_resources,
};

struct platform_device htc_drm = {
	.name = "htcdrm",
	.id = 0,
};

struct platform_device msm_mi2s_device = {
	.name = "mi2s",
	.id   = 0,
	.num_resources = ARRAY_SIZE(msm_mi2s_resources),
	.resource = msm_mi2s_resources,
};

struct platform_device msm_lpa_device = {
	.name = "lpa",
	.id   = 0,
	.num_resources = ARRAY_SIZE(msm_lpa_resources),
	.resource = msm_lpa_resources,
	.dev		= {
		.platform_data = &lpa_pdata,
	},
};
#endif /* CONFIG_MSM7KV2_AUDIO */



#define DEC0_FORMAT ((1<<MSM_ADSP_CODEC_MP3)| \
	(1<<MSM_ADSP_CODEC_AAC)|(1<<MSM_ADSP_CODEC_WMA)| \
	(1<<MSM_ADSP_CODEC_WMAPRO)|(1<<MSM_ADSP_CODEC_AMRWB)| \
	(1<<MSM_ADSP_CODEC_AMRNB)|(1<<MSM_ADSP_CODEC_WAV)| \
	(1<<MSM_ADSP_CODEC_ADPCM)|(1<<MSM_ADSP_CODEC_YADPCM)| \
	(1<<MSM_ADSP_CODEC_EVRC)|(1<<MSM_ADSP_CODEC_QCELP))
#define DEC1_FORMAT ((1<<MSM_ADSP_CODEC_MP3)| \
	(1<<MSM_ADSP_CODEC_AAC)|(1<<MSM_ADSP_CODEC_WMA)| \
	(1<<MSM_ADSP_CODEC_WMAPRO)|(1<<MSM_ADSP_CODEC_AMRWB)| \
	(1<<MSM_ADSP_CODEC_AMRNB)|(1<<MSM_ADSP_CODEC_WAV)| \
	(1<<MSM_ADSP_CODEC_ADPCM)|(1<<MSM_ADSP_CODEC_YADPCM)| \
	(1<<MSM_ADSP_CODEC_EVRC)|(1<<MSM_ADSP_CODEC_QCELP))
#define DEC2_FORMAT ((1<<MSM_ADSP_CODEC_MP3)| \
	(1<<MSM_ADSP_CODEC_AAC)|(1<<MSM_ADSP_CODEC_WMA)| \
	(1<<MSM_ADSP_CODEC_WMAPRO)|(1<<MSM_ADSP_CODEC_AMRWB)| \
	(1<<MSM_ADSP_CODEC_AMRNB)|(1<<MSM_ADSP_CODEC_WAV)| \
	(1<<MSM_ADSP_CODEC_ADPCM)|(1<<MSM_ADSP_CODEC_YADPCM)| \
	(1<<MSM_ADSP_CODEC_EVRC)|(1<<MSM_ADSP_CODEC_QCELP))
#define DEC3_FORMAT ((1<<MSM_ADSP_CODEC_MP3)| \
	(1<<MSM_ADSP_CODEC_AAC)|(1<<MSM_ADSP_CODEC_WMA)| \
	(1<<MSM_ADSP_CODEC_WMAPRO)|(1<<MSM_ADSP_CODEC_AMRWB)| \
	(1<<MSM_ADSP_CODEC_AMRNB)|(1<<MSM_ADSP_CODEC_WAV)| \
	(1<<MSM_ADSP_CODEC_ADPCM)|(1<<MSM_ADSP_CODEC_YADPCM)| \
	(1<<MSM_ADSP_CODEC_EVRC)|(1<<MSM_ADSP_CODEC_QCELP))
#define DEC4_FORMAT (1<<MSM_ADSP_CODEC_MIDI)

static unsigned int dec_concurrency_table[] = {
	/* Audio LP */
	0,
	(DEC3_FORMAT|(1<<MSM_ADSP_MODE_NONTUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC2_FORMAT|(1<<MSM_ADSP_MODE_NONTUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC1_FORMAT|(1<<MSM_ADSP_MODE_NONTUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC0_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_MODE_LP)|
	(1<<MSM_ADSP_OP_DM)),

	/* Concurrency 1 */
	(DEC4_FORMAT),
	(DEC3_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC2_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC1_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC0_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),

	 /* Concurrency 2 */
	(DEC4_FORMAT),
	(DEC3_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC2_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC1_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC0_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),

	/* Concurrency 3 */
	(DEC4_FORMAT),
	(DEC3_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC2_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC1_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC0_FORMAT|(1<<MSM_ADSP_MODE_NONTUNNEL)|(1<<MSM_ADSP_OP_DM)),

	/* Concurrency 4 */
	(DEC4_FORMAT),
	(DEC3_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC2_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC1_FORMAT|(1<<MSM_ADSP_MODE_NONTUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC0_FORMAT|(1<<MSM_ADSP_MODE_NONTUNNEL)|(1<<MSM_ADSP_OP_DM)),

	/* Concurrency 5 */
	(DEC4_FORMAT),
	(DEC3_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC2_FORMAT|(1<<MSM_ADSP_MODE_NONTUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC1_FORMAT|(1<<MSM_ADSP_MODE_NONTUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC0_FORMAT|(1<<MSM_ADSP_MODE_NONTUNNEL)|(1<<MSM_ADSP_OP_DM)),

	/* Concurrency 6 */
	(DEC4_FORMAT),
	(DEC3_FORMAT|(1<<MSM_ADSP_MODE_NONTUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC2_FORMAT|(1<<MSM_ADSP_MODE_NONTUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC1_FORMAT|(1<<MSM_ADSP_MODE_NONTUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC0_FORMAT|(1<<MSM_ADSP_MODE_NONTUNNEL)|(1<<MSM_ADSP_OP_DM)),
};

#define DEC_INFO(name, queueid, decid, nr_codec) { .module_name = name, \
	.module_queueid = queueid, .module_decid = decid, \
	.nr_codec_support = nr_codec}

#define DEC_INSTANCE(max_instance_same, max_instance_diff) { \
	.max_instances_same_dec = max_instance_same, \
	.max_instances_diff_dec = max_instance_diff}

static struct msm_adspdec_info dec_info_list[] = {
	DEC_INFO("AUDPLAY4TASK", 17, 4, 1),  /* AudPlay4BitStreamCtrlQueue */
	DEC_INFO("AUDPLAY3TASK", 16, 3, 11),  /* AudPlay3BitStreamCtrlQueue */
	DEC_INFO("AUDPLAY2TASK", 15, 2, 11),  /* AudPlay2BitStreamCtrlQueue */
	DEC_INFO("AUDPLAY1TASK", 14, 1, 11),  /* AudPlay1BitStreamCtrlQueue */
	DEC_INFO("AUDPLAY0TASK", 13, 0, 11), /* AudPlay0BitStreamCtrlQueue */
};

static struct dec_instance_table dec_instance_list[][MSM_MAX_DEC_CNT] = {
	/* Non Turbo Mode */
	{
		DEC_INSTANCE(4, 3), /* WAV */
		DEC_INSTANCE(4, 3), /* ADPCM */
		DEC_INSTANCE(4, 2), /* MP3 */
		DEC_INSTANCE(0, 0), /* Real Audio */
		DEC_INSTANCE(4, 2), /* WMA */
		DEC_INSTANCE(3, 2), /* AAC */
		DEC_INSTANCE(0, 0), /* Reserved */
		DEC_INSTANCE(0, 0), /* MIDI */
		DEC_INSTANCE(4, 3), /* YADPCM */
		DEC_INSTANCE(4, 3), /* QCELP */
		DEC_INSTANCE(4, 3), /* AMRNB */
		DEC_INSTANCE(1, 1), /* AMRWB/WB+ */
		DEC_INSTANCE(4, 3), /* EVRC */
		DEC_INSTANCE(1, 1), /* WMAPRO */
	},
	/* Turbo Mode */
	{
		DEC_INSTANCE(4, 3), /* WAV */
		DEC_INSTANCE(4, 3), /* ADPCM */
		DEC_INSTANCE(4, 3), /* MP3 */
		DEC_INSTANCE(0, 0), /* Real Audio */
		DEC_INSTANCE(4, 3), /* WMA */
		DEC_INSTANCE(4, 3), /* AAC */
		DEC_INSTANCE(0, 0), /* Reserved */
		DEC_INSTANCE(0, 0), /* MIDI */
		DEC_INSTANCE(4, 3), /* YADPCM */
		DEC_INSTANCE(4, 3), /* QCELP */
		DEC_INSTANCE(4, 3), /* AMRNB */
		DEC_INSTANCE(2, 3), /* AMRWB/WB+ */
		DEC_INSTANCE(4, 3), /* EVRC */
		DEC_INSTANCE(1, 2), /* WMAPRO */
	},
};

static struct msm_adspdec_database msm_device_adspdec_database = {
	.num_dec = ARRAY_SIZE(dec_info_list),
	.num_concurrency_support = (ARRAY_SIZE(dec_concurrency_table) / \
					ARRAY_SIZE(dec_info_list)),
	.dec_concurrency_table = dec_concurrency_table,
	.dec_info_list = dec_info_list,
	.dec_instance_list = &dec_instance_list[0][0],
};

static struct platform_device msm_device_adspdec = {
	.name = "msm_adspdec",
	.id = -1,
	.dev    = {
		.platform_data = &msm_device_adspdec_database
	},
};

#ifdef CONFIG_USB_G_ANDROID
static struct android_usb_platform_data android_usb_pdata = {
	.vendor_id	= 0x0BB4,
	.product_id	= 0x0ce5,
	.version	= 0x0100,
	.product_name		= "Android Phone",
	.manufacturer_name	= "HTC",
	.num_products = ARRAY_SIZE(usb_products),
	.products = usb_products,
	.num_functions = ARRAY_SIZE(usb_functions_all),
	.functions = usb_functions_all,
	.fserial_init_string = "tty:modem,tty:autobot,tty:serial,tty:autobot",
	.nluns = 2,
	.usb_id_pin_gpio = FLYER_GPIO_USB_ID_PIN,
	//.req_reset_during_switch_func = 1,
};

static struct platform_device android_usb_device = {
	.name	= "android_usb",
	.id		= -1,
	.dev		= {
		.platform_data = &android_usb_pdata,
	},
};

#endif

static struct i2c_board_info msm_marimba_board_info[] = {
	{
		I2C_BOARD_INFO("marimba", 0xc),
		.platform_data = &marimba_pdata,
	}
};

static struct msm_handset_platform_data hs_platform_data = {
	.hs_name = "7k_handset",
	.pwr_key_delay_ms = 500, /* 0 will disable end key */
};

static struct platform_device hs_device = {
	.name   = "msm-handset",
	.id     = -1,
	.dev    = {
		.platform_data = &hs_platform_data,
	},
};

static struct msm_pm_platform_data msm_pm_data[MSM_PM_SLEEP_MODE_NR] = {
	[MSM_PM_SLEEP_MODE_POWER_COLLAPSE] = {
		.idle_supported = 1,
		.suspend_supported = 1,
		.idle_enabled = 1,
		.suspend_enabled = 1,
		.latency = 8594,
		.residency = 23740,
	},
	[MSM_PM_SLEEP_MODE_APPS_SLEEP] = {
		.idle_supported = 1,
		.suspend_supported = 1,
		.idle_enabled = 1,
		.suspend_enabled = 1,
		.latency = 8594,
		.residency = 23740,
	},
/*
	[MSM_PM_SLEEP_MODE_POWER_COLLAPSE_NO_XO_SHUTDOWN] = {
		.idle_supported = 1,
		.suspend_supported = 1,
		.idle_enabled = 1,
		.suspend_enabled = 1,
		.latency = 4594,
		.residency = 23740,
	},
*/
	[MSM_PM_SLEEP_MODE_POWER_COLLAPSE_STANDALONE] = {
#ifdef CONFIG_MSM_STANDALONE_POWER_COLLAPSE
		.idle_supported = 1,
		.suspend_supported = 1,
		.idle_enabled = 1,
		.suspend_enabled = 0,
#else /*CONFIG_MSM_STANDALONE_POWER_COLLAPSE*/
		.idle_supported = 0,
		.suspend_supported = 0,
		.idle_enabled = 0,
		.suspend_enabled = 0,
#endif /*CONFIG_MSM_STANDALONE_POWER_COLLAPSE*/
		.latency = 500,
		.residency = 6000,
	},
	[MSM_PM_SLEEP_MODE_RAMP_DOWN_AND_WAIT_FOR_INTERRUPT] = {
		.idle_supported = 1,
		.suspend_supported = 1,
		.idle_enabled = 0,
		.suspend_enabled = 1,
		.latency = 443,
		.residency = 1098,
	},
	[MSM_PM_SLEEP_MODE_WAIT_FOR_INTERRUPT] = {
		.idle_supported = 1,
		.suspend_supported = 1,
		.idle_enabled = 1,
		.suspend_enabled = 1,
		.latency = 2,
		.residency = 0,
	},
};

static struct resource qsd_spi_resources[] = {
	{
		.name   = "spi_irq_in",
		.start	= INT_SPI_INPUT,
		.end	= INT_SPI_INPUT,
		.flags	= IORESOURCE_IRQ,
	},
	{
		.name   = "spi_irq_out",
		.start	= INT_SPI_OUTPUT,
		.end	= INT_SPI_OUTPUT,
		.flags	= IORESOURCE_IRQ,
	},
	{
		.name   = "spi_irq_err",
		.start	= INT_SPI_ERROR,
		.end	= INT_SPI_ERROR,
		.flags	= IORESOURCE_IRQ,
	},
	{
		.name   = "spi_base",
		.start	= 0xA8000000,
		.end	= 0xA8000000 + SZ_4K - 1,
		.flags	= IORESOURCE_MEM,
	},
	{
		.name   = "spidm_channels",
		.flags  = IORESOURCE_DMA,
	},
	{
		.name   = "spidm_crci",
		.flags  = IORESOURCE_DMA,
	},
};

static int msm_qsd_spi_dma_config(void)
{
	return -ENOENT;
}

static struct platform_device qsd_device_spi = {
	.name		= "spi_qsd",
	.id		= 0,
	.num_resources	= ARRAY_SIZE(qsd_spi_resources),
	.resource	= qsd_spi_resources,
};

#ifdef CONFIG_SPI_QSD
static struct spi_board_info msm_spi_board_info[] __initdata = {
	{
		.modalias	= "spi_aic3254",
		.mode           = SPI_MODE_1,
		.bus_num        = 0,
		.chip_select    = 3,
		.max_speed_hz   = 9963243,
	},
	{
		.modalias		= NTRIG_NAME,
		.mode           = SPI_MODE_0,
		.bus_num        = 0,
		.chip_select    = 2,
		.max_speed_hz   = 9963243,
		.platform_data  = &flyer_ts_ntrig_data,
		.irq	= MSM_GPIO_TO_INT(FLYER_GPIO_TP_ATT_N),
	}
};
#endif

static uint32_t qsd_spi_gpio_on_table[] = {
	PCOM_GPIO_CFG(45, 1, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_6MA),
	PCOM_GPIO_CFG(47, 1, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_6MA),
	PCOM_GPIO_CFG(48, 1, GPIO_INPUT, GPIO_NO_PULL, GPIO_6MA),
	PCOM_GPIO_CFG(87, 2, GPIO_INPUT, GPIO_NO_PULL, GPIO_16MA),
	/* SPI GPIO for AIC3254 */
	PCOM_GPIO_CFG(89, 2, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_6MA)
};

static uint32_t qsd_spi_gpio_off_table[] = {
	PCOM_GPIO_CFG(45, 0, GPIO_INPUT, GPIO_NO_PULL, GPIO_16MA),
	PCOM_GPIO_CFG(47, 0, GPIO_INPUT, GPIO_NO_PULL, GPIO_16MA),
	PCOM_GPIO_CFG(48, 0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_16MA),
	PCOM_GPIO_CFG(87, 0, GPIO_INPUT, GPIO_NO_PULL, GPIO_16MA),
	PCOM_GPIO_CFG(89, 0, GPIO_INPUT, GPIO_NO_PULL, GPIO_16MA)
};

static int msm_qsd_spi_gpio_config(void)
{
	config_gpio_table(qsd_spi_gpio_on_table,
		ARRAY_SIZE(qsd_spi_gpio_on_table));
	return 0;
}

static void msm_qsd_spi_gpio_release(void)
{
	config_gpio_table(qsd_spi_gpio_off_table,
		ARRAY_SIZE(qsd_spi_gpio_off_table));
}

static struct msm_spi_platform_data qsd_spi_pdata = {
	.max_clock_speed = 26331429,
	.gpio_config  = msm_qsd_spi_gpio_config,
	.gpio_release = msm_qsd_spi_gpio_release,
	.dma_config = msm_qsd_spi_dma_config,
};

static void __init msm_qsd_spi_init(void)
{
	qsd_device_spi.dev.platform_data = &qsd_spi_pdata;
}

#ifdef CONFIG_USB_EHCI_MSM_72K
static void msm_hsusb_vbus_power(unsigned phy_info, int on)
{
	int rc;
	static int vbus_is_on;
	struct pm8xxx_gpio_init_info usb_vbus = {
		PM8058_GPIO_PM_TO_SYS(36),
		{
			.direction      = PM_GPIO_DIR_OUT,
			.pull           = PM_GPIO_PULL_NO,
			.output_buffer  = PM_GPIO_OUT_BUF_CMOS,
			.output_value   = 1,
			.vin_sel        = 2,
			.out_strength   = PM_GPIO_STRENGTH_MED,
			.function       = PM_GPIO_FUNC_NORMAL,
			.inv_int_pol    = 0,
		},
	};

	/* If VBUS is already on (or off), do nothing. */
	if (unlikely(on == vbus_is_on))
		return;

	if (on) {
		rc = pm8xxx_gpio_config(usb_vbus.gpio, &usb_vbus.config);
		if (rc) {
			pr_err("%s PMIC GPIO 36 write failed\n", __func__);
			return;
		}
	} else {
		gpio_set_value_cansleep(PM8058_GPIO_PM_TO_SYS(36), 0);
	}

	vbus_is_on = on;
}

static struct msm_usb_host_platform_data msm_usb_host_pdata = {
		.phy_info   = (USB_PHY_INTEGRATED | USB_PHY_MODEL_45NM),
		.vbus_power = msm_hsusb_vbus_power,
		.power_budget   = 180,
};
#endif

static int phy_init_seq[] = { 0x06, 0x36, 0x0C, 0x31, 0x31, 0x32, 0x1, 0x0D, 0x1, 0x10, -1 };
static struct msm_hsusb_gadget_platform_data msm_gadget_pdata = {
	.phy_init_seq		= phy_init_seq,
	.is_phy_status_timer_on = 1,
};

static struct msm_otg_platform_data msm_otg_pdata = {
#ifdef CONFIG_USB_EHCI_MSM_72K
	.vbus_power = msm_hsusb_vbus_power,
#endif
	.pemp_level		= PRE_EMPHASIS_WITH_20_PERCENT,
	.cdr_autoreset	= CDR_AUTO_RESET_DISABLE,
	.drv_ampl		= HS_DRV_AMPLITUDE_DEFAULT,
	.se1_gating		= SE1_GATING_DISABLE,
};

static struct resource msm_fb_resources[] = {
	{
		.flags  = IORESOURCE_DMA,
	}
};

static struct platform_device msm_migrate_pages_device = {
	.name   = "msm_migrate_pages",
	.id     = -1,
};

static struct android_pmem_platform_data android_pmem_pdata = {
	.name = "pmem",
	.allocator_type = PMEM_ALLOCATORTYPE_ALLORNOTHING,
	.cached = 1,
	.memory_type = MEMTYPE_EBI0,
};

static struct platform_device android_pmem_device = {
	.name = "android_pmem",
	.id = 0,
	.dev = { .platform_data = &android_pmem_pdata },
};

static struct android_pmem_platform_data android_pmem_adsp_pdata = {
       .name = "pmem_adsp",
       .allocator_type = PMEM_ALLOCATORTYPE_BITMAP,
       .cached = 1,
	.memory_type = MEMTYPE_EBI0,
};

static struct platform_device android_pmem_adsp_device = {
	.name = "android_pmem",
	.id = 2,
	.dev = { .platform_data = &android_pmem_adsp_pdata },
};

#if defined(CONFIG_CRYPTO_DEV_QCRYPTO) || \
		defined(CONFIG_CRYPTO_DEV_QCRYPTO_MODULE) || \
		defined(CONFIG_CRYPTO_DEV_QCEDEV) || \
		defined(CONFIG_CRYPTO_DEV_QCEDEV_MODULE)

#define QCE_SIZE		0x10000
#define QCE_0_BASE		0xA8400000

#define QCE_HW_KEY_SUPPORT	1
#define QCE_SHA_HMAC_SUPPORT	0
#define QCE_SHARE_CE_RESOURCE	0
#define QCE_CE_SHARED		0

static struct resource qcrypto_resources[] = {
	[0] = {
		.start = QCE_0_BASE,
		.end = QCE_0_BASE + QCE_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.name = "crypto_channels",
		.start = DMOV_CE_IN_CHAN,
		.end = DMOV_CE_OUT_CHAN,
		.flags = IORESOURCE_DMA,
	},
	[2] = {
		.name = "crypto_crci_in",
		.start = DMOV_CE_IN_CRCI,
		.end = DMOV_CE_IN_CRCI,
		.flags = IORESOURCE_DMA,
	},
	[3] = {
		.name = "crypto_crci_out",
		.start = DMOV_CE_OUT_CRCI,
		.end = DMOV_CE_OUT_CRCI,
		.flags = IORESOURCE_DMA,
	},
	[4] = {
		.name = "crypto_crci_hash",
		.start = DMOV_CE_HASH_CRCI,
		.end = DMOV_CE_HASH_CRCI,
		.flags = IORESOURCE_DMA,
	},
};

static struct resource qcedev_resources[] = {
	[0] = {
		.start = QCE_0_BASE,
		.end = QCE_0_BASE + QCE_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.name = "crypto_channels",
		.start = DMOV_CE_IN_CHAN,
		.end = DMOV_CE_OUT_CHAN,
		.flags = IORESOURCE_DMA,
	},
	[2] = {
		.name = "crypto_crci_in",
		.start = DMOV_CE_IN_CRCI,
		.end = DMOV_CE_IN_CRCI,
		.flags = IORESOURCE_DMA,
	},
	[3] = {
		.name = "crypto_crci_out",
		.start = DMOV_CE_OUT_CRCI,
		.end = DMOV_CE_OUT_CRCI,
		.flags = IORESOURCE_DMA,
	},
	[4] = {
		.name = "crypto_crci_hash",
		.start = DMOV_CE_HASH_CRCI,
		.end = DMOV_CE_HASH_CRCI,
		.flags = IORESOURCE_DMA,
	},
};

#endif

#if defined(CONFIG_CRYPTO_DEV_QCRYPTO) || \
		defined(CONFIG_CRYPTO_DEV_QCRYPTO_MODULE)

static struct msm_ce_hw_support qcrypto_ce_hw_suppport = {
	.ce_shared = QCE_CE_SHARED,
	.shared_ce_resource = QCE_SHARE_CE_RESOURCE,
	.hw_key_support = QCE_HW_KEY_SUPPORT,
	.sha_hmac = QCE_SHA_HMAC_SUPPORT,
};

static struct platform_device qcrypto_device = {
	.name		= "qcrypto",
	.id		= 0,
	.num_resources	= ARRAY_SIZE(qcrypto_resources),
	.resource	= qcrypto_resources,
	.dev		= {
		.coherent_dma_mask = DMA_BIT_MASK(32),
		.platform_data = &qcrypto_ce_hw_suppport,
	},
};
#endif

#if defined(CONFIG_CRYPTO_DEV_QCEDEV) || \
		defined(CONFIG_CRYPTO_DEV_QCEDEV_MODULE)

static struct msm_ce_hw_support qcedev_ce_hw_suppport = {
	.ce_shared = QCE_CE_SHARED,
	.shared_ce_resource = QCE_SHARE_CE_RESOURCE,
	.hw_key_support = QCE_HW_KEY_SUPPORT,
	.sha_hmac = QCE_SHA_HMAC_SUPPORT,
};
static struct platform_device qcedev_device = {
	.name		= "qce",
	.id		= 0,
	.num_resources	= ARRAY_SIZE(qcedev_resources),
	.resource	= qcedev_resources,
	.dev		= {
		.coherent_dma_mask = DMA_BIT_MASK(32),
		.platform_data = &qcedev_ce_hw_suppport,
	},
};

#endif

UINT32 flyer_battery_id_tbl[] =
{
	/* id resister range = [min, max)*/
	7000, 50000,	/* ID = 1 ATL  (22k) 7k ~ 50k */
	600000, -1,	/* ID = 2 LG  (>600k) */
	-1, /* end of table */
};


static void flyer_poweralg_config_init(struct poweralg_config_type *config)
{
	pr_info("batt: %s() is used\n",__func__);
	config->full_charging_mv = 4110;
	config->full_charging_ma = 150;
	config->full_pending_ma = 0;		/* disabled*/
	config->full_charging_timeout_sec = 30 * 60;
	config->voltage_recharge_mv = 0; /* disabled */
	config->capacity_recharge_p = 98;
	config->voltage_exit_full_mv = 3800;
	config->min_taper_current_mv = 0;	/* disabled */
	config->min_taper_current_ma = 0;	/* disabled */
	config->wait_votlage_statble_sec = 1 * 60;
	config->predict_timeout_sec = 10;
	/* TODO: doesn't be used. use program instead. (FAST_POLL/SLOW_POLL) */
	config->polling_time_in_charging_sec = 30;
	/* TODO: doesn't be used. use program instead. (FAST_POLL/SLOW_POLL) */
	config->polling_time_in_discharging_sec = 30;

	config->enable_full_calibration = TRUE;
	config->enable_weight_percentage = TRUE;
	config->software_charger_timeout_sec = 0;   	 /* disabled*/
	config->superchg_software_charger_timeout_sec = 16 * 60 * 60;	/* 16 hrs */
	config->charger_hw_safety_timer_watchdog_sec =  4 * 60 * 60;	/* 4 hrs */

	config->debug_disable_shutdown = FALSE;
	config->debug_fake_room_temp = FALSE;
	config->debug_disable_hw_timer = FALSE;
	config->debug_always_predict = FALSE;
	config->full_level = 0;
}

static int flyer_update_charging_protect_flag(int ibat_ma, int vbat_mv, int temp_01c, BOOL* chg_allowed, BOOL* hchg_allowed, BOOL *temp_fault)
{
	static int pState = 0;
	int old_pState = pState;
	/* pStates:
		0: initial (temp detection)
		1: temp < 0 degree c
		2: 0 <= temp <= 48 degree c
		3: 48 < temp <= 55 degree c
		4: 55 < temp
	*/
	enum {
		PSTAT_DETECT=0,
		PSTAT_LOW_STOP,
		PSTAT_NORMAL,
		PSTAT_LIMITED,
		PSTAT_HIGH_STOP
	};
	/* generally we assumed that pState implies last temp.
		it won't hold if temp changes faster than sample rate */

	/* step 1. check if change state condition is hit */
	/* pr_info("batt: %s(i=%d, v=%d, t=%d, %d, %d)\n",__func__, ibat_ma, vbat_mv, temp_01c, *chg_allowed, *hchg_allowed); */
	switch(pState) {
		default:
			pr_info("error: unexpected pState\n");
		case PSTAT_DETECT:
			if (temp_01c < 0)
				pState = PSTAT_LOW_STOP;
			if ((0 <= temp_01c) && (temp_01c <= 480))
				pState = PSTAT_NORMAL;
			if ((480 < temp_01c) && (temp_01c <= 550))
				pState = PSTAT_LIMITED;
			if (550 < temp_01c)
				pState = PSTAT_HIGH_STOP;
			break;
		case PSTAT_LOW_STOP:
			if (30 <= temp_01c)
				pState = PSTAT_NORMAL;
			/* suppose never jump to LIMITED/HIGH_STOP from here */
			break;
		case PSTAT_NORMAL:
			if (temp_01c < 0)
				pState = PSTAT_LOW_STOP;
			else if (550 < temp_01c)
				pState = PSTAT_HIGH_STOP;
			else if (480 < temp_01c) /* also implies t <= 550 */
				pState = PSTAT_LIMITED;
			break;
		case PSTAT_LIMITED:
			if (temp_01c < 470)
				pState = PSTAT_NORMAL;
			if (550 < temp_01c)
				pState = PSTAT_HIGH_STOP;
			/* suppose never jump to LOW_STOP from here */
			break;
		case PSTAT_HIGH_STOP:
			if (temp_01c < 450)
				pState = PSTAT_NORMAL;
			else if ((temp_01c < 530) && (vbat_mv < 3800))
				pState = PSTAT_LIMITED;
			/* suppose never jump to LOW_STOP from here */
			break;
	}
	if (old_pState != pState)
		pr_info("batt: Protect pState changed from %d to %d\n", old_pState, pState);

	/* step 2. check state protect condition */
	/* chg_allowed = TRUE:only means it's allowed no matter it has charger.
		same as hchg_allowed. */
	switch(pState) {
		default:
		case PSTAT_DETECT:
			pr_info("batt: error: unexpected pState\n");
			break;
		case PSTAT_LOW_STOP:
			*chg_allowed = FALSE;
			*hchg_allowed = FALSE;
			break;
		case PSTAT_NORMAL:
			*chg_allowed = TRUE;
			*hchg_allowed = TRUE;
			break;
		case PSTAT_LIMITED:
			if (PSTAT_LIMITED != old_pState)
				*chg_allowed = TRUE;
			if (4000 < vbat_mv)
				*chg_allowed = FALSE;
			else if (vbat_mv < 3800)
				*chg_allowed = TRUE;
			*hchg_allowed = FALSE;
			break;
		case PSTAT_HIGH_STOP:
			*chg_allowed = FALSE;
			*hchg_allowed = FALSE;
			break;
	}

	/* update temp_fault */
	if (PSTAT_NORMAL == pState)
		*temp_fault = FALSE;
	else
		*temp_fault = TRUE;

	return pState;
}

static int is_support_super_charger(void)
{
	return TRUE;
}

static int flyer_battery_charging_ctrl(enum batt_ctl_t ctl)
{
	int result = 0;

	switch (ctl) {
	case DISABLE:
		pr_info("batt: flyer charger OFF\n");
		if (system_rev >= XC)
			result = gpio_direction_output(FLYER_GPIO_MCHG_EN_N_XC, 1);
		else
			result = gpio_direction_output(FLYER_GPIO_MCHG_EN_N, 1);
		break;
	case ENABLE_SLOW_CHG:
		if (1 <= engineerid) {
			pr_info("batt: flyer charger ON (SLOW) - adapter mode\n");
			result = gpio_direction_output(FLYER_GPIO_ISET_XC, 1);
			result = gpio_direction_output(FLYER_GPIO_MCHG_EN_N_XC, 0);
			gpio_set_value(PM8058_GPIO_PM_TO_SYS(FLYER_ADP_9V), 0);
		} else if (system_rev >= XC) {
			pr_info("batt: flyer charger ON (SLOW)\n");
			result = gpio_direction_output(FLYER_GPIO_ISET_XC, 0);
			result = gpio_direction_output(FLYER_GPIO_MCHG_EN_N_XC, 0);
			gpio_set_value(PM8058_GPIO_PM_TO_SYS(FLYER_ADP_9V), 0);
		} else {
			pr_info("batt: flyer charger ON (SLOW)\n");
			result = gpio_direction_output(FLYER_GPIO_ISET, 0);
			result = gpio_direction_output(FLYER_GPIO_MCHG_EN_N, 0);
			result = gpio_direction_output(FLYER_GPIO_ADP_9V, 0);
		}
		break;
	case ENABLE_FAST_CHG:
		pr_info("batt: flyer charger ON (SUPER)\n");
		if (system_rev >= XC) {
			result = gpio_direction_output(FLYER_GPIO_ISET_XC, 1);
			result = gpio_direction_output(FLYER_GPIO_MCHG_EN_N_XC, 0);
			gpio_set_value(PM8058_GPIO_PM_TO_SYS(FLYER_ADP_9V), 1);
		} else {
			result = gpio_direction_output(FLYER_GPIO_ISET, 1);
			result = gpio_direction_output(FLYER_GPIO_MCHG_EN_N, 0);
			result = gpio_direction_output(FLYER_GPIO_ADP_9V, 1);
		}
		break;
	case ENABLE_SUPER_CHG:
		pr_info("batt: flyer charger ON (SUPER)\n");
		if (system_rev >= XC) {
			result = gpio_direction_output(FLYER_GPIO_ISET_XC, 1);
			result = gpio_direction_output(FLYER_GPIO_MCHG_EN_N_XC, 0);
			gpio_set_value(PM8058_GPIO_PM_TO_SYS(FLYER_ADP_9V), 1);
		} else {
			result = gpio_direction_output(FLYER_GPIO_ISET, 1);
			result = gpio_direction_output(FLYER_GPIO_MCHG_EN_N, 0);
			result = gpio_direction_output(FLYER_GPIO_ADP_9V, 1);
		}
		break;
	case TOGGLE_CHARGER:
		pr_info("batt: flyer toggle charger(->OFF->ON)\n");
		if (system_rev >= XC)
			result = gpio_direction_output(FLYER_GPIO_MCHG_EN_N_XC, 1);
		else
			result = gpio_direction_output(FLYER_GPIO_MCHG_EN_N, 1);
		udelay(200);
		if (system_rev >= XC)
			result = gpio_direction_output(FLYER_GPIO_MCHG_EN_N_XC, 0);
		else
			result = gpio_direction_output(FLYER_GPIO_MCHG_EN_N, 0);
		break;
	case ENABLE_MIN_TAPER: /* express only */
	case DISABLE_MIN_TAPER: /* express ondly */
	default:
		pr_info("%s: Not supported battery ctr(%d) called.!\n", __func__, ctl);
		result = -EINVAL;
		break;
	}

	return result;
}

static int flyer_battery_gpio_init(void)
{

	if (system_rev >= XC) {
		gpio_request(FLYER_GPIO_MCHG_EN_N_XC, "charger_en");
		gpio_request(FLYER_GPIO_ISET_XC, "charge_current");
		pm8xxx_gpio_cfg(FLYER_ADP_9V, PM_GPIO_DIR_OUT, PM_GPIO_OUT_BUF_CMOS, 1, PM_GPIO_PULL_NO,
		PM_GPIO_VIN_BB, PM_GPIO_STRENGTH_HIGH, PM_GPIO_FUNC_NORMAL, 0);
	} else {
		gpio_request(FLYER_GPIO_MCHG_EN_N, "charger_en");
		gpio_request(FLYER_GPIO_ISET, "charge_current");
		gpio_request(FLYER_GPIO_ADP_9V, "super_charge_current");
	}
	return 0;
}

static struct htc_battery_platform_data htc_battery_pdev_data = {
	.func_show_batt_attr = htc_battery_show_attr,
	.func_is_support_super_charger = is_support_super_charger,
	.func_battery_charging_ctrl = flyer_battery_charging_ctrl,
	.func_battery_gpio_init = flyer_battery_gpio_init,
	.guage_driver = GUAGE_DS2746,
	.charger = LINEAR_CHARGER,
	.m2a_cable_detect = 1,
};

static struct platform_device htc_battery_pdev = {
	.name = "htc_battery",
	.id = -1,
	.dev	= {
		.platform_data = &htc_battery_pdev_data,
	},
};

/* battery parameters */
UINT32 m_parameter_unknown_3700mah[] = {
	/* capacity (in 0.01%) -> voltage (in mV)*/
	/* ex: 100% (in 0.01%) -> 4.132 (in mV) for the first two elements.*/
	10000, 4132, 9000 ,4104, 8000, 4019, 7000, 3949, 6000, 3884, 5000, 3827,
	4000, 3796, 3000, 3775, 2000, 3751, 1000, 3685, 0, 3572
};
UINT32 m_parameter_atl_3840mah[] = {
	10000, 4132, 9000 ,4088, 8000, 4007, 7000, 3939, 6000, 3874, 5000, 3821,
	4000, 3789, 3000, 3766, 2000, 3739, 1000, 3682, 0, 3575
};
UINT32 m_parameter_lg_3850mah[] = {
	10000, 4132, 9000 ,4104, 8000, 4019, 7000, 3949, 6000, 3884, 5000, 3827,
	4000, 3796, 3000, 3775, 2000, 3751, 1000, 3685, 0, 3572
};
UINT32 m_parameter_old_lg_3260mah[] =
{
	10000, 4135, 7500, 3960, 4700, 3800, 1700,
	3727, 900, 3674, 300, 3640, 0, 3575,
};

/* arrays' size listed below should be the same =
	num of batteryID + 1 (for unknown batt) */
/* note: flyer only has 2 battery ids, why ary size is 4:
	to compatible to old battery before XD device,
	we define ID=3 for old device battery param. */
static UINT32* m_param_tbl[] = {
	m_parameter_unknown_3700mah,
	m_parameter_atl_3840mah,
	m_parameter_lg_3850mah,
	m_parameter_old_lg_3260mah,
};
static UINT32 fl_25[] = {3700, 3840, 3850, 3260};
static UINT32 pd_m_coef[] = {14, 14, 14, 30};
static UINT32 pd_m_resl[] = {100, 100, 100, 100};
static UINT32 pd_t_coef[] = {70, 70, 70, 250};
static INT32 padc[] = {200, 200, 200, 200};
static INT32 pw[] = {5, 5, 5, 5};

/* arrays' size listed below should be the same =
	num of temp_index level
	(only one level for flyer) */
static UINT32* pd_m_coef_tbl[] = {pd_m_coef,};
static UINT32* pd_m_resl_tbl[] = {pd_m_resl,};
static UINT32 capacity_deduction_tbl_01p[] = {0,};

static struct battery_parameter flyer_battery_parameter = {
	.fl_25 = fl_25,
	.pd_m_coef_tbl = pd_m_coef_tbl,
	.pd_m_coef_tbl_boot = pd_m_coef_tbl,
	.pd_m_resl_tbl = pd_m_resl_tbl,
	.pd_m_resl_tbl_boot = pd_m_resl_tbl,
	.pd_t_coef = pd_t_coef,
	.padc = padc,
	.pw = pw,
	.capacity_deduction_tbl_01p = capacity_deduction_tbl_01p,
	.id_tbl = flyer_battery_id_tbl,
	.temp_index_tbl = NULL,
	.m_param_tbl = m_param_tbl,
	.m_param_tbl_size = sizeof(m_param_tbl)/sizeof(UINT32*),

	.voltage_adc_to_mv_coef = 244,
	.voltage_adc_to_mv_resl = 100,
	.current_adc_to_mv_coef = 625,
	.current_adc_to_mv_resl = 1580,
	.discharge_adc_to_mv_coef = 625,
	.discharge_adc_to_mv_resl = 1580,
	.acr_adc_to_mv_coef = 625,
	.acr_adc_to_mv_resl = 1580,
	.charge_counter_zero_base_mAh = 500,
	.id_adc_overflow = 3067,
	.id_adc_resl = 2047,
	.temp_adc_resl = 2047,
};

static ds2746_platform_data ds2746_pdev_data = {
	.func_get_thermal_id = get_thermal_id,
	.func_get_battery_id = get_battery_id,
	.func_poweralg_config_init = flyer_poweralg_config_init,
	.func_update_charging_protect_flag = flyer_update_charging_protect_flag,
	.r2_kohm = 300,
	.batt_param = &flyer_battery_parameter,
};

static struct platform_device ds2746_battery_pdev = {
	.name = "ds2746-battery",
	.id = -1,
	.dev = {
		.platform_data = &ds2746_pdev_data,
	},
};

#ifdef CONFIG_SUPPORT_DQ_BATTERY
static int __init check_dq_setup(char *str)
{
	if (!strcmp(str, "PASS"))
		tps65200_data.dq_result = 1;
	else
		tps65200_data.dq_result = 0;

	return 1;
}
__setup("androidboot.dq=", check_dq_setup);
#endif

#ifdef CONFIG_SERIAL_MSM_HS
static struct msm_serial_hs_platform_data msm_uart_dm1_pdata = {
	.inject_rx_on_wakeup = 0,
	.cpu_lock_supported = 1,
#ifdef CONFIG_SERIAL_BCM_BT_LPM
        .exit_lpm_cb = bcm_bt_lpm_exit_lpm_locked,
#endif
	/* for bcm BT */
	.bt_wakeup_pin_supported = 1,
	.bt_wakeup_pin = FLYER_GPIO_BT_CHIP_WAKE,
	.host_wakeup_pin = FLYER_GPIO_BT_HOST_WAKE,
};

#ifdef CONFIG_SERIAL_BCM_BT_LPM
static struct bcm_bt_lpm_platform_data bcm_bt_lpm_pdata = {
  .gpio_wake = FLYER_GPIO_BT_CHIP_WAKE,
  .gpio_host_wake = FLYER_GPIO_BT_HOST_WAKE,
  .request_clock_off_locked = msm_hs_request_clock_off_locked,
  .request_clock_on_locked = msm_hs_request_clock_on_locked,
};

struct platform_device bcm_bt_lpm_device = {
  .name = "bcm_bt_lpm",
  .id = 0,
  .dev = {
    .platform_data = &bcm_bt_lpm_pdata,
  },
};
#endif
#endif

#ifdef CONFIG_BT
static struct platform_device flyer_rfkill = {
	.name = "flyer_rfkill",
	.id = -1,
};
#endif


static struct resource ram_console_resources[] = {
	{
		.start  = MSM_RAM_CONSOLE_BASE,
		.end    = MSM_RAM_CONSOLE_BASE + MSM_RAM_CONSOLE_SIZE - 1,
		.flags  = IORESOURCE_MEM,
	},
};

static struct platform_device ram_console_device = {
	.name           = "ram_console",
	.id             = -1,
	.num_resources  = ARRAY_SIZE(ram_console_resources),
	.resource       = ram_console_resources,
};

static struct platform_device *devices[] __initdata = {
        &ram_console_device,
#if defined(CONFIG_SERIAL_MSM) || defined(CONFIG_MSM_SERIAL_DEBUGGER)
        &msm_device_uart2,
#endif
#ifdef CONFIG_SERIAL_BCM_BT_LPM
        &bcm_bt_lpm_device,
#endif
#ifdef CONFIG_MSM_PROC_COMM_REGULATOR
        &msm_proccomm_regulator_dev,
#endif
        &asoc_msm_pcm,
        &asoc_msm_dai0,
        &asoc_msm_dai1,
#if defined(CONFIG_SND_MSM_MVS_DAI_SOC)
        &asoc_msm_mvs,
        &asoc_mvs_dai0,
        &asoc_mvs_dai1,
#endif
        &msm_device_smd,
        &msm_device_dmov,
        &qsd_device_spi,
#ifdef CONFIG_MSM_SSBI
        &msm_device_ssbi_pmic1,
#endif
#ifdef CONFIG_I2C_SSBI
#if 0
        &msm_device_ssbi6,
#endif
        &msm_device_ssbi7,
#endif
        &android_pmem_device,
        &msm_migrate_pages_device,
#ifdef CONFIG_MSM_ROTATOR
        &msm_rotator_device,
#endif
        &android_pmem_adsp_device,
#ifdef CONFIG_ION_MSM
		&ion_dev,
#endif
        &msm_device_i2c,
        &msm_device_i2c_2,
        &hs_device,
#ifdef CONFIG_MSM7KV2_AUDIO
        &msm_aictl_device,
        &msm_mi2s_device,
        &msm_lpa_device,
        &msm_aux_pcm_device,
#endif
#ifdef CONFIG_S5K4E1GX
		&msm_camera_sensor_s5k4e1gx,
#endif
#ifdef CONFIG_S5K6AAFX
		&msm_camera_sensor_s5k6aafx,
#endif
        &msm_device_adspdec,
        &qup_device_i2c,
        &msm_kgsl_3d0,
        &msm_kgsl_2d0,
        &msm_device_vidc_720p,
#ifdef CONFIG_MSM_GEMINI
        &msm_gemini_device,
#endif
#ifdef CONFIG_MSM_VPE
        &msm_vpe_device,
#endif
#if defined(CONFIG_TSIF) || defined(CONFIG_TSIF_MODULE)
        &msm_device_tsif,
#endif

#if defined(CONFIG_CRYPTO_DEV_QCRYPTO) || \
                defined(CONFIG_CRYPTO_DEV_QCRYPTO_MODULE)
        &qcrypto_device,
#endif

#if defined(CONFIG_CRYPTO_DEV_QCEDEV) || \
                defined(CONFIG_CRYPTO_DEV_QCEDEV_MODULE)
        &qcedev_device,
#endif

        &htc_battery_pdev,
		&ds2746_battery_pdev,
        &msm_ebi0_thermal,
        &msm_ebi1_thermal,
#ifdef CONFIG_SERIAL_MSM_HS
        &msm_device_uart_dm1,
#endif
#ifdef CONFIG_BT
        &flyer_rfkill,
#endif

        &gpio_leds,
        &htc_drm, //Might need to remove
};

static struct msm_gpio msm_i2c_gpios_hw[] = {
	{ GPIO_CFG(70, 1, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_16MA), "i2c_scl" },
	{ GPIO_CFG(71, 1, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_16MA), "i2c_sda" },
};

static struct msm_gpio msm_i2c_gpios_io[] = {
	{ GPIO_CFG(70, 0, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_16MA), "i2c_scl" },
	{ GPIO_CFG(71, 0, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_16MA), "i2c_sda" },
};

static struct msm_gpio qup_i2c_gpios_io[] = {
	{ GPIO_CFG(16, 0, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_16MA), "qup_scl" },
	{ GPIO_CFG(17, 0, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_16MA), "qup_sda" },
};
static struct msm_gpio qup_i2c_gpios_hw[] = {
	{ GPIO_CFG(16, 2, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_16MA), "qup_scl" },
	{ GPIO_CFG(17, 2, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_16MA), "qup_sda" },
};

static void
msm_i2c_gpio_config(int adap_id, int config_type)
{
	struct msm_gpio *msm_i2c_table;

	/* Each adapter gets 2 lines from the table */
	if (adap_id > 0)
		return;
	if (config_type)
		msm_i2c_table = &msm_i2c_gpios_hw[adap_id*2];
	else
		msm_i2c_table = &msm_i2c_gpios_io[adap_id*2];
	msm_gpios_enable(msm_i2c_table, 2);
}
/*This needs to be enabled only for OEMS*/
#ifndef CONFIG_QUP_EXCLUSIVE_TO_CAMERA
static struct vreg *qup_vreg;
#endif
static void
qup_i2c_gpio_config(int adap_id, int config_type)
{
	int rc = 0;
	struct msm_gpio *qup_i2c_table;
	/* Each adapter gets 2 lines from the table */
	if (adap_id != 4)
		return;
	if (config_type)
		qup_i2c_table = qup_i2c_gpios_hw;
	else
		qup_i2c_table = qup_i2c_gpios_io;
	rc = msm_gpios_enable(qup_i2c_table, 2);
	if (rc < 0)
		printk(KERN_ERR "QUP GPIO enable failed: %d\n", rc);
	/*This needs to be enabled only for OEMS*/
#ifndef CONFIG_QUP_EXCLUSIVE_TO_CAMERA
	if (qup_vreg) {
		int rc = vreg_set_level(qup_vreg, 1800);
		if (rc) {
			pr_err("%s: vreg LVS1 set level failed (%d)\n",
			__func__, rc);
		}
		rc = vreg_enable(qup_vreg);
		if (rc) {
			pr_err("%s: vreg_enable() = %d \n",
			__func__, rc);
		}
	}
#endif
}

static struct msm_i2c_platform_data msm_i2c_pdata = {
	.clk_freq = 400000,
	.pri_clk = 70,
	.pri_dat = 71,
	.rmutex  = 1,
	.rsl_id = "D:I2C02000021",
	.msm_i2c_config_gpio = msm_i2c_gpio_config,
};

static void __init msm_device_i2c_init(void)
{
	if (msm_gpios_request(msm_i2c_gpios_hw, ARRAY_SIZE(msm_i2c_gpios_hw)))
		pr_err("failed to request I2C gpios\n");

	msm_device_i2c.dev.platform_data = &msm_i2c_pdata;
}

static struct msm_i2c_platform_data msm_i2c_2_pdata = {
	.clk_freq = 100000,
	.rmutex  = 1,
	.rsl_id = "D:I2C02000022",
	.msm_i2c_config_gpio = msm_i2c_gpio_config,
};

static void __init msm_device_i2c_2_init(void)
{
	msm_device_i2c_2.dev.platform_data = &msm_i2c_2_pdata;
}

static struct msm_i2c_platform_data qup_i2c_pdata = {
	.clk_freq = 384000,
	.msm_i2c_config_gpio = qup_i2c_gpio_config,
};

static void __init qup_device_i2c_init(void)
{
	/* Remove the gpio_request due to i2c-qup.c is done so. */
	/*if (msm_gpios_request(qup_i2c_gpios_hw, ARRAY_SIZE(qup_i2c_gpios_hw)))
		pr_err("failed to request I2C gpios\n");*/

	qup_device_i2c.dev.platform_data = &qup_i2c_pdata;
	/*This needs to be enabled only for OEMS*/
#ifndef CONFIG_QUP_EXCLUSIVE_TO_CAMERA
	qup_vreg = vreg_get(NULL, "lvsw1");
	if (IS_ERR(qup_vreg)) {
		printk(KERN_ERR "%s: vreg get failed (%ld)\n",
			__func__, PTR_ERR(qup_vreg));
	}
#endif
}

#ifdef CONFIG_I2C_SSBI
#if 0
static struct msm_i2c_ssbi_platform_data msm_i2c_ssbi6_pdata = {
	.rsl_id = "D:PMIC_SSBI",
	.controller_type = MSM_SBI_CTRL_SSBI2,
};
#endif

static struct msm_i2c_ssbi_platform_data msm_i2c_ssbi7_pdata = {
	.rsl_id = "D:CODEC_SSBI",
	.controller_type = MSM_SBI_CTRL_SSBI,
};
#endif

static void __init flyer_init_irq(void)
{
	msm_init_irq();
}

struct vreg *vreg_s3;
struct vreg *vreg_mmc;

#if (defined(CONFIG_MMC_MSM_SDC1_SUPPORT)\
	|| defined(CONFIG_MMC_MSM_SDC2_SUPPORT)\
	|| defined(CONFIG_MMC_MSM_SDC3_SUPPORT)\
	|| defined(CONFIG_MMC_MSM_SDC4_SUPPORT))

struct sdcc_gpio {
	struct msm_gpio *cfg_data;
	uint32_t size;
	struct msm_gpio *sleep_cfg_data;
};
#if defined(CONFIG_MMC_MSM_SDC1_SUPPORT)
static struct msm_gpio sdc1_lvlshft_cfg_data[] = {
	{GPIO_CFG(35, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_16MA), "sdc1_lvlshft"},
};
#endif
static struct msm_gpio sdc1_cfg_data[] = {
	{GPIO_CFG(38, 1, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_16MA), "sdc1_clk"},
	{GPIO_CFG(39, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA), "sdc1_cmd"},
	{GPIO_CFG(40, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA), "sdc1_dat_3"},
	{GPIO_CFG(41, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA), "sdc1_dat_2"},
	{GPIO_CFG(42, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA), "sdc1_dat_1"},
	{GPIO_CFG(43, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA), "sdc1_dat_0"},
};

static struct msm_gpio sdc2_cfg_data[] = {
	{GPIO_CFG(64, 1, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_16MA), "sdc2_clk"},
	{GPIO_CFG(65, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA), "sdc2_cmd"},
	{GPIO_CFG(66, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA), "sdc2_dat_3"},
	{GPIO_CFG(67, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA), "sdc2_dat_2"},
	{GPIO_CFG(68, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA), "sdc2_dat_1"},
	{GPIO_CFG(69, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA), "sdc2_dat_0"},

#ifdef CONFIG_MMC_MSM_SDC2_8_BIT_SUPPORT
	{GPIO_CFG(115, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA), "sdc2_dat_4"},
	{GPIO_CFG(114, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA), "sdc2_dat_5"},
	{GPIO_CFG(113, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA), "sdc2_dat_6"},
	{GPIO_CFG(112, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA), "sdc2_dat_7"},
#endif
};

static struct msm_gpio sdc3_cfg_data[] = {
	{GPIO_CFG(110, 1, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_16MA), "sdc3_clk"},
	{GPIO_CFG(111, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA), "sdc3_cmd"},
	{GPIO_CFG(116, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA), "sdc3_dat_3"},
	{GPIO_CFG(117, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA), "sdc3_dat_2"},
	{GPIO_CFG(118, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA), "sdc3_dat_1"},
	{GPIO_CFG(119, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA), "sdc3_dat_0"},
};

static struct msm_gpio sdc4_cfg_data[] = {
	{GPIO_CFG(58, 1, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_16MA), "sdc4_clk"},
	{GPIO_CFG(59, 1, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_10MA), "sdc4_cmd"},
	{GPIO_CFG(60, 1, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_10MA), "sdc4_dat_3"},
	{GPIO_CFG(61, 1, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_10MA), "sdc4_dat_2"},
	{GPIO_CFG(62, 1, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_10MA), "sdc4_dat_1"},
	{GPIO_CFG(63, 1, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_10MA), "sdc4_dat_0"},
};

static struct msm_gpio sdc3_sleep_cfg_data[] = {
	{GPIO_CFG(110, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
			"sdc3_clk"},
	{GPIO_CFG(111, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
			"sdc3_cmd"},
	{GPIO_CFG(116, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
			"sdc3_dat_3"},
	{GPIO_CFG(117, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
			"sdc3_dat_2"},
	{GPIO_CFG(118, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
			"sdc3_dat_1"},
	{GPIO_CFG(119, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
			"sdc3_dat_0"},
};

static struct msm_gpio sdc4_sleep_cfg_data[] = {
	{GPIO_CFG(58, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
			"sdc4_clk"},
	{GPIO_CFG(59, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),
			"sdc4_cmd"},
	{GPIO_CFG(60, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),
			"sdc4_dat_3"},
	{GPIO_CFG(61, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),
			"sdc4_dat_2"},
	{GPIO_CFG(62, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),
			"sdc4_dat_1"},
	{GPIO_CFG(63, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),
			"sdc4_dat_0"},
};

static struct sdcc_gpio sdcc_cfg_data[] = {
	{
		.cfg_data = sdc1_cfg_data,
		.size = ARRAY_SIZE(sdc1_cfg_data),
		.sleep_cfg_data = NULL,
	},
	{
		.cfg_data = sdc2_cfg_data,
		.size = ARRAY_SIZE(sdc2_cfg_data),
		.sleep_cfg_data = NULL,
	},
	{
		.cfg_data = sdc3_cfg_data,
		.size = ARRAY_SIZE(sdc3_cfg_data),
		.sleep_cfg_data = sdc3_sleep_cfg_data,
	},
	{
		.cfg_data = sdc4_cfg_data,
		.size = ARRAY_SIZE(sdc4_cfg_data),
		.sleep_cfg_data = sdc4_sleep_cfg_data,
	},
};

struct sdcc_vreg {
	struct vreg *vreg_data;
	unsigned level;
};

static struct sdcc_vreg sdcc_vreg_data[4];

static unsigned long vreg_sts, gpio_sts;

static uint32_t msm_sdcc_setup_gpio(int dev_id, unsigned int enable)
{
	int rc = 0;
	struct sdcc_gpio *curr;

	curr = &sdcc_cfg_data[dev_id - 1];

	if (!(test_bit(dev_id, &gpio_sts)^enable))
		return rc;

	if (enable) {
		set_bit(dev_id, &gpio_sts);
		rc = msm_gpios_request_enable(curr->cfg_data, curr->size);
		if (rc)
			printk(KERN_ERR "%s: Failed to turn on GPIOs for slot %d\n",
				__func__,  dev_id);
	} else {
		clear_bit(dev_id, &gpio_sts);
		if (curr->sleep_cfg_data) {
			msm_gpios_enable(curr->sleep_cfg_data, curr->size);
			msm_gpios_free(curr->sleep_cfg_data, curr->size);
		} else {
			msm_gpios_disable_free(curr->cfg_data, curr->size);
		}
	}

	return rc;
}

static uint32_t msm_sdcc_setup_vreg(int dev_id, unsigned int enable)
{
	int rc = 0;
	struct sdcc_vreg *curr;
	static int enabled_once[] = {0, 0, 0, 0};

	curr = &sdcc_vreg_data[dev_id - 1];

	if (!(test_bit(dev_id, &vreg_sts)^enable))
		return rc;

	if (dev_id != 4) {
		if (!enable || enabled_once[dev_id - 1])
			return 0;
	}

	if (enable) {
		if (dev_id == 4) {
			printk(KERN_INFO "%s: Enabling SD slot power\n", __func__);
			mdelay(5);
		}
		set_bit(dev_id, &vreg_sts);
		rc = vreg_set_level(curr->vreg_data, curr->level);
		if (rc) {
			printk(KERN_ERR "%s: vreg_set_level() = %d \n",
					__func__, rc);
		}
		rc = vreg_enable(curr->vreg_data);
		if (rc) {
			printk(KERN_ERR "%s: vreg_enable() = %d \n",
					__func__, rc);
		}
		enabled_once[dev_id - 1] = 1;
	} else {
		if (dev_id == 4) {
			printk(KERN_INFO "%s: Disabling SD slot power\n", __func__);
			mdelay(5);
		}
		clear_bit(dev_id, &vreg_sts);
		rc = vreg_disable(curr->vreg_data);
		if (rc) {
			printk(KERN_ERR "%s: vreg_disable() = %d \n",
					__func__, rc);
		}
	}
	return rc;
}

static uint32_t msm_sdcc_setup_power(struct device *dv, unsigned int vdd)
{
	int rc = 0;
	struct platform_device *pdev;

	pdev = container_of(dv, struct platform_device, dev);
	rc = msm_sdcc_setup_gpio(pdev->id, (vdd ? 1 : 0));
	if (rc)
		goto out;

	if (pdev->id == 4) /* S3 is always ON and cannot be disabled */
		rc = msm_sdcc_setup_vreg(pdev->id, (vdd ? 1 : 0));
out:
	return rc;
}

#endif

#ifdef CONFIG_MMC_MSM_SDC4_SUPPORT

#ifdef CONFIG_MMC_MSM_CARD_HW_DETECTION
static unsigned int msm7x30_sdcc_slot_status(struct device *dev)
{
	return (unsigned int)
		!gpio_get_value_cansleep(
			PM8058_GPIO_PM_TO_SYS(FLYER_SDMC_CD_N));
}
#endif
#endif

#if defined(CONFIG_MMC_MSM_SDC1_SUPPORT)
static struct mmc_platform_data msm7x30_sdc1_data = {
	.ocr_mask	= MMC_VDD_165_195,
	.translate_vdd	= msm_sdcc_setup_power,
	.mmc_bus_width  = MMC_CAP_4_BIT_DATA,
	.msmsdcc_fmin	= 144000,
	.msmsdcc_fmid	= 24576000,
	.msmsdcc_fmax	= 49152000,
	.nonremovable	= 0,
};
#endif

#ifdef CONFIG_MMC_MSM_SDC2_SUPPORT
static unsigned int flyer_sdc2_slot_type = MMC_TYPE_MMC;
static struct mmc_platform_data msm7x30_sdc2_data = {
	.ocr_mask       = MMC_VDD_165_195 | MMC_VDD_27_28,
#ifdef CONFIG_MMC_MSM_SDC2_8_BIT_SUPPORT
	.mmc_bus_width  = MMC_CAP_8_BIT_DATA,
#else
	.mmc_bus_width  = MMC_CAP_4_BIT_DATA,
#endif
	.msmsdcc_fmin	= 144000,
	.msmsdcc_fmid	= 24576000,
	.msmsdcc_fmax	= 49152000,
	.slot_type		= &flyer_sdc2_slot_type,
	.nonremovable	= 1,
	.emmc_dma_ch	= 7,
};
#endif

#ifdef CONFIG_MMC_MSM_SDC3_SUPPORT
/* HTC_WIFI_START */
/*
static unsigned int flyer_sdc3_slot_type = MMC_TYPE_SDIO_WIFI;
static struct mmc_platform_data msm7x30_sdc3_data = {
	.ocr_mask	= MMC_VDD_27_28 | MMC_VDD_28_29,
	.translate_vdd	= msm_sdcc_setup_power,
	.mmc_bus_width  = MMC_CAP_4_BIT_DATA,
#ifdef CONFIG_MMC_MSM_SDIO_SUPPORT
	.sdiowakeup_irq = MSM_GPIO_TO_INT(118),
#endif
	.msmsdcc_fmin	= 144000,
	.msmsdcc_fmid	= 24576000,
	.msmsdcc_fmax	= 49152000,
	.slot_type		= &flyer_sdc3_slot_type,
	.nonremovable	= 0,
};
*/
/* HTC_WIFI_END */
#endif

#ifdef CONFIG_MMC_MSM_SDC4_SUPPORT
static unsigned int flyer_sdc4_slot_type = MMC_TYPE_SD;
static struct mmc_platform_data msm7x30_sdc4_data = {
	.ocr_mask	= MMC_VDD_27_28 | MMC_VDD_28_29,
	.translate_vdd	= msm_sdcc_setup_power,
	.mmc_bus_width  = MMC_CAP_4_BIT_DATA,

#ifdef CONFIG_MMC_MSM_CARD_HW_DETECTION
	.status      = msm7x30_sdcc_slot_status,
	.status_irq  = PM8058_GPIO_IRQ(PMIC8058_IRQ_BASE, FLYER_SDMC_CD_N),
	.irq_flags   = IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
#endif

#ifdef CONFIG_MMC_MSM_SDC4_WP_SUPPORT
	.wpswitch    = msm_sdcc_get_wpswitch,
#endif
	.msmsdcc_fmin	= 144000,
	.msmsdcc_fmid	= 24576000,
	.msmsdcc_fmax	= 49152000,
	.nonremovable	= 0,
	.slot_type     = &flyer_sdc4_slot_type,
};
#endif

#ifdef CONFIG_MMC_MSM_SDC1_SUPPORT
static void msm_sdc1_lvlshft_enable(void)
{
	int rc;

	/* Enable LDO5, an input to the FET that powers slot 1 */
	rc = vreg_set_level(vreg_mmc, 2850);
	if (rc)
		printk(KERN_ERR "%s: vreg_set_level() = %d \n",	__func__, rc);

	rc = vreg_enable(vreg_mmc);
	if (rc)
		printk(KERN_ERR "%s: vreg_enable() = %d \n", __func__, rc);

	/* Enable GPIO 35, to turn on the FET that powers slot 1 */
	rc = msm_gpios_request_enable(sdc1_lvlshft_cfg_data,
				ARRAY_SIZE(sdc1_lvlshft_cfg_data));
	if (rc)
		printk(KERN_ERR "%s: Failed to enable GPIO 35\n", __func__);

	rc = gpio_direction_output(GPIO_PIN(sdc1_lvlshft_cfg_data[0].gpio_cfg),
				1);
	if (rc)
		printk(KERN_ERR "%s: Failed to turn on GPIO 35\n", __func__);
}
#endif

static void __init msm7x30_init_mmc(void)
{
	vreg_s3 = vreg_get(NULL, "s3");
	if (IS_ERR(vreg_s3)) {
		printk(KERN_ERR "%s: vreg get failed (%ld)\n",
		       __func__, PTR_ERR(vreg_s3));
		return;
	}

	vreg_mmc = vreg_get(NULL, "gp10");
	if (IS_ERR(vreg_mmc)) {
		printk(KERN_ERR "%s: vreg get failed (%ld)\n",
		       __func__, PTR_ERR(vreg_mmc));
		return;
	}

#ifdef CONFIG_MMC_MSM_SDC1_SUPPORT
	if (machine_is_msm7x30_fluid()) {
		msm7x30_sdc1_data.ocr_mask =  MMC_VDD_27_28 | MMC_VDD_28_29;
		msm_sdc1_lvlshft_enable();
	}
	sdcc_vreg_data[0].vreg_data = vreg_s3;
	sdcc_vreg_data[0].level = 1800;
	msm_add_sdcc(1, &msm7x30_sdc1_data);
#endif
#ifdef CONFIG_MMC_MSM_SDC2_SUPPORT
	if (machine_is_msm8x55_svlte_surf())
		msm7x30_sdc2_data.msmsdcc_fmax =  24576000;
	sdcc_vreg_data[1].vreg_data = vreg_s3;
	sdcc_vreg_data[1].level = 1800;
	msm7x30_sdc2_data.swfi_latency =
		msm_pm_data[MSM_PM_SLEEP_MODE_WAIT_FOR_INTERRUPT].latency;
	msm_add_sdcc(2, &msm7x30_sdc2_data);
#endif
#ifdef CONFIG_MMC_MSM_SDC3_SUPPORT
	sdcc_vreg_data[2].vreg_data = vreg_s3;
	sdcc_vreg_data[2].level = 1800;
/* HTC_WIFI_START */
	/*
	msm_sdcc_setup_gpio(3, 1);
	msm_add_sdcc(3, &msm7x30_sdc3_data);
	*/
	flyer_init_mmc(system_rev);
/* HTC_WIFI_END*/
#endif
#ifdef CONFIG_MMC_MSM_SDC4_SUPPORT
	sdcc_vreg_data[3].vreg_data = vreg_mmc;
	sdcc_vreg_data[3].level = 2850;
	msm_add_sdcc(4, &msm7x30_sdc4_data);
#endif

}

static struct msm_spm_platform_data msm_spm_data __initdata = {
	.reg_base_addr = MSM_SAW_BASE,

	.reg_init_values[MSM_SPM_REG_SAW_CFG] = 0x05,
	.reg_init_values[MSM_SPM_REG_SAW_SPM_CTL] = 0x18,
	.reg_init_values[MSM_SPM_REG_SAW_SPM_SLP_TMR_DLY] = 0x00006666,
	.reg_init_values[MSM_SPM_REG_SAW_SPM_WAKE_TMR_DLY] = 0xFF000666,

	.reg_init_values[MSM_SPM_REG_SAW_SLP_CLK_EN] = 0x01,
	.reg_init_values[MSM_SPM_REG_SAW_SLP_HSFS_PRECLMP_EN] = 0x03,
	.reg_init_values[MSM_SPM_REG_SAW_SLP_HSFS_POSTCLMP_EN] = 0x00,

	.reg_init_values[MSM_SPM_REG_SAW_SLP_CLMP_EN] = 0x01,
	.reg_init_values[MSM_SPM_REG_SAW_SLP_RST_EN] = 0x00,
	.reg_init_values[MSM_SPM_REG_SAW_SPM_MPM_CFG] = 0x00,

	.awake_vlevel = 0xF2,
	.retention_vlevel = 0xE0,
	.collapse_vlevel = 0x72,
	.retention_mid_vlevel = 0xE0,
	.collapse_mid_vlevel = 0xE0,

	.vctl_timeout_us = 50,
};

#ifdef CONFIG_PERFLOCK
static unsigned flyer_perf_acpu_table[] = {
	245000000,
	768000000,
	1024000000,
};

static struct perflock_platform_data flyer_perflock_data = {
	.perf_acpu_table = flyer_perf_acpu_table,
	.table_size = ARRAY_SIZE(flyer_perf_acpu_table),
};
#endif

static void flyer_reset(void)
{
	gpio_set_value(FLYER_GPIO_PS_HOLD, 0);
}

void flyer_add_usb_devices(void)
{
	printk(KERN_INFO "%s rev: %d\n", __func__, system_rev);
	android_usb_pdata.products[0].product_id =
			android_usb_pdata.product_id;
	android_usb_pdata.products[0].product_id =
			android_usb_pdata.product_id;
	msm_device_gadget_peripheral.dev.platform_data = &msm_gadget_pdata;

	if (system_rev >= 2) {
		android_usb_pdata.usb_id_pin_gpio = FLYER_GPIO_USB_ID_PIN_XC;
	}
	
#if defined(CONFIG_USB_OTG)
	msm_device_otg.dev.platform_data = &msm_otg_pdata;
	platform_device_register(&msm_device_otg);
#endif
	platform_device_register(&msm_device_gadget_peripheral);
	platform_device_register(&android_usb_device);
}

static int __init board_serialno_setup(char *serialno)
{
	android_usb_pdata.serial_number = serialno;
	return 1;
}

#ifdef CONFIG_MDP4_HW_VSYNC
static void flyer_te_gpio_config(void)
{
	uint32_t te_gpio_table[] = {
	PCOM_GPIO_CFG(30, 1, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA),
	};
	config_gpio_table(te_gpio_table, ARRAY_SIZE(te_gpio_table));
}
#endif

__setup("androidboot.serialno=", board_serialno_setup);
static void __init flyer_init(void)
{
	int rc = 0;
	struct kobject *properties_kobj;
	unsigned smem_size;
	uint32_t soc_version = 0;
	struct proc_dir_entry *entry = NULL;
	char *device_mid;

	soc_version = socinfo_get_version();

	/* Must set msm_hw_reset_hook before first proc comm */
	msm_hw_reset_hook = flyer_reset;

	msm_clock_init(&msm7x30_clock_init_data);
	msm_spm_init(&msm_spm_data, 1);
	acpuclk_init(&acpuclk_7x30_soc_data);

#ifdef CONFIG_PERFLOCK
	perflock_init(&flyer_perflock_data);
#endif

#ifdef CONFIG_BT
	bt_export_bd_address();
#endif

#ifdef CONFIG_SERIAL_MSM_HS
#ifdef CONFIG_SERIAL_BCM_BT_LPM
    msm_uart_dm1_pdata.rx_wakeup_irq = -1;
#else
	msm_uart_dm1_pdata.rx_wakeup_irq = gpio_to_irq(FLYER_GPIO_BT_HOST_WAKE);
	msm_device_uart_dm1.name = "msm_serial_hs_brcm";
#endif
	msm_device_uart_dm1.dev.platform_data = &msm_uart_dm1_pdata;
#endif

#ifdef CONFIG_USB_MSM_OTG_72K
	if (SOCINFO_VERSION_MAJOR(soc_version) >= 2 &&
			SOCINFO_VERSION_MINOR(soc_version) >= 1) {
		pr_debug("%s: SOC Version:2.(1 or more)\n", __func__);
		msm_otg_pdata.ldo_set_voltage = 0;
	}

#ifdef CONFIG_USB_GADGET
	msm_otg_pdata.swfi_latency =
	msm_pm_data
	[MSM_PM_SLEEP_MODE_RAMP_DOWN_AND_WAIT_FOR_INTERRUPT].latency;
	msm_device_gadget_peripheral.dev.platform_data = &msm_gadget_pdata;
#endif
#endif
	msm_device_otg.dev.platform_data = &msm_otg_pdata;

#ifdef CONFIG_MSM_SSBI
	msm_device_ssbi_pmic1.dev.platform_data =
				&msm7x30_ssbi_pm8058_pdata;
#endif
	buses_init();
    device_mid = get_model_id();
	platform_add_devices(devices, ARRAY_SIZE(devices));

	/*usb driver won't be loaded in MFG 58 station and gift mode*/
	if (!(board_mfg_mode() == 6 || board_mfg_mode() == 7))
		flyer_add_usb_devices();


#ifdef CONFIG_USB_EHCI_MSM_72K
	msm_add_host(0, &msm_usb_host_pdata);
#endif
	msm7x30_init_mmc();
	msm_qsd_spi_init();

	spi_register_board_info(msm_spi_board_info, ARRAY_SIZE(msm_spi_board_info));

	msm_pm_set_platform_data(msm_pm_data, ARRAY_SIZE(msm_pm_data));
	BUG_ON(msm_pm_boot_init(MSM_PM_BOOT_CONFIG_RESET_VECTOR, ioremap(0x0, PAGE_SIZE)));

	msm_device_i2c_init();
	msm_device_i2c_2_init();
#ifdef CONFIG_MICROP_COMMON
	flyer_microp_init();
#endif
	qup_device_i2c_init();
	flyer_init_marimba();
#ifdef CONFIG_MSM7KV2_AUDIO
	aux_pcm_gpio_init();
	msm_snddev_init();
	flyer_audio_init();
#endif

#if 0
	if (!machine_is_msm8x55_svlte_ffa() && !machine_is_msm7x30_fluid())
		marimba_pdata.tsadc = &marimba_tsadc_pdata;
#endif

	i2c_register_board_info(2, msm_marimba_board_info,
			ARRAY_SIZE(msm_marimba_board_info));

	i2c_register_board_info(2, msm_i2c_gsbi7_timpani_info,
			ARRAY_SIZE(msm_i2c_gsbi7_timpani_info));


	i2c_register_board_info(4 /* QUP ID */, msm_camera_boardinfo,
			ARRAY_SIZE(msm_camera_boardinfo));

#ifdef CONFIG_I2C_SSBI
#if 0
	msm_device_ssbi6.dev.platform_data = &msm_i2c_ssbi6_pdata;
#endif
	msm_device_ssbi7.dev.platform_data = &msm_i2c_ssbi7_pdata;
#endif

	pm8058_gpios_init();

	entry = create_proc_read_entry("emmc", 0, NULL, emmc_partition_read_proc, NULL);
	if (!entry)
		printk(KERN_ERR"Create /proc/emmc FAILED!\n");

	entry = create_proc_read_entry("dying_processes", 0, NULL, dying_processors_read_proc, NULL);
	if (!entry)
		printk(KERN_ERR"Create /proc/dying_processes FAILED!\n");

	boot_reason = *(unsigned int *)
		(smem_get_entry(SMEM_POWER_ON_STATUS_INFO, &smem_size));
	printk(KERN_NOTICE "Boot Reason = 0x%02x\n", boot_reason);

	i2c_register_board_info(0, i2c_devices_microp, ARRAY_SIZE(i2c_devices_microp));
	if (board_ver >= 30 && board_ver < 43)
		htc_headset_gpio_data.mic_select_gpio = FLYER_AUD_MICPATH_SEL_XC;	

	if (system_rev <= 1) {
		i2c_register_board_info(0, i2c_compass_devices_XA_XB, ARRAY_SIZE(i2c_compass_devices_XA_XB));
	} else if (system_rev == 2) {
		i2c_register_board_info(0, i2c_compass_devices_XC, ARRAY_SIZE(i2c_compass_devices_XC));
	} else if (system_rev == 3) {
		i2c_register_board_info(0, i2c_compass_devices_XD, ARRAY_SIZE(i2c_compass_devices_XD));
	} else {
		i2c_register_board_info(0, i2c_compass_devices_VER_A, ARRAY_SIZE(i2c_compass_devices_VER_A));
	}

	if (system_rev >= 2) {
		flyer_ts_ntrig_data[0].spi_enable = FLYER_GPIO_SPI_ENABLE_XC;
	}

	if (system_rev >= 3) {/* for Camera XD board */
		pr_info("[camera] XD GPIO pin changed\n");
		gCAM_GPIO_SEL = FLYER_CLK_SWITCH_XC;
		gCAM_RST_GPIO = FLYER_CAM_RST_XC;
		gCAM2_PWN_GPIO = FLYER_CAM2_PWD_XD;
	} else if (system_rev == 2) {/* for Camera XC board */
		pr_info("[camera] XC GPIO pin changed\n");
		gCAM_GPIO_SEL = FLYER_CLK_SWITCH_XC;
		gCAM_RST_GPIO = FLYER_CAM_RST_XC;
		gCAM2_PWN_GPIO = FLYER_CAM2_PWD_XC;
	}
#ifdef CONFIG_S5K4E1GX
	msm_camera_sensor_s5k4e1gx_data.sensor_reset = gCAM_RST_GPIO;
	msm_camera_sensor_s5k4e1gx_data.cam_select_pin = gCAM_GPIO_SEL;
#endif
#ifdef CONFIG_S5K6AAFX
	msm_camera_sensor_s5k6aafx_data.sensor_pwd = gCAM2_PWN_GPIO;
	msm_camera_sensor_s5k6aafx_data.cam_select_pin = gCAM_GPIO_SEL;
#endif

	if (system_rev == 2) {/* for Led XC board */
		printk(KERN_INFO "device=%d change gpio_led_config to XC\n",system_rev);
		gpio_leds_data.leds = gpio_led_config_XC;
		gpio_leds_data.num_leds= ARRAY_SIZE(gpio_led_config_XC);
	}
	else if (system_rev >= 3) {/* for Led XD board */
		if(gpio_get_value(PM8058_GPIO_PM_TO_SYS(FLYER_PEN_LED3))){
			printk(KERN_INFO "device=%d change gpio_led_config to XD\n",system_rev);
			gpio_leds_data.leds = gpio_led_config_XD;
			gpio_leds_data.num_leds= ARRAY_SIZE(gpio_led_config_XD);
		}
		else{
			printk(KERN_INFO "device=%d with XD I/O board change gpio_led_config to XD2\n",system_rev);
			gpio_leds_data.leds = gpio_led_config_XD2;
			gpio_leds_data.num_leds= ARRAY_SIZE(gpio_led_config_XD2);

			microp_leds_data.num_leds	= ARRAY_SIZE(up_led_config_XD2);
			microp_leds_data.led_config	= up_led_config_XD2;
		}
	}

#ifdef CONFIG_SENSORS_CAPSENSE
	if (system_rev >= 2) {
		pm8xxx_gpio_cfg(FLYER_NC_PMIC14, PM_GPIO_DIR_OUT, PM_GPIO_OUT_BUF_CMOS, 0, PM_GPIO_PULL_UP_31P5, PM8058_GPIO_VIN_S3, PM_GPIO_STRENGTH_HIGH, PM_GPIO_FUNC_NORMAL, 0);
		mdelay(1);
		if (gpio_get_value(PM8058_GPIO_PM_TO_SYS(FLYER_NC_PMIC14)) == 0) {
			i2c_register_board_info(0, i2c_capsense_devices_XC, ARRAY_SIZE(i2c_capsense_devices_XC));
		}
		pm8xxx_gpio_cfg(FLYER_NC_PMIC14, PM_GPIO_DIR_OUT, PM_GPIO_OUT_BUF_CMOS, 0, PM_GPIO_PULL_DN, PM8058_GPIO_VIN_S3, PM_GPIO_STRENGTH_HIGH, PM_GPIO_FUNC_NORMAL, 0);
	}
#endif
	
	/*Virtual_key*/
	properties_kobj = kobject_create_and_add("board_properties", NULL);
	if (properties_kobj)
		rc = sysfs_create_group(properties_kobj,
				&flyer_properties_attr_group);
	if (!properties_kobj || rc)
		pr_err("failed to create board_properties\n");

	flyer_init_keypad();
#ifdef CONFIG_MDP4_HW_VSYNC
	flyer_te_gpio_config();
#endif
	flyer_init_panel();
	flyer_wifi_init();
}

static unsigned pmem_sf_size = MSM_PMEM_SF_SIZE;
static int __init pmem_sf_size_setup(char *p)
{
	pmem_sf_size = memparse(p, NULL);
	return 0;
}
early_param("pmem_sf_size", pmem_sf_size_setup);

static unsigned fb_size;
static int __init fb_size_setup(char *p)
{
	fb_size = memparse(p, NULL);
	return 0;
}
early_param("fb_size", fb_size_setup);

static unsigned pmem_adsp_size = MSM_PMEM_ADSP_SIZE;
static int __init pmem_adsp_size_setup(char *p)
{
	pmem_adsp_size = memparse(p, NULL);
	return 0;
}
early_param("pmem_adsp_size", pmem_adsp_size_setup);

#ifdef CONFIG_ION_MSM
#ifdef CONFIG_MSM_MULTIMEDIA_USE_ION
static struct ion_co_heap_pdata co_ion_pdata = {
	.adjacent_mem_id = INVALID_HEAP_ID,
	.align = PAGE_SIZE,
};
#endif

/*
 * These heaps are listed in the order they will be allocated.
 * Don't swap the order unless you know what you are doing!
 */

struct ion_platform_heap msm7x30_heaps[] = {
		{
			.id		= ION_SYSTEM_HEAP_ID,
			.type	= ION_HEAP_TYPE_SYSTEM,
			.name	= ION_VMALLOC_HEAP_NAME,
		},
#ifdef CONFIG_MSM_MULTIMEDIA_USE_ION
		/* CAMERA */
		{
			.id		= ION_CAMERA_HEAP_ID,
			.type	= ION_HEAP_TYPE_CARVEOUT,
			.name	= ION_CAMERA_HEAP_NAME,
			.memory_type = ION_EBI_TYPE,
			.extra_data = (void *)&co_ion_pdata,
		},
		/* PMEM_MDP =SF */
		{
			.id		= ION_SF_HEAP_ID,
			.type	= ION_HEAP_TYPE_CARVEOUT,
			.name	= ION_SF_HEAP_NAME,
			.memory_type = ION_EBI_TYPE,
			.extra_data = (void *)&co_ion_pdata,
		},
#endif
		
};

static struct ion_platform_data ion_pdata = {
	.nr = MSM_ION_HEAP_NUM,
	.heaps = msm7x30_heaps,
};

static struct platform_device ion_dev = {
	.name = "ion-msm",
	.id = 1,
	.dev = { .platform_data = &ion_pdata },
};
#endif

static struct memtype_reserve msm7x30_reserve_table[] __initdata = {
	[MEMTYPE_SMI] = {
	},
	[MEMTYPE_EBI0] = {
		.flags	=	MEMTYPE_FLAGS_1M_ALIGN,
	},
	[MEMTYPE_EBI1] = {
		.flags	=	MEMTYPE_FLAGS_1M_ALIGN,
	},
};

unsigned long msm_ion_camera_size;
static void fix_sizes(void)
{
#ifdef CONFIG_ION_MSM
	msm_ion_camera_size = pmem_adsp_size;
#endif
}

static void __init size_pmem_device(struct android_pmem_platform_data *pdata, unsigned long start, unsigned long size)
{
	pdata->start = start;
	pdata->size = size;
	pr_info("%s: allocating %lu bytes at 0x%p (0x%lx physical) for %s\n",
		__func__, size, __va(start), start, pdata->name);
}

static void __init size_pmem_devices(void)
{
#ifdef CONFIG_ANDROID_PMEM
	size_pmem_device(&android_pmem_adsp_pdata, 0, pmem_adsp_size);
#ifndef CONFIG_MSM_MULTIMEDIA_USE_ION
	android_pmem_pdata.size = pmem_sf_size;
#endif
#endif
}

#ifdef CONFIG_ANDROID_PMEM
#ifndef CONFIG_MSM_MULTIMEDIA_USE_ION
static void __init reserve_memory_for(struct android_pmem_platform_data *p)
{
	if (p->size > 0) {
		pr_info("%s: reserve %lu bytes from memory %d for %s.\n", __func__, p->size, p->memory_type, p->name);
		msm7x30_reserve_table[p->memory_type].size += p->size;
	}
}
#endif
#endif

static void __init reserve_pmem_memory(void)
{
#ifdef CONFIG_ANDROID_PMEM
	msm7x30_reserve_table[MEMTYPE_EBI0].size += PMEM_KERNEL_EBI0_SIZE;
#ifndef CONFIG_MSM_MULTIMEDIA_USE_ION
	reserve_memory_for(&android_pmem_pdata);	
#endif
#endif
}

static void __init size_ion_devices(void)
{
#ifdef CONFIG_MSM_MULTIMEDIA_USE_ION
	ion_pdata.heaps[1].size = MSM_ION_CAMERA_SIZE;
	ion_pdata.heaps[2].size = MSM_ION_SF_SIZE;
#endif
}

static void __init reserve_ion_memory(void)
{
#if defined(CONFIG_ION_MSM) && defined(CONFIG_MSM_MULTIMEDIA_USE_ION)
	msm7x30_reserve_table[MEMTYPE_EBI0].size += MSM_ION_CAMERA_SIZE;
	msm7x30_reserve_table[MEMTYPE_EBI0].size += MSM_ION_SF_SIZE;
#endif
}

static void __init msm7x30_calculate_reserve_sizes(void)
{
	fix_sizes();
	size_pmem_devices();
	reserve_pmem_memory();
	size_ion_devices();
	reserve_ion_memory();
}

static int msm7x30_paddr_to_memtype(unsigned int paddr)
{
	if (paddr < 0x40000000)
		return MEMTYPE_EBI0;
	if (paddr >= 0x40000000 && paddr < 0x80000000)
		return MEMTYPE_EBI1;
	return MEMTYPE_NONE;
}

static struct reserve_info msm7x30_reserve_info __initdata = {
	.memtype_reserve_table = msm7x30_reserve_table,
	.calculate_reserve_sizes = msm7x30_calculate_reserve_sizes,
	.paddr_to_memtype = msm7x30_paddr_to_memtype,
};

static void __init flyer_reserve(void)
{
	reserve_info = &msm7x30_reserve_info;
	msm_reserve();
}

static void __init flyer_allocate_memory_regions(void)
{
	void *addr;
	unsigned long size;

	size = MSM_FB_SIZE;
	addr = alloc_bootmem_align(size, 0x1000);
	msm_fb_resources[0].start = __pa(addr);
	msm_fb_base = msm_fb_resources[0].start;
	msm_fb_resources[0].end = msm_fb_resources[0].start + size - 1;
	printk("allocating %lu bytes at %p (%lx physical) for fb\n",
			size, addr, __pa(addr));
}

static void __init flyer_map_io(void)
{
	msm_shared_ram_phys = 0x00400000;
	msm_map_msm7x30_io();
	if (socinfo_init() < 0)
		printk(KERN_ERR "%s: socinfo_init() failed!\n",
		       __func__);
}

static void __init flyer_init_early(void)
{
	flyer_allocate_memory_regions();
}

static void __init flyer_fixup(struct machine_desc *desc, struct tag *tags,
								char **cmdline, struct meminfo *mi)
{
	parse_tag_memsize((const struct tag *)tags);
	engineerid = parse_tag_engineerid(tags);

	mi->nr_banks = 4;
	mi->bank[0].start = MSM_LINUX_BASE1;
	mi->bank[0].size = MSM_LINUX_SIZE1;
	mi->bank[1].start = MSM_LINUX_BASE2;
	mi->bank[1].size = MSM_LINUX_SIZE2;
	mi->bank[2].start = MSM_LINUX_BASE3;
	mi->bank[2].size = MSM_LINUX_SIZE3;
	mi->bank[3].start = MSM_LINUX_BASE4;
	mi->bank[3].size = MSM_LINUX_SIZE4;
}

MACHINE_START(FLYER, "flyer")
	.fixup = flyer_fixup,
	.map_io = flyer_map_io,
	.reserve = flyer_reserve,
	.init_irq = flyer_init_irq,
	.init_machine = flyer_init,
	.timer = &msm_timer,
	.init_early = flyer_init_early,
MACHINE_END
