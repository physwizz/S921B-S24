/* linux/arch/arm/mach-exynos/setup-is-module.c
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * FIMC-IS gpio and clock configuration
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/gpio.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/regulator/consumer.h>
#include <linux/delay.h>
#include <linux/clk-provider.h>
#include <linux/clkdev.h>
#ifdef CONFIG_OF
#include <linux/of_gpio.h>
#endif

#include <exynos-is-module.h>
#include "is-ixc-config.h"
#include "is-vendor.h"
#include "is-time.h"
#include "pablo-debug.h"
#include "is-resourcemgr.h"

static int acquire_shared_rsc(struct exynos_sensor_pin *pin_ctrls)
{
	if (pin_ctrls->shared_rsc_count)
		return atomic_inc_return(pin_ctrls->shared_rsc_count);

	return 1;
}

static int release_shared_rsc(struct exynos_sensor_pin *pin_ctrls)
{
	if (pin_ctrls->shared_rsc_count)
		return atomic_dec_return(pin_ctrls->shared_rsc_count);

	return 0;
}

static bool need_to_control_shared_rsc(struct exynos_sensor_pin *pin_ctrls,
					struct is_module_enum *module)
{
	int active_count = 0;
	u32 delay = pin_ctrls->delay;

	if (pin_ctrls->shared_rsc_type == SRT_ACQUIRE)
		active_count = acquire_shared_rsc(pin_ctrls);
	else if (pin_ctrls->shared_rsc_type == SRT_RELEASE)
		active_count = release_shared_rsc(pin_ctrls);

	minfo(" shared rsc (act(%d), pin(%ld), val(%d), nm(%s), active(cur: %d, target: %d))\n",
			module,
			pin_ctrls->act,
			(pin_ctrls->act == PIN_FUNCTION) ? 0 : pin_ctrls->pin,
			pin_ctrls->value,
			pin_ctrls->name,
			active_count,
			pin_ctrls->shared_rsc_active);

	if (active_count != pin_ctrls->shared_rsc_active) {
		udelay(delay);
		return false;
	}

	return true;
}

static int is_module_regulator_ctrl(struct is_module_regulator *imr,
				struct exynos_sensor_pin *pin_ctrls,
				struct is_module_enum *module,
				struct device *dev)
{
	char* name = pin_ctrls->name;
	u32 value = pin_ctrls->value;
	u32 vol = pin_ctrls->voltage;
	bool retention_pin = imr->retention_pin;
	int ret = 0;

	mutex_lock(&imr->regulator_lock);

	if (value) {
		if (!imr->regulator)
			imr->regulator = regulator_get_optional(dev, name);

		if (IS_ERR_OR_NULL(imr->regulator)) {
			merr("regulator_get(%s) fail", module, name);

			ret = PTR_ERR(imr->regulator);
			imr->regulator = NULL;
			goto regulator_err;
		}

		if (regulator_is_enabled(imr->regulator)) {
			mwarn("regulator(%s) is already enabled", module, name);
			goto regulator_err;
		}

		if (vol > 0) {
			minfo("regulator_set_voltage(%d)", module, vol);
			ret = regulator_set_voltage(imr->regulator, vol, vol);
			if(ret) {
				merr("regulator_set_voltage(%d) fail", module, ret);
			}
		}

		ret = regulator_enable(imr->regulator);

		if (pin_ctrls->actuator_i2c_delay > 0) {
			module->act_available_time = ktime_add_us(ktime_get_boottime(),
							pin_ctrls->actuator_i2c_delay);
		}

		if (ret) {
			merr("regulator_enable(%s) fail", module, name);
			regulator_put(imr->regulator);
			imr->regulator = NULL;
			goto regulator_err;
		}
	} else {
		if (IS_ERR_OR_NULL(imr->regulator)) {
			merr("regulator(%s) get failed", module, name);
			ret = PTR_ERR(imr->regulator);
			goto regulator_err;
		}

		if ((module->ext.use_retention_mode == SENSOR_RETENTION_ACTIVATED
			|| module->ext.use_retention_mode == SENSOR_RETENTION_UNSUPPORTED)
			&& retention_pin) {
			mwarn("[%s] regulator(%s) is retention IO pin\n",
				module, module->sensor_name, name);
			imr->regulator = NULL;
			goto regulator_err;
		}

		if (!regulator_is_enabled(imr->regulator)) {
			mwarn("regulator(%s) is already disabled", module, name);
			regulator_put(imr->regulator);
			imr->regulator = NULL;
			goto regulator_err;
		}

		ret = regulator_disable(imr->regulator);
		if (ret) {
			merr("regulator_disable(%s) fail", module, name);
			regulator_put(imr->regulator);
			imr->regulator = NULL;
			goto regulator_err;
		}

		regulator_put(imr->regulator);
		imr->regulator = NULL;
	}

regulator_err:
	mutex_unlock(&imr->regulator_lock);

	return ret;
}

static int is_module_regulator_ctrl_voltage(struct exynos_sensor_pin *pin_ctrls,
				struct is_module_enum *module,
				struct device *dev)
{
	char *name = pin_ctrls->name;
	u32 vol = pin_ctrls->voltage;
	struct regulator *regulator;
	int ret;

	regulator = regulator_get(dev, name);

	if (IS_ERR_OR_NULL(regulator)) {
		merr("regulator_get(%s) fail", module, name);

		ret = PTR_ERR(regulator);
		return ret;
	}

	if (vol > 0) {
		minfo("regulator_set_voltage(%d)", module, vol);
		ret = regulator_set_voltage(regulator, vol, vol);
		if (ret)
			merr("regulator_set_voltage fail. ret(%d)", module, ret);
	}

	if (pin_ctrls->actuator_i2c_delay > 0)
		module->act_available_time = ktime_add_us(ktime_get_boottime(),
						pin_ctrls->actuator_i2c_delay);

	regulator_put(regulator);

	return 0;
}

static int is_module_regulator_opt(struct exynos_sensor_pin *pin_ctrls,
				struct is_module_enum *module,
				struct device *dev)
{
	char* name = pin_ctrls->name;
	u32 value = pin_ctrls->value;
	struct regulator *regulator;
	int ret;

	regulator = regulator_get(dev, name);
	if (IS_ERR_OR_NULL(regulator)) {
		merr("regulator_get(%s) fail", module, name);
		return PTR_ERR(regulator);
	}

	if (value) {
		ret = regulator_set_mode(regulator, REGULATOR_MODE_FAST);
		if(ret) {
			dev_err(dev, "Failed to configure fPWM mode: %d\n", ret);
			regulator_put(regulator);
			return ret;
		}
	} else {
		ret = regulator_set_mode(regulator, REGULATOR_MODE_NORMAL);
		if (ret) {
			dev_err(dev, "Failed to configure auto mode: %d\n", ret);
			regulator_put(regulator);
			return ret;
		}
	}

	regulator_put(regulator);

	return 0;
}

static int is_module_mclk_ctrl(struct exynos_sensor_pin *pin_ctrls,
				struct is_module_enum *module,
				struct is_device_sensor *ids,
				u32 scenario)
{
	u32 value = pin_ctrls->value;
	int ret;

	if (pin_ctrls->shared_rsc_type) {
		if (value) {
			if (ids->pdata->mclk_on)
				ret = ids->pdata->mclk_on(&ids->pdev->dev,
						scenario, module->pdata->mclk_ch,
						module->pdata->mclk_freq_khz);
			else
				ret = -EPERM;
		} else {
			if (ids->pdata->mclk_off)
				ret = ids->pdata->mclk_off(&ids->pdev->dev,
						scenario, module->pdata->mclk_ch);
			else
				ret = -EPERM;
		}
	} else {
		if (value)
			ret = is_sensor_mclk_on(ids,
					scenario, module->pdata->mclk_ch,
					module->pdata->mclk_freq_khz);
		else
			ret = is_sensor_mclk_off(ids,
					scenario, module->pdata->mclk_ch);
	}

	if (ret) {
		merr(" failed to %s MCLK(%d)", module, value ? "on" : "off",  ret);
		return ret;
	}

	return 0;
}

static int exynos_is_module_pin_control(struct is_module_enum *module,
	struct pinctrl *pinctrl, struct exynos_sensor_pin *pin_ctrls, u32 scenario)
{
	struct device *dev = module->dev;
	char *name = pin_ctrls->name;
	ulong pin = pin_ctrls->pin;
	u32 delay = pin_ctrls->delay;
	u32 value = pin_ctrls->value;
	u32 act = pin_ctrls->act;
	int ret = 0;
	struct v4l2_subdev *subdev_module;
	struct is_device_sensor *sensor;
	struct is_resourcemgr *resourcemgr;

	if (pin_ctrls->shared_rsc_type) {
		mutex_lock(pin_ctrls->shared_rsc_lock);

		if (!need_to_control_shared_rsc(pin_ctrls, module))
			goto exit;
	}

	minfo("[%s] pin_ctrl(%s, val(%d), delay(%d), act(%d), pin(%ld))\n", module,
		module->sensor_name, name, value, delay, act, (act == PIN_FUNCTION) ? 0 : pin);

	subdev_module = module->subdev;
	if (!subdev_module) {
		merr("module's subdev was not probed", module);
		ret = -ENODEV;
		goto exit;
	}

	sensor = (struct is_device_sensor *)v4l2_get_subdev_hostdata(subdev_module);
	if (!sensor) {
		merr("failed to get sensor device", module);
		ret = -EINVAL;
		goto exit;
	}

	resourcemgr = sensor->resourcemgr;
	if (!resourcemgr) {
		merr("failed to get resourcemgr", module);
		ret = -EINVAL;
		goto exit;
	}

	switch (act) {
	case PIN_NONE:
		break;
	case PIN_OUTPUT:
		if (gpio_is_valid(pin)) {
			if (value)
				gpio_request_one(pin, GPIOF_OUT_INIT_HIGH, "CAM_GPIO_OUTPUT_HIGH");
			else
				gpio_request_one(pin, GPIOF_OUT_INIT_LOW, "CAM_GPIO_OUTPUT_LOW");
			gpio_free(pin);
		}
		break;
	case PIN_INPUT:
		if (gpio_is_valid(pin)) {
			gpio_request_one(pin, GPIOF_IN, "CAM_GPIO_INPUT");
			gpio_free(pin);
		}
		break;
	case PIN_RESET:
		if (gpio_is_valid(pin)) {
			gpio_request_one(pin, GPIOF_OUT_INIT_HIGH, "CAM_GPIO_RESET");
			udelay(1000);
			__gpio_set_value(pin, 0);
			udelay(1000);
			__gpio_set_value(pin, 1);
			udelay(1000);
			gpio_free(pin);
		}
		break;
	case PIN_FUNCTION:
		{
			struct pinctrl_state *s = (struct pinctrl_state *)pin;

			ret = pinctrl_select_state(pinctrl, s);
			if (ret < 0) {
				merr("pinctrl_select_state(%s) is fail(%d)", module, name, ret);
				goto exit;
			}
		}
		break;
	case PIN_REGULATOR:
		{
			struct is_module_regulator *imr = NULL;

			list_for_each_entry(imr, &resourcemgr->regulator_list, list) {
				if (!strcmp(imr->name, name))
					break;
			}

			ret = is_module_regulator_ctrl(imr, pin_ctrls, module, dev);
		}
		break;
	case PIN_REGULATOR_VOLTAGE:
		ret = is_module_regulator_ctrl_voltage(pin_ctrls, module, dev);
		break;
	case PIN_REGULATOR_OPTION:
		ret = is_module_regulator_opt(pin_ctrls, module, dev);
		break;
	case PIN_I2C:
		ret = is_ixc_pin_control(module, scenario, value);
		break;
	case PIN_MCLK:
		ret = is_module_mclk_ctrl(pin_ctrls, module, sensor, scenario);
		break;
	default:
		merr("unknown act for pin (%d) %s", module, act, pin_ctrls->name);
		break;
	}

	udelay(delay);

exit:
	if (pin_ctrls->shared_rsc_type)
		mutex_unlock(pin_ctrls->shared_rsc_lock);

	return ret;
}

int exynos_is_module_pins_cfg(struct is_module_enum *module,
	u32 scenario,
	u32 gpio_scenario)
{
	int ret = 0;
	u32 idx_max, idx;
	struct pinctrl *pinctrl;
	struct exynos_sensor_pin (*pin_ctrls)[GPIO_SCENARIO_MAX][GPIO_CTRL_MAX];
	struct exynos_platform_is_module *pdata;

	FIMC_BUG(!module);
	FIMC_BUG(!module->pdata);
	FIMC_BUG(gpio_scenario >= GPIO_SCENARIO_MAX);
	FIMC_BUG(scenario > SENSOR_SCENARIO_MAX);

	pdata = module->pdata;
	pinctrl = pdata->pinctrl;
	pin_ctrls = pdata->pin_ctrls;
	idx_max = pdata->pinctrl_index[scenario][gpio_scenario];

	if (idx_max == 0) {
		err("There is no such a scenario(scen:%d, on:%d)", scenario, gpio_scenario);
		ret = -EINVAL;
		goto p_err;
	}

	minfo("[P%d:S%d:GS%d]: pin_ctrl start\n", module,
		module->position, scenario, gpio_scenario);

	/* do configs */
	for (idx = 0; idx < idx_max; ++idx) {
		ret = exynos_is_module_pin_control(module, pinctrl,
			&pin_ctrls[scenario][gpio_scenario][idx], scenario);
		if (ret) {
			merr("exynos_is_module_pin_control(%d) is fail(%d)", module, idx, ret);
			goto p_err;
		}
	}

	minfo("[P%d:S%d:GS%d]: pin_ctrl end\n", module,
		module->position, scenario, gpio_scenario);
