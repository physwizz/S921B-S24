/*
 * s2mps28.c
 *
 * Copyright (c) 2023 Samsung Electronics Co., Ltd
 *              http://www.samsung.com
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 */

#include <linux/bug.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
//#include <../drivers/pinctrl/samsung/pinctrl-samsung.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/regmap.h>
#include <linux/platform_device.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include <linux/regulator/of_regulator.h>
#include <linux/samsung/pmic/s2mps28.h>
#include <linux/samsung/pmic/s2mps28-regulator.h>
#include <linux/samsung/pmic/s2mps28-4-info.h>
//#include <linux/reset/exynos/exynos-reset.h>
#include <linux/io.h>
#include <linux/mutex.h>
#include <linux/interrupt.h>
#include <linux/samsung/pmic/pmic_class.h>
#if IS_ENABLED(CONFIG_EXYNOS_ACPM)
#include <soc/samsung/acpm_mfd.h>
static struct device_node *acpm_mfd_node;
#endif

static struct s2mps28_info *s2mps28_static_info;

#if IS_ENABLED(CONFIG_DRV_SAMSUNG_PMIC)
struct pmic_sysfs_dev {
	uint8_t base_addr;
	uint8_t read_addr;
	uint8_t read_val;
	struct device *dev;
};
#endif

struct s2mps28_info {
	struct regulator_dev *rdev[S2MPS28_REGULATOR_MAX];
	unsigned int opmode[S2MPS28_REGULATOR_MAX];
	struct s2mps28_dev *iodev;
	struct mutex lock;
	struct i2c_client *i2c;
	int num_regulators;
	int wtsr_en;
#if IS_ENABLED(CONFIG_DRV_SAMSUNG_PMIC)
	struct pmic_sysfs_dev *pmic_sysfs;
#endif
};

static unsigned int s2mps28_of_map_mode(unsigned int val) {
	switch (val) {
	case SEC_OPMODE_SUSPEND:	/* ON in Standby Mode */
		return 0x1;
	case SEC_OPMODE_MIF:		/* ON in PWREN_MIF mode */
		return 0x2;
	case SEC_OPMODE_ON:		/* ON in Normal Mode */
		return 0x3;
	default:
		return REGULATOR_MODE_INVALID;
	}
}

/* Some LDOs supports [LPM/Normal]ON mode during suspend state */
static int s2m_set_mode(struct regulator_dev *rdev,
			unsigned int mode)
{
	struct s2mps28_info *s2mps28 = rdev_get_drvdata(rdev);
	unsigned int val;
	int id = rdev_get_id(rdev);

	val = mode << S2MPS28_ENABLE_SHIFT;
	s2mps28->opmode[id] = val;

	return 0;
}

static int s2m_enable(struct regulator_dev *rdev)
{
	struct s2mps28_info *s2mps28 = rdev_get_drvdata(rdev);

	return s2mps28_4_update_reg(s2mps28->i2c, rdev->desc->enable_reg,
				  s2mps28->opmode[rdev_get_id(rdev)],
				  rdev->desc->enable_mask);
}

static int s2m_disable_regmap(struct regulator_dev *rdev)
{
	struct s2mps28_info *s2mps28 = rdev_get_drvdata(rdev);
	unsigned int val;

	if (rdev->desc->enable_is_inverted)
		val = rdev->desc->enable_mask;
	else
		val = 0;

	return s2mps28_4_update_reg(s2mps28->i2c, rdev->desc->enable_reg,
				  val, rdev->desc->enable_mask);
}

