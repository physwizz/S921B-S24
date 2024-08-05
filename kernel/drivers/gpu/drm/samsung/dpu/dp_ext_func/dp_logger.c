/*
 * Copyright (c) 2016 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com
 *
 * DP logger
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/types.h>
#include <linux/stat.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/ktime.h>
#include <linux/uaccess.h>
#include <linux/sched/clock.h>
#include <linux/device.h>
#include <linux/spinlock.h>
#include <linux/delay.h>

#if IS_ENABLED(CONFIG_DRV_SAMSUNG)
#include <linux/sec_class.h>
#endif

#include "dp_logger.h"

#define BUF_SIZE	SZ_64K
#define PROC_FILE_NAME	"dplog"
#define LOG_PREFIX	"Displayport"
#define PRINT_DATE_FREQ	20

static char dp_log_buf[BUF_SIZE];
static unsigned int g_curpos;
static int is_dp_logger_init;
static int is_buf_full;
static int log_max_count = -1;
static unsigned int log_count = PRINT_DATE_FREQ;

static int uevent_enable;

void dp_logger_print_date_time(void)
{
	char tmp[64] = {0x0, };
	struct timespec64 ts;
	struct tm tm;
	unsigned long sec;

	ktime_get_real_ts64(&ts);
	sec = ts.tv_sec - (sys_tz.tz_minuteswest * 60);
	time64_to_tm(sec, 0, &tm);
	snprintf(tmp, sizeof(tmp), "@%02d-%02d %02d:%02d:%02d.%03lu", tm.tm_mon + 1, tm.tm_mday,
			tm.tm_hour, tm.tm_min, tm.tm_sec, ts.tv_nsec / 1000000);

	dp_logger_print("%s\n", tmp);
}

/* set max log count, if count is -1, no limit */
void dp_logger_set_max_count(int count)
{
	log_max_count = count;

	dp_logger_print_date_time();
	log_count = PRINT_DATE_FREQ;
}
EXPORT_SYMBOL(dp_logger_set_max_count);

void dp_logger_print(const char *fmt, ...)
{
	int len;
	va_list args;
	char buf[MAX_DPLOG_STR_LEN] = {0, };
	u64 time;
	unsigned long nsec;
	unsigned int curpos;

	if (!is_dp_logger_init)
		return;

	if (!uevent_enable && log_max_count == 0)
		return;
	else if (log_max_count > 0)
		log_max_count--;

	if (--log_count == 0) {
		dp_logger_print_date_time();
		log_count = PRINT_DATE_FREQ;
	}

	time = local_clock();
	nsec = do_div(time, 1000000000);

	len = snprintf(buf, sizeof(buf), "[%5lu.%06ld] ", (unsigned long)time, nsec / 1000);

	va_start(args, fmt);
	len += vsnprintf(buf + len, MAX_DPLOG_STR_LEN - len, fmt, args);
	va_end(args);

	if (len > MAX_DPLOG_STR_LEN)
		len = MAX_DPLOG_STR_LEN;

	curpos = g_curpos;
	if (curpos + len >= BUF_SIZE) {
		g_curpos = curpos = 0;
		is_buf_full = 1;
	}
	memcpy(dp_log_buf + curpos, buf, len);
	g_curpos += len;

	dp_print_log_to_uevent(buf, len, SEND_OPT_MESSAGE);
}
EXPORT_SYMBOL(dp_logger_print);

void dp_logger_hex_dump(void *buf, void *pref, size_t size)
{
	uint8_t *ptr = buf;
	size_t i;
	char tmp[128] = {0x0, };
	char *ptmp = tmp;
	int len;

	if (!is_dp_logger_init)
		return;

	if (!uevent_enable && log_max_count == 0)
		return;
	else if (log_max_count > 0)
		log_max_count--;

	for (i = 0; i < size; i++) {
		len = snprintf(ptmp, 4, "%02x ", *ptr++);
		ptmp = ptmp + len;
		if (((i+1)%16) == 0) {
			dp_logger_print("%s%s\n", pref, tmp);
			ptmp = tmp;
		}
	}

	if (i % 16) {
		len = ptmp - tmp;
		tmp[len] = 0x0;
		dp_logger_print("%s%s\n", pref, tmp);
	}
}
EXPORT_SYMBOL(dp_logger_hex_dump);

static ssize_t dp_logger_read(struct file *file, char __user *buf, size_t len, loff_t *offset)
{
	loff_t pos = *offset;
	ssize_t count;
	size_t size;
	volatile unsigned int curpos = g_curpos;

	if (is_buf_full || BUF_SIZE <= curpos)
		size = BUF_SIZE;
	else
		size = (size_t)curpos;

	if (pos >= size)
		return 0;

	count = min(len, size);

	if ((pos + count) > size)
		count = size - pos;

	if (copy_to_user(buf, dp_log_buf + pos, count))
		return -EFAULT;

	*offset += count;

	return count;
}

static const struct proc_ops dp_logger_ops = {
	.proc_read = dp_logger_read,
	.proc_lseek = default_llseek,
};

