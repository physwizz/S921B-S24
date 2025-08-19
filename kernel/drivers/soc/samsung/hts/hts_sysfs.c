/*
 * Copyright (c) 2023 Samsung Electronics Co., Ltd.
 *              http://www.samsung.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include "hts_sysfs.h"
#include "hts_vh.h"
#include "hts_var.h"
#include "hts_ext.h"

#include <linux/types.h>
#include <linux/stddef.h>
#include <linux/pid.h>
#include <linux/slab.h>
#include <linux/sched/task.h>

#include <linux/plist.h>

static struct hts_config_control __percpu *hts_default_value;

struct hts_config_control *hts_core_default(int cpu)
{
	return per_cpu_ptr(hts_default_value, cpu);
}

static void read_ectlr(void *val)
{
	*(unsigned long *)val = read_sysreg_s(SYS_ECTLR);
}

static void write_ectlr(void *val)
{
	write_sysreg_s(*(unsigned long *)val, SYS_ECTLR);
}

static void read_ectlr2(void *val)
{
	*(unsigned long *)val = read_sysreg_s(SYS_ECTLR2);
}

static void write_ectlr2(void *val)
{
	write_sysreg_s(*(unsigned long *)val, SYS_ECTLR2);
}

static void read_total(void *val)
{
	struct hts_config_control *handle = (struct hts_config_control *)val;

	if (handle->ctrl[REG_CTRL1].enable)
		read_ectlr(&handle->ctrl[REG_CTRL1].ext_ctrl[0]);
	if (handle->ctrl[REG_CTRL2].enable)
		read_ectlr2(&handle->ctrl[REG_CTRL2].ext_ctrl[0]);
}

static void write_total(void *val)
{
	struct hts_config_control *handle = (struct hts_config_control *)val;

	if (handle->ctrl[REG_CTRL1].enable)
		write_ectlr(&handle->ctrl[REG_CTRL1].ext_ctrl[0]);
	if (handle->ctrl[REG_CTRL2].enable)
		write_ectlr2(&handle->ctrl[REG_CTRL2].ext_ctrl[0]);
}

void hts_core_reset_default(void)
{
	int cpu, ret;
	struct hts_config_control *handle;

	for_each_online_cpu(cpu) {
		handle = per_cpu_ptr(hts_default_value, cpu);

		ret = smp_call_function_single(cpu, write_total, handle, 1);
		if (ret)
			pr_err("HTS : Couldn't reset core default status! - (%d)", cpu);
	}
}

static int hts_backup_default(struct platform_device *pdev)
{
	int cpu, ret;
	int reg_cnt = 0;
	struct hts_config_control *handle;
	struct hts_data *data = platform_get_drvdata(pdev);

	hts_default_value = alloc_percpu(struct hts_config_control);

	for_each_online_cpu(cpu) {
		handle = per_cpu_ptr(hts_default_value, cpu);
		for (reg_cnt = 0; reg_cnt < data->nr_reg; reg_cnt++)
			handle->ctrl[reg_cnt].enable = cpumask_test_cpu(cpu, &data->hts_reg[reg_cnt].support_cpu);

		ret = smp_call_function_single(cpu, read_total, handle, 1);
		if (ret)
			return ret;
	}

	return 0;
}

static ssize_t extended_control_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	unsigned long value = 0;
	int cpu, ret, written = 0;
	struct hts_data *data = dev_get_drvdata(dev);

	for_each_online_cpu(cpu) {
		value = 0;
		if (cpumask_test_cpu(cpu, &data->hts_reg[REG_CTRL1].support_cpu)) {
			ret = smp_call_function_single(cpu, read_ectlr, &value, 1);
			if (ret)
				return ret;
		}
		written += scnprintf(buf + written, PAGE_SIZE - written, "CPU %d : %lx\n", cpu, value);
	}

	return written;
}

static ssize_t extended_control_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct task_struct *target_task;
	struct hts_config_control *handle;
	int pid, idx, maskCount;
	unsigned long value;

	if (sscanf(buf, "%d %d %lx", &pid, &idx, &value) != 3)
		return -EINVAL;

	maskCount = hts_get_mask_count();

	if (idx < 0 ||
		maskCount <= idx)
		return -EINVAL;

	target_task = get_pid_task(find_vpid(pid), PIDTYPE_PID);
	if (target_task == NULL)
		return -ENOENT;

	handle = hts_get_or_alloc_task_config(target_task);
	handle->ctrl[REG_CTRL1].ext_ctrl[idx] = value;
	put_task_struct(target_task);

	return count;
}
DEVICE_ATTR_RW(extended_control);

static ssize_t extended_control2_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	unsigned long value = ULONG_MAX;
	int cpu, ret, written = 0;
	struct hts_data *data = dev_get_drvdata(dev);

	for_each_online_cpu(cpu) {
		value = ULONG_MAX;
		if (cpumask_test_cpu(cpu, &data->hts_reg[REG_CTRL2].support_cpu)) {
			ret = smp_call_function_single(cpu, read_ectlr2, &value, 1);
			if (ret)
				return ret;
		}
		written += scnprintf(buf + written, PAGE_SIZE - written, "CPU %d : %lx\n", cpu, value);
	}
	return written;
}

static ssize_t extended_control2_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct task_struct *target_task;
	struct hts_config_control *handle;
	int pid, idx, maskCount;
	unsigned long value;

	if (sscanf(buf, "%d %d %lx", &pid, &idx, &value) != 3)
		return -EINVAL;

	maskCount = hts_get_mask_count();

	if (idx < 0 ||
		maskCount <= idx)
		return -EINVAL;

	target_task = get_pid_task(find_vpid(pid), PIDTYPE_PID);
	if (target_task == NULL)
		return -ENOENT;

	handle = hts_get_or_alloc_task_config(target_task);
	handle->ctrl[REG_CTRL2].ext_ctrl[idx] = value;
	put_task_struct(target_task);

	return count;
}
DEVICE_ATTR_RW(extended_control2);

static struct attribute *hts_knob_attrs[] = {
	&dev_attr_extended_control.attr,
	&dev_attr_extended_control2.attr,
	NULL,
};

static struct attribute_group hts_knob_group = {
	.name = "knob",
	.attrs = hts_knob_attrs,
};

#if IS_ENABLED(CONFIG_HTS_DEBUG)
static ssize_t selector_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	int written = 0;
	struct hts_data *data = dev_get_drvdata(dev);
	struct hts_debug_data *debug_data = NULL;

	if (data == NULL)
		return -ENOENT;

	debug_data = &data->debug_data;

	written += scnprintf(buf + written, PAGE_SIZE - written,
			"pid = %d, idx = %d", debug_data->pid, debug_data->idx);

	return written;
}

static ssize_t selector_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	int pid, idx;
	struct hts_data *data = dev_get_drvdata(dev);
	struct hts_debug_data *debug_data = NULL;

	if (sscanf(buf, "%d %d", &pid, &idx) != 2)
		return -EINVAL;

	if (data == NULL)
		return -ENOENT;

	if (idx < 0 ||
			hts_get_mask_count() <= idx)
		return -EINVAL;

	debug_data = &data->debug_data;
	debug_data->pid = pid;
	debug_data->idx = idx;

	return count;
}
DEVICE_ATTR_RW(selector);

static ssize_t extended_data_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct task_struct *target_task;
	struct hts_config_control *handle;
	int written = 0;
	struct hts_data *data = dev_get_drvdata(dev);
	struct hts_debug_data *debug_data = NULL;

	if (data == NULL)
		return -ENOENT;

	debug_data = &data->debug_data;

	target_task = get_pid_task(find_vpid(debug_data->pid), PIDTYPE_PID);
	if (target_task == NULL)
		return -ENOENT;

	handle = (struct hts_config_control *)TASK_CONFIG_CONTROL(target_task);
	if (handle == NULL) {
		handle = kzalloc(sizeof(struct hts_config_control), GFP_KERNEL);
		TASK_CONFIG_CONTROL(target_task) = (unsigned long)handle;
	}
	written += scnprintf(buf + written, PAGE_SIZE - written, "%lx\n", handle->ctrl[REG_CTRL1].ext_ctrl[debug_data->idx]);
	put_task_struct(target_task);

	return written;
}
DEVICE_ATTR_RO(extended_data);

static ssize_t extended_data2_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct task_struct *target_task;
	struct hts_config_control *handle;
	int written = 0;
	struct hts_data *data = dev_get_drvdata(dev);
	struct hts_debug_data *debug_data = NULL;

	if (data == NULL)
		return -ENOENT;

	debug_data = &data->debug_data;

	target_task = get_pid_task(find_vpid(debug_data->pid), PIDTYPE_PID);
	if (target_task == NULL)
		return -ENOENT;

	handle = (struct hts_config_control *)TASK_CONFIG_CONTROL(target_task);
	if (handle == NULL) {
		handle = kzalloc(sizeof(struct hts_config_control), GFP_KERNEL);
		TASK_CONFIG_CONTROL(target_task) = (unsigned long)handle;
	}
	written += scnprintf(buf + written, PAGE_SIZE - written, "%lx\n", handle->ctrl[REG_CTRL2].ext_ctrl[debug_data->idx]);
	put_task_struct(target_task);

	return written;
}
DEVICE_ATTR_RO(extended_data2);

static struct attribute *hts_debug_attrs[] = {
	&dev_attr_selector.attr,
	&dev_attr_extended_data.attr,
	&dev_attr_extended_data2.attr,
	NULL,
};

static struct attribute_group hts_debug_group = {
	.name = "debug",
	.attrs = hts_debug_attrs,
};
#endif

int initialize_hts_sysfs(struct platform_device *pdev)
{
	if (hts_backup_default(pdev)) {
		pr_err("HTS : Couldn't backup default");
		return -EINVAL;
	}

	if (sysfs_create_group(&pdev->dev.kobj, &hts_knob_group)) {
		pr_err("HTS : Couldn't create knob sysfs");
		return -EINVAL;
	}

#if IS_ENABLED(CONFIG_HTS_DEBUG)
	if (sysfs_create_group(&pdev->dev.kobj, &hts_debug_group)) {
		pr_err("HTS : Couldn't create debug sysfs");
		return -EINVAL;
	}
#endif

	return 0;
}

int uninitialize_hts_sysfs(struct platform_device *pdev)
{
	return 0;
}