static int s2m_is_enabled_regmap(struct regulator_dev *rdev)
{
	struct s2mps28_info *s2mps28 = rdev_get_drvdata(rdev);
	int ret;
	uint8_t val;

	ret = s2mps28_4_read_reg(s2mps28->i2c,
			       rdev->desc->enable_reg, &val);
	if (ret)
		return ret;

	if (rdev->desc->enable_is_inverted)
		return (val & rdev->desc->enable_mask) == 0;
	else
		return (val & rdev->desc->enable_mask) != 0;
}
#if 0
static int get_ramp_delay(int ramp_delay)
{
	unsigned char cnt = 0;

	ramp_delay /= 6;

	while (true) {
		ramp_delay = ramp_delay >> 1;
		if (ramp_delay == 0)
			break;
		cnt++;
	}

	return cnt;
}
#endif
static int s2m_set_ramp_delay(struct regulator_dev *rdev, int ramp_delay)
{
	struct s2mps28_info *s2mps28 = rdev_get_drvdata(rdev);
	int reg_id = rdev_get_id(rdev);
	unsigned int ramp_value = 0;
	uint8_t ramp_addr = 0;

	//ramp_value = get_ramp_delay(ramp_delay / 1000);
	//if (ramp_value > 4) {
	//	pr_warn("%s: ramp_delay: %d not supported\n",
	//		rdev->desc->name, ramp_delay);
	//}
	ramp_value = 0x00; 	// 6.25mv/us fixed

	switch (reg_id) {
	case S2MPS28_BUCK1:
		ramp_addr = S2MPS28_PM1_BUCK1_DVS;
		break;
	case S2MPS28_BUCK2:
		ramp_addr = S2MPS28_PM1_BUCK2_DVS;
		break;
	case S2MPS28_BUCK3:
		ramp_addr = S2MPS28_PM1_BUCK3_DVS;
		break;
	case S2MPS28_BUCK4:
		ramp_addr = S2MPS28_PM1_BUCK4_DVS;
		break;
	case S2MPS28_BUCK5:
		ramp_addr = S2MPS28_PM1_BUCK5_DVS;
		break;
	case S2MPS28_BUCK_SR1:
		ramp_addr = S2MPS28_PM1_BUCK_SR1_DVS;
		break;
	default:
		return -EINVAL;
	}

	return s2mps28_4_update_reg(s2mps28->i2c, ramp_addr,
				  ramp_value << BUCK_RAMP_UP_SHIFT,
				  BUCK_RAMP_MASK << BUCK_RAMP_UP_SHIFT);
}

static int s2m_get_voltage_sel_regmap(struct regulator_dev *rdev)
{
	struct s2mps28_info *s2mps28 = rdev_get_drvdata(rdev);
	int ret;
	uint8_t val;

	ret = s2mps28_4_read_reg(s2mps28->i2c, rdev->desc->vsel_reg, &val);
	if (ret)
		return ret;

	val &= rdev->desc->vsel_mask;

	return val;
}

static int s2m_set_voltage_sel_regmap(struct regulator_dev *rdev, unsigned sel)
{
	struct s2mps28_info *s2mps28 = rdev_get_drvdata(rdev);
	int ret;

	ret = s2mps28_4_update_reg(s2mps28->i2c, rdev->desc->vsel_reg,
				 sel, rdev->desc->vsel_mask);
	if (ret < 0) {
		pr_warn("%s: failed to set voltage_sel_regmap\n", rdev->desc->name);
		return -EINVAL;
	}

	if (rdev->desc->apply_bit)
		ret = s2mps28_4_update_reg(s2mps28->i2c, rdev->desc->apply_reg,
					 rdev->desc->apply_bit,
					 rdev->desc->apply_bit);
	return ret;
}

static int s2m_set_voltage_sel_regmap_buck(struct regulator_dev *rdev, unsigned sel)
{
	struct s2mps28_info *s2mps28 = rdev_get_drvdata(rdev);
	int ret = 0;

	ret = s2mps28_4_write_reg(s2mps28->i2c, rdev->desc->vsel_reg, sel);
	if (ret < 0) {
		pr_warn("%s: failed to set voltage_sel_regmap\n", rdev->desc->name);
		return -EINVAL;
	}

	if (rdev->desc->apply_bit)
		ret = s2mps28_4_update_reg(s2mps28->i2c, rdev->desc->apply_reg,
					 rdev->desc->apply_bit,
					 rdev->desc->apply_bit);

	return ret;
}

static int s2m_set_voltage_time_sel(struct regulator_dev *rdev,
				    unsigned int old_selector,
				    unsigned int new_selector)
{
	unsigned int ramp_delay = 0;
	int old_volt, new_volt;

	if (rdev->constraints->ramp_delay)
		ramp_delay = rdev->constraints->ramp_delay;
	else if (rdev->desc->ramp_delay)
		ramp_delay = rdev->desc->ramp_delay;

	if (ramp_delay == 0) {
		pr_warn("%s: ramp_delay not set\n", rdev->desc->name);
		return -EINVAL;
	}

	/* sanity check */
	if (!rdev->desc->ops->list_voltage)
		return -EINVAL;

	old_volt = rdev->desc->ops->list_voltage(rdev, old_selector);
	new_volt = rdev->desc->ops->list_voltage(rdev, new_selector);

	if (old_selector < new_selector)
		return DIV_ROUND_UP(new_volt - old_volt, ramp_delay);
	else
		return DIV_ROUND_UP(old_volt - new_volt, ramp_delay);

	return 0;
}