#if IS_ENABLED(CONFIG_DRV_SAMSUNG)
static struct device *dp_uevent_device;
#endif
static struct work_struct uevent_work;
static struct list_head list_uevent;
DEFINE_SPINLOCK(uevent_lock);
static unsigned long uevent_flags;

struct dp_uevent {
	struct list_head list;
	char str[MAX_DPLOG_STR_LEN];
	int size;
	int option;
};

void dp_enable_uevent(int enable)
{
#ifdef CONFIG_SEC_DISPLAYPORT_DBG
	uevent_enable = enable;
#else
	uevent_enable = 0;
#endif
}
EXPORT_SYMBOL(dp_enable_uevent);

#define MIN(x, y) (x > y ? y : x)
#define MAX_HEADER	32
#define PAYLOAD_TAG	"P1="

void dp_print_log_to_uevent(char *str, int size, int option)
{
#ifdef CONFIG_SEC_DISPLAYPORT_DBG
	struct dp_uevent *uevent_data;
	int payload_size = MIN(MAX_DPLOG_STR_LEN, size + strlen(PAYLOAD_TAG));

	if (!uevent_enable || !is_dp_logger_init)
		return;

	uevent_data = kzalloc(sizeof(struct dp_uevent), GFP_KERNEL);
	if (!uevent_data)
		return;

	spin_lock_irqsave(&uevent_lock, uevent_flags);
	strncpy(uevent_data->str, str, payload_size);
	uevent_data->size = payload_size;
	uevent_data->option = option;

	list_add_tail(&uevent_data->list, &list_uevent);
	spin_unlock_irqrestore(&uevent_lock, uevent_flags);

	schedule_work(&uevent_work);
#endif
}
EXPORT_SYMBOL(dp_print_log_to_uevent);

void dp_logger_print_to_toast(const char *fmt, ...)
{
	int len = 0;
	va_list args;
	char buf[MAX_DPLOG_STR_LEN] = {0, };

	if (!uevent_enable || !is_dp_logger_init)
		return;

	va_start(args, fmt);
	len = vsnprintf(buf, MAX_DPLOG_STR_LEN - len, fmt, args);
	va_end(args);

	dp_print_log_to_uevent(buf, len, SEND_OPT_TOAST);
}
EXPORT_SYMBOL(dp_logger_print_to_toast);

static void dp_send_uevent(struct dp_uevent *data)
{
#ifdef CONFIG_SEC_DISPLAYPORT_DBG
#if IS_ENABLED(CONFIG_DRV_SAMSUNG)
	char dp_dbg_header[MAX_HEADER] = {0, };
	char dp_dbg_payload[MAX_DPLOG_STR_LEN] = {0, };
	char *envp[] = {dp_dbg_header, dp_dbg_payload, NULL};

	if (!uevent_enable || !is_dp_logger_init)
		return;

	if (IS_ERR(dp_uevent_device))
		return;

	if (data->option == SEND_OPT_TOAST)
		snprintf(dp_dbg_header, MAX_HEADER, "HEADER=%s", "opt_toast");
	else if (data->option == SEND_OPT_MESSAGE)
		snprintf(dp_dbg_header, MAX_HEADER, "HEADER=%s", "opt_message");
	snprintf(dp_dbg_payload, data->size, "%s%s", PAYLOAD_TAG, data->str);
	kobject_uevent_env(&dp_uevent_device->kobj, KOBJ_CHANGE, envp);
#endif
#endif
}

static void dp_send_uevent_work(struct work_struct *work)
{
	struct dp_uevent *pdata;
	struct dp_uevent data;

	while (!list_empty(&list_uevent)) {
		spin_lock_irqsave(&uevent_lock, uevent_flags);
		pdata = list_first_entry(&list_uevent, typeof(*pdata), list);
		memcpy(&data, pdata, sizeof(data));
		list_del(&pdata->list);
		kfree(pdata);
		spin_unlock_irqrestore(&uevent_lock, uevent_flags);

		dp_send_uevent(&data);
		usleep_range(1000, 1001);
	}
}

int dp_logger_init(void)
{
	struct proc_dir_entry *entry;

	if (is_dp_logger_init)
		return 0;

	entry = proc_create(PROC_FILE_NAME, 0444, NULL, &dp_logger_ops);
	if (!entry) {
		pr_err("%s: failed to create proc entry\n", __func__);
		return 0;
	}

	proc_set_size(entry, BUF_SIZE);

#if IS_ENABLED(CONFIG_DRV_SAMSUNG)
	/* DEVPATH=/devices/virtual/sec/secdp */
	dp_uevent_device = sec_device_create(NULL, "secdp");
#endif
	INIT_WORK(&uevent_work, dp_send_uevent_work);
	INIT_LIST_HEAD(&list_uevent);

	is_dp_logger_init = 1;
	dp_logger_print("dp logger init ok\n");

	return 0;
}
EXPORT_SYMBOL(dp_logger_init);


MODULE_DESCRIPTION("SEC Displaport logger");
MODULE_LICENSE("GPL");