p_err:
	return ret;
}

static int exynos_is_module_pin_debug(struct device *dev,
	struct pinctrl *pinctrl, struct exynos_sensor_pin *pin_ctrls)
{
	int ret = 0;
	ulong pin = pin_ctrls->pin;
	char* name = pin_ctrls->name;
	u32 act = pin_ctrls->act;

	switch (act) {
	case PIN_NONE:
	case PIN_FUNCTION:
		break;
	case PIN_OUTPUT:
	case PIN_INPUT:
	case PIN_RESET:
		if (gpio_is_valid(pin))
			pr_info("[@] pin %s : %d\n", name, gpio_get_value(pin));
		break;
	case PIN_REGULATOR:
		{
			struct regulator *regulator;
			int voltage;

			regulator = regulator_get_optional(dev, name);
			if (IS_ERR(regulator)) {
				pr_err("%s : regulator_get(%s) fail\n", __func__, name);
				return PTR_ERR(regulator);
			}

			if (regulator_is_enabled(regulator))
				voltage = regulator_get_voltage(regulator);
			else
				voltage = 0;

			regulator_put(regulator);

			pr_info("[@] %s LDO : %d\n", name, voltage);
		}
		break;
	default:
		pr_err("unknown act for pin\n");
		break;
	}

	return ret;
}