static struct regulator_ops s2mps28_ldo_ops = {
	.list_voltage		= regulator_list_voltage_linear,
	.map_voltage		= regulator_map_voltage_linear,
	.is_enabled		= s2m_is_enabled_regmap,
	.enable			= s2m_enable,
	.disable		= s2m_disable_regmap,
	.get_voltage_sel	= s2m_get_voltage_sel_regmap,
	.set_voltage_sel	= s2m_set_voltage_sel_regmap,
	.set_voltage_time_sel	= s2m_set_voltage_time_sel,
	.set_mode		= s2m_set_mode,
};

static struct regulator_ops s2mps28_buck_ops = {
	.list_voltage		= regulator_list_voltage_linear,
	.map_voltage		= regulator_map_voltage_linear,
	.is_enabled		= s2m_is_enabled_regmap,
	.enable			= s2m_enable,
	.disable		= s2m_disable_regmap,
	.get_voltage_sel	= s2m_get_voltage_sel_regmap,
	.set_voltage_sel	= s2m_set_voltage_sel_regmap_buck,
	.set_voltage_time_sel	= s2m_set_voltage_time_sel,
	.set_mode		= s2m_set_mode,
	.set_ramp_delay		= s2m_set_ramp_delay,
};
#if 0
static struct regulator_ops s2mps28_bb_ops = {
	.list_voltage		= regulator_list_voltage_linear,
	.map_voltage		= regulator_map_voltage_linear,
	.is_enabled		= s2m_is_enabled_regmap,
	.enable			= s2m_enable,
	.disable		= s2m_disable_regmap,
	.get_voltage_sel	= s2m_get_voltage_sel_regmap,
	.set_voltage_sel	= s2m_set_voltage_sel_regmap,
	.set_voltage_time_sel	= s2m_set_voltage_time_sel,
	.set_mode		= s2m_set_mode,
};
#endif

#define _BUCK(macro)		S2MPS28_BUCK##macro
#define _buck_ops(num)		s2mps28_buck_ops##num
#define _LDO(macro)		S2MPS28_LDO##macro
#define _ldo_ops(num)		s2mps28_ldo_ops##num
#define _BB(macro)              S2MPS28_BB##macro
#define _bb_ops(num)  		s2mps28_bb_ops##num

#define _REG(ctrl)		S2MPS28_PM1##ctrl
#define _TIME(macro)		S2MPS28_ENABLE_TIME##macro

#define _LDO_MIN(group)		S2MPS28_LDO_MIN##group
#define _LDO_STEP(group)	S2MPS28_LDO_STEP##group
#define _LDO_MASK(num)		S2MPS28_LDO_VSEL_MASK##num
#define _BUCK_MIN(group)	S2MPS28_BUCK_MIN##group
#define _BUCK_STEP(group)	S2MPS28_BUCK_STEP##group
#define _BB_MIN(group)		S2MPS28_BB_MIN##group
#define _BB_STEP(group)		S2MPS28_BB_STEP##group

#define BUCK_DESC(_name, _id, _ops, g, v, e, t)	{		\
	.name		= _name,				\
	.id		= _id,					\
	.ops		= _ops,					\
	.type		= REGULATOR_VOLTAGE,			\
	.owner		= THIS_MODULE,				\
	.min_uV		= _BUCK_MIN(g),				\
	.uV_step	= _BUCK_STEP(g),			\
	.n_voltages	= S2MPS28_BUCK_N_VOLTAGES,		\
	.vsel_reg	= v,					\
	.vsel_mask	= S2MPS28_BUCK_VSEL_MASK,		\
	.enable_reg	= e,					\
	.enable_mask	= S2MPS28_ENABLE_MASK,			\
	.enable_time	= t,					\
	.of_map_mode	= s2mps28_of_map_mode			\
}

#define LDO_DESC(_name, _id, _ops, g, v, v_m, e, t)	{	\
	.name		= _name,				\
	.id		= _id,					\
	.ops		= _ops,					\
	.type		= REGULATOR_VOLTAGE,			\
	.owner		= THIS_MODULE,				\
	.min_uV		= _LDO_MIN(g),				\
	.uV_step	= _LDO_STEP(g),				\
	.n_voltages	= _LDO_MASK(v_m) + 1,			\
	.vsel_reg	= v,					\
	.vsel_mask	= _LDO_MASK(v_m),			\
	.enable_reg	= e,					\
	.enable_mask	= S2MPS28_ENABLE_MASK,			\
	.enable_time	= t,					\
	.of_map_mode	= s2mps28_of_map_mode			\
}

#define BB_DESC(_name, _id, _ops, g, v, e, t)	{		\
	.name		= _name,				\
	.id		= _id,					\
	.ops		= _ops,					\
	.type		= REGULATOR_VOLTAGE,			\
	.owner		= THIS_MODULE,				\
	.min_uV		= _BB_MIN(g),				\
	.uV_step	= _BB_STEP(g),				\
	.n_voltages	= S2MPS28_BB_N_VOLTAGES,		\
	.vsel_reg	= v,					\
	.vsel_mask	= S2MPS28_BB_VSEL_MASK,			\
	.enable_reg	= e,					\
	.enable_mask	= S2MPS28_ENABLE_MASK,			\
	.enable_time	= t,					\
	.of_map_mode	= s2mps28_of_map_mode			\
}

static struct regulator_desc regulators[S2MPS28_REG_MAX] = {
	/* name, id, ops, group, vsel_reg, vsel_mask, enable_reg, ramp_delay */
	/* BUCK 1S ~ 5S */
	BUCK_DESC("BUCK1", _BUCK(1), &_buck_ops(), 1, _REG(_BUCK1_OUT2), _REG(_BUCK1_CTRL), _TIME(_BUCK)),
	BUCK_DESC("BUCK2", _BUCK(2), &_buck_ops(), 1, _REG(_BUCK2_OUT2), _REG(_BUCK2_CTRL), _TIME(_BUCK)),
	BUCK_DESC("BUCK3", _BUCK(3), &_buck_ops(), 1, _REG(_BUCK3_OUT1), _REG(_BUCK3_CTRL), _TIME(_BUCK)),
	BUCK_DESC("BUCK4", _BUCK(4), &_buck_ops(), 1, _REG(_BUCK4_OUT1), _REG(_BUCK4_CTRL), _TIME(_BUCK)),
	BUCK_DESC("BUCK5", _BUCK(5), &_buck_ops(), 1, _REG(_BUCK5_OUT1), _REG(_BUCK5_CTRL), _TIME(_BUCK)),
	BUCK_DESC("BUCK_SR1", _BUCK(_SR1), &_buck_ops(), 2, _REG(_BUCK_SR1_OUT2), _REG(_BUCK_SR1_CTRL), _TIME(_BUCK_SR)),
	/* LDO 1 ~ 5 */
	LDO_DESC("LDO1", _LDO(1), &_ldo_ops(), 1, _REG(_LDO1_OUT1), 1, _REG(_LDO1_CTRL), _TIME(_LDO)),
	LDO_DESC("LDO2", _LDO(2), &_ldo_ops(), 1, _REG(_LDO2_OUT1), 1, _REG(_LDO2_CTRL), _TIME(_LDO)),
	//LDO_DESC("LDO3", _LDO(3), &_ldo_ops(), 1, _REG(_LDO3_OUT1), 1, _REG(_LDO3_CTRL), _TIME(_LDO)),
	LDO_DESC("LDO4", _LDO(4), &_ldo_ops(), 1, _REG(_LDO4_OUT1), 1, _REG(_LDO4_CTRL), _TIME(_LDO)),
	LDO_DESC("LDO5", _LDO(5), &_ldo_ops(), 1, _REG(_LDO5_OUT1), 1, _REG(_LDO5_CTRL), _TIME(_LDO)),
};


#if IS_ENABLED(CONFIG_OF)
static int s2mps28_pmic_dt_parse_pdata(struct s2mps28_dev *iodev,
				       struct s2mps28_platform_data *pdata)
{
	struct device_node *pmic_np, *regulators_np, *reg_np;
	struct s2mps28_regulator_data *rdata;
	uint32_t i, val;
	int ret;

	pmic_np = iodev->dev->of_node;
	if (!pmic_np) {
		dev_err(iodev->dev, "could not find pmic sub4-node\n");
		return -ENODEV;
	}
#if IS_ENABLED(CONFIG_EXYNOS_ACPM)
	acpm_mfd_node = pmic_np;
#endif
	/* wtsr_en */
	pdata->wtsr_en = 0;
	ret = of_property_read_u32(pmic_np, "wtsr_en", &val);
	if (ret < 0)
		pr_info("%s: fail to read wtsr_en\n", __func__);
	pdata->wtsr_en = val;