int exynos_is_module_pins_dbg(struct is_module_enum *module,
	u32 scenario,
	u32 gpio_scenario)
{
	int ret = 0;
	u32 idx_max, idx;
	struct pinctrl *pinctrl;
	struct exynos_sensor_pin (*pin_ctrls)[GPIO_SCENARIO_MAX][GPIO_CTRL_MAX];
	struct exynos_platform_is_module *pdata;

	FIMC_BUG(!module);
	FIMC_BUG(!module->pdata);
	FIMC_BUG(gpio_scenario > 1);
	FIMC_BUG(scenario > SENSOR_SCENARIO_MAX);

	pdata = module->pdata;
	pinctrl = pdata->pinctrl;
	pin_ctrls = pdata->pin_ctrls;
	idx_max = pdata->pinctrl_index[scenario][gpio_scenario];

	/* print configs */
	for (idx = 0; idx < idx_max; ++idx) {
		minfo(" pin_ctrl(act(%d), pin(%ld), val(%d), nm(%s)\n", module,
			pin_ctrls[scenario][gpio_scenario][idx].act,
			(pin_ctrls[scenario][gpio_scenario][idx].act == PIN_FUNCTION) ? 0 : pin_ctrls[scenario][gpio_scenario][idx].pin,
			pin_ctrls[scenario][gpio_scenario][idx].value,
			pin_ctrls[scenario][gpio_scenario][idx].name);
	}

	/* do configs */
	for (idx = 0; idx < idx_max; ++idx) {
		ret = exynos_is_module_pin_debug(module->dev, pinctrl, &pin_ctrls[scenario][gpio_scenario][idx]);
		if (ret) {
			merr("exynos_is_module_pin_debug(%d) is fail(%d)", module, idx, ret);
			goto p_err;
		}
	}

p_err:
	return ret;
}

void is_get_gpio_ops(struct exynos_platform_is_module *pdata)
{
	pdata->gpio_cfg = exynos_is_module_pins_cfg;
	pdata->gpio_dbg = exynos_is_module_pins_dbg;
}
KUNIT_EXPORT_SYMBOL(is_get_gpio_ops);