	regulators_np = of_find_node_by_name(pmic_np, "regulators");
	if (!regulators_np) {
		dev_err(iodev->dev, "could not find regulators sub4-node\n");
		return -EINVAL;
	}

	/* count the number of regulators to be supported in pmic */
	pdata->num_regulators = 0;
	for_each_child_of_node(regulators_np, reg_np) {
		pdata->num_regulators++;
	}

	rdata = devm_kzalloc(iodev->dev, sizeof(*rdata) * pdata->num_regulators, GFP_KERNEL);
	if (!rdata)
		return -ENOMEM;

	pdata->regulators = rdata;
	pdata->num_rdata = 0;
	for_each_child_of_node(regulators_np, reg_np) {
		for (i = 0; i < ARRAY_SIZE(regulators); i++)
			if (!of_node_cmp(reg_np->name, regulators[i].name))
				break;

		if (i == ARRAY_SIZE(regulators)) {
			dev_warn(iodev->dev,
				 "[PMIC] %s: don't know how to configure regulator %s\n",
				 __func__, reg_np->name);
			continue;
		}

		rdata->id = i;
		rdata->initdata = of_get_regulator_init_data(iodev->dev, reg_np, &regulators[i]);
		rdata->reg_node = reg_np;
		rdata++;
		pdata->num_rdata++;
	}

	return 0;
}
#else
static int s2mps28_pmic_dt_parse_pdata(struct s2mps28_pmic_dev *iodev,
				       struct s2mps28_platform_data *pdata)
{
	return 0;
}
#endif /* CONFIG_OF */

#if IS_ENABLED(CONFIG_EXYNOS_AFM) || IS_ENABLED(CONFIG_NPU_AFM)
int sub4_pmic_read_reg(struct i2c_client *i2c, uint8_t reg, uint8_t *val)
{
	return (i2c) ? s2mps28_4_read_reg(i2c, reg, val) : -ENODEV;
}
EXPORT_SYMBOL_GPL(sub4_pmic_read_reg);

int sub4_pmic_update_reg(struct i2c_client *i2c, u8 reg, u8 val, u8 mask)
{
	return (i2c) ? s2mps28_4_update_reg(i2c, reg, val, mask) : -ENODEV;
}
EXPORT_SYMBOL_GPL(sub4_pmic_update_reg);

int sub4_pmic_get_i2c(struct i2c_client **i2c)
{
	if (!s2mps28_static_info)
		return -ENODEV;

	*i2c = s2mps28_static_info->i2c;

	return 0;
}
EXPORT_SYMBOL_GPL(sub4_pmic_get_i2c);
#endif

struct s2mps28_oi_data {
	uint8_t reg;
	uint8_t val;
};

#define DECLARE_OI(_reg, _val) { .reg = (_reg), .val = (_val) }
static const struct s2mps28_oi_data s2mps28_oi[] = {
	/* BUCK 1~5, SR1S, OCL,OI function enable, OI power down,OVP disable */
	/* OI detection time window : 300us, OI comp. output count : 50 times */
	DECLARE_OI(S2MPS28_PM1_BUCK1_OCP, 0xCC),
	DECLARE_OI(S2MPS28_PM1_BUCK2_OCP, 0xCC),
	DECLARE_OI(S2MPS28_PM1_BUCK3_OCP, 0xCC),
	DECLARE_OI(S2MPS28_PM1_BUCK4_OCP, 0xCC),
	DECLARE_OI(S2MPS28_PM1_BUCK5_OCP, 0xCC),
	DECLARE_OI(S2MPS28_PM1_BUCK_SR1_OCP, 0xCC),
};

static int s2mps28_oi_function(struct s2mps28_dev *iodev)
{
	struct i2c_client *i2c = iodev->pm1;
	uint32_t i;
	uint8_t val;
	int ret, cnt = 0;
	char buf[1024] = {0, };

	for (i = 0; i < ARRAY_SIZE(s2mps28_oi); i++) {
		ret = s2mps28_4_write_reg(i2c, s2mps28_oi[i].reg, s2mps28_oi[i].val);
		if (ret) {
			pr_err("%s: failed to write register\n", __func__);
			goto err;
		}
	}

	for (i = 0; i < ARRAY_SIZE(s2mps28_oi); i++) {
		ret = s2mps28_4_read_reg(i2c, s2mps28_oi[i].reg, &val);
		if (ret)
			goto err;

		cnt += snprintf(buf + cnt, sizeof(buf) - 1, "0x%x[0x%02hhx], ", s2mps28_oi[i].reg, val);
	}
	pr_info("%s: %s\n", __func__, buf);

	return 0;
err:
	return -1;
}

static void s2mps28_wtsr_enable(struct s2mps28_info *s2mps28,
				struct s2mps28_platform_data *pdata)
{
	int ret;

	pr_info("[PMIC] %s: WTSR (%s)\n", __func__,
		pdata->wtsr_en ? "enable" : "disable");

	ret = s2mps28_4_update_reg(s2mps28->i2c, S2MPS28_PM1_CFG_PM,
				 S2MPS28_WTSREN_MASK, S2MPS28_WTSREN_MASK);

	if (ret < 0)
		pr_info("%s: fail to update WTSR reg(%d)\n", __func__, ret);

	s2mps28->wtsr_en = pdata->wtsr_en;
}

static void s2mps28_wtsr_disable(struct s2mps28_info *s2mps28)
{
	int ret;

	pr_info("[PMIC] %s: disable WTSR\n", __func__);

	ret = s2mps28_4_update_reg(s2mps28->i2c, S2MPS28_PM1_CFG_PM,
				 0x00, S2MPS28_WTSREN_MASK);
	if (ret < 0)
		pr_info("%s: fail to update WTSR reg(%d)\n", __func__, ret);
}

#if IS_ENABLED(CONFIG_DRV_SAMSUNG_PMIC)
static int check_base_address(uint8_t base_addr)
{
	switch (base_addr) {
	case VGPIO_ADDR:
	case COM_ADDR:
	case PM1_ADDR:
	case PM2_ADDR:
	case PM3_ADDR:
	case GPIO_ADDR:
	case EXT_ADDR:
		break;
	default:
		pr_err("%s: base address error(0x%02hhx)\n", __func__, base_addr);
		return -EINVAL;
	}

	return 0;
}

static ssize_t s2mps28_read_store(struct device *dev,
				  struct device_attribute *attr,
				  const char *buf, size_t size)
{
	struct s2mps28_info *s2mps28 = dev_get_drvdata(dev);
	struct pmic_sysfs_dev *pmic_sysfs = s2mps28->pmic_sysfs;
	uint8_t base_addr = 0, reg_addr = 0, val = 0;
	int ret;

	if (buf == NULL) {
		pr_info("%s: empty buffer\n", __func__);
		return -1;
	}

	ret = sscanf(buf, "0x%02hhx%02hhx", &base_addr, &reg_addr);
	if (ret != 2) {
		pr_err("%s: input error\n", __func__);
		return size;
	}

	ret = check_base_address(base_addr);
	if (ret < 0)
		return ret;

#if IS_ENABLED(CONFIG_EXYNOS_ACPM)
	mutex_lock(&s2mps28->iodev->i2c_lock);
	ret = exynos_acpm_read_reg(acpm_mfd_node, SUB4_ID, base_addr, reg_addr, &val);
	mutex_unlock(&s2mps28->iodev->i2c_lock);
	if (ret)
		pr_info("%s: fail to read i2c addr/data\n", __func__);
#endif
	pmic_sysfs->base_addr = base_addr;
	pmic_sysfs->read_addr = reg_addr;
	pmic_sysfs->read_val = val;

	dev_info(s2mps28->iodev->dev, "%s: reg(0x%02hhx%02hhx) data(0x%02hhx)\n",
					__func__, base_addr, reg_addr, val);

	return size;
}

static ssize_t s2mps28_read_show(struct device *dev,
				 struct device_attribute *attr,
				 char *buf)
{
	struct s2mps28_info *s2mps28 = dev_get_drvdata(dev);
	struct pmic_sysfs_dev *pmic_sysfs = s2mps28->pmic_sysfs;

	return sprintf(buf, "0x%02hhx%02hhx: 0x%02hhx\n",
			pmic_sysfs->base_addr, pmic_sysfs->read_addr, pmic_sysfs->read_val);
}

static ssize_t s2mps28_write_store(struct device *dev,
				   struct device_attribute *attr,
				   const char *buf, size_t size)
{
	struct s2mps28_info *s2mps28 = dev_get_drvdata(dev);
	struct pmic_sysfs_dev *pmic_sysfs = s2mps28->pmic_sysfs;
	uint8_t base_addr = 0, reg_addr = 0, val = 0;
	int ret;

	if (buf == NULL) {
		pr_info("%s: empty buffer\n", __func__);
		return size;
	}

	ret = sscanf(buf, "0x%02hhx%02hhx 0x%02hhx", &base_addr, &reg_addr, &val);
	if (ret != 3) {
		pr_err("%s: input error\n", __func__);
		return size;
	}

	ret = check_base_address(base_addr);
	if (ret < 0)
		return ret;

#if IS_ENABLED(CONFIG_EXYNOS_ACPM)
	mutex_lock(&s2mps28->iodev->i2c_lock);
	ret = exynos_acpm_write_reg(acpm_mfd_node, SUB4_ID, base_addr, reg_addr, val);
	mutex_unlock(&s2mps28->iodev->i2c_lock);
	if (ret)
		pr_info("%s: fail to write i2c addr/data\n", __func__);
#endif
	pmic_sysfs->base_addr = base_addr;
	pmic_sysfs->read_addr = reg_addr;
	pmic_sysfs->read_val = val;

	dev_info(s2mps28->iodev->dev, "%s: reg(0x%02hhx%02hhx) data(0x%02hhx)\n",
		       			__func__, base_addr, reg_addr, val);

	return size;
}

static ssize_t s2mps28_write_show(struct device *dev,
				  struct device_attribute *attr,
				  char *buf)
{
	struct s2mps28_info *s2mps28 = dev_get_drvdata(dev);
	struct pmic_sysfs_dev *pmic_sysfs = s2mps28->pmic_sysfs;

	return sprintf(buf, "0x%02hhx%02hhx: 0x%02hhx\n",
			pmic_sysfs->base_addr, pmic_sysfs->read_addr, pmic_sysfs->read_val);
}

static struct pmic_device_attribute regulator_attr[] = {
	PMIC_ATTR(write, S_IRUGO | S_IWUSR, s2mps28_write_show, s2mps28_write_store),
	PMIC_ATTR(read, S_IRUGO | S_IWUSR, s2mps28_read_show, s2mps28_read_store),
};

static int s2mps28_create_sysfs(struct s2mps28_info *s2mps28)
{
	struct device *dev = s2mps28->iodev->dev;
	struct device *sysfs_dev = NULL;
	char device_name[32] = {0,};
	int err = -ENODEV, i = 0;

	pr_info("%s()\n", __func__);

	/* Dynamic allocation for device name */
	snprintf(device_name, sizeof(device_name) - 1, "%s@%s",
		 dev_driver_string(dev), dev_name(dev));

	s2mps28->pmic_sysfs = devm_kzalloc(dev, sizeof(struct pmic_sysfs_dev), GFP_KERNEL);
	s2mps28->pmic_sysfs->dev = pmic_device_create(s2mps28, device_name);
	sysfs_dev = s2mps28->pmic_sysfs->dev;

	/* Create sysfs entries */
	for (i = 0; i < ARRAY_SIZE(regulator_attr); i++) {
		err = device_create_file(sysfs_dev, &regulator_attr[i].dev_attr);
		if (err)
			goto remove_pmic_device;
	}

	return 0;

remove_pmic_device:
	for (i--; i >= 0; i--)
		device_remove_file(sysfs_dev, &regulator_attr[i].dev_attr);
	pmic_device_destroy(sysfs_dev->devt);

	return -EINVAL;
}
#endif

static int s2mps28_pmic_probe(struct platform_device *pdev)
{
	struct s2mps28_dev *iodev = dev_get_drvdata(pdev->dev.parent);
	struct s2mps28_platform_data *pdata = iodev->pdata;
	struct regulator_config config = { };
	struct s2mps28_info *s2mps28;
	int ret;
	uint32_t i;

	pr_info("[SUB4_PMIC] %s: start\n", __func__);

	if (iodev->dev->of_node) {
		ret = s2mps28_pmic_dt_parse_pdata(iodev, pdata);
		if (ret)
			goto err_pdata;
	}

	if (!pdata) {
		dev_err(pdev->dev.parent, "Platform data not supplied\n");
		return -ENODEV;
	}

	s2mps28 = devm_kzalloc(&pdev->dev, sizeof(struct s2mps28_info), GFP_KERNEL);
	if (!s2mps28)
		return -ENOMEM;

	s2mps28->iodev = iodev;
	s2mps28->i2c = iodev->pm1;

	mutex_init(&s2mps28->lock);

	s2mps28_static_info = s2mps28;

	platform_set_drvdata(pdev, s2mps28);

	config.dev = &pdev->dev;
	config.driver_data = s2mps28;
	for (i = 0; i < pdata->num_rdata; i++) {
		int id = pdata->regulators[i].id;
		config.init_data = pdata->regulators[i].initdata;
		config.of_node = pdata->regulators[i].reg_node;
		s2mps28->opmode[id] = regulators[id].enable_mask;
		s2mps28->rdev[i] = devm_regulator_register(&pdev->dev, &regulators[id], &config);
		if (IS_ERR(s2mps28->rdev[i])) {
			ret = PTR_ERR(s2mps28->rdev[i]);
			dev_err(&pdev->dev, "[PMIC] regulator init failed for %s(%d)\n",
				regulators[i].name, i);
			goto err_s2mps28_data;
		}
	}

	s2mps28->num_regulators = pdata->num_rdata;

	if (pdata->wtsr_en)
		s2mps28_wtsr_enable(s2mps28, pdata);

	ret = s2mps28_oi_function(iodev);
	if (ret < 0)
		pr_err("%s: s2mps28_oi_function fail\n", __func__);

	//exynos_reboot_register_pmic_ops(NULL, s2mps28_power_off_wa, NULL, NULL);

#if IS_ENABLED(CONFIG_DRV_SAMSUNG_PMIC)
	ret = s2mps28_create_sysfs(s2mps28);
	if (ret < 0) {
		pr_err("%s: s2mps28_create_sysfs fail\n", __func__);
		goto err_s2mps28_data;
	}
#endif
	pr_info("[SUB4_PMIC] %s: end\n", __func__);

	return 0;

err_s2mps28_data:
	mutex_destroy(&s2mps28->lock);
err_pdata:
	return ret;
}

#if IS_ENABLED(CONFIG_DRV_SAMSUNG_PMIC)
static void s2mps28_remove_sysfs_entries(struct device *sysfs_dev)
{
	uint32_t i = 0;

	for (i = 0; i < ARRAY_SIZE(regulator_attr); i++)
		device_remove_file(sysfs_dev, &regulator_attr[i].dev_attr);
	pmic_device_destroy(sysfs_dev->devt);
}
#endif

static int s2mps28_pmic_remove(struct platform_device *pdev)
{
	struct s2mps28_info *s2mps28 = platform_get_drvdata(pdev);

#if IS_ENABLED(CONFIG_DRV_SAMSUNG_PMIC)
	s2mps28_remove_sysfs_entries(s2mps28->pmic_sysfs->dev);
#endif
	mutex_destroy(&s2mps28->lock);

	return 0;
}

static void s2mps28_pmic_shutdown(struct platform_device *pdev)
{
	struct s2mps28_info *s2mps28 = platform_get_drvdata(pdev);

	/* disable WTSR */
	if (s2mps28->wtsr_en)
		s2mps28_wtsr_disable(s2mps28);

	pr_info("%s\n", __func__);
}

#if IS_ENABLED(CONFIG_PM)
static int s2mps28_pmic_suspend(struct device *dev)
{
	pr_info("%s\n", __func__);

	return 0;
}

static int s2mps28_pmic_resume(struct device *dev)
{
	pr_info("%s\n", __func__);

	return 0;
}
#else
#define s2mps28_pmic_suspend	NULL
#define s2mps28_pmic_resume	NULL
#endif /* CONFIG_PM */

static SIMPLE_DEV_PM_OPS(s2mps28_pmic_pm, s2mps28_pmic_suspend, s2mps28_pmic_resume);

static const struct platform_device_id s2mps28_pmic_id[] = {
	{ "s2mps28-4-regulator", 0},
	{ },
};
MODULE_DEVICE_TABLE(platform, s2mps28_pmic_id);

static struct platform_driver s2mps28_pmic_driver = {
	.driver = {
		.name = "s2mps28-4-regulator",
		.owner = THIS_MODULE,
#if IS_ENABLED(CONFIG_PM)
		.pm = &s2mps28_pmic_pm,
#endif
		.suppress_bind_attrs = true,
	},
	.probe = s2mps28_pmic_probe,
	.remove = s2mps28_pmic_remove,
	.shutdown = s2mps28_pmic_shutdown,
	.id_table = s2mps28_pmic_id,
};

module_platform_driver(s2mps28_pmic_driver);

/* Module information */
MODULE_AUTHOR("Samsung Electronics");
MODULE_DESCRIPTION("SAMSUNG S2MPS28 Regulator Driver");
MODULE_LICENSE("GPL");
