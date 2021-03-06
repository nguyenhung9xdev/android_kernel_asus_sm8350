// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2019-2020, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 */

#include <linux/device.h>
#include <linux/i2c.h>
#include "fts.h"

static ssize_t touch_event_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{

	return 0;
}

ssize_t aoi_set_store(struct device *dev, struct device_attribute *attr,
	const char *buf, size_t count)
{
	int ret;
	struct i2c_client *client = to_i2c_client(dev);
	struct fts_ts_info *info = i2c_get_clientdata(client);
	int left, top, right, bottom;

	ret = sscanf(buf, "%d %d %d %d", &left, &top, &right, &bottom);
	if (ret != 4)
		return -EINVAL;

	if (right > X_AXIS_MAX)
		right = X_AXIS_MAX;
	if (bottom > Y_AXIS_MAX)
		bottom = Y_AXIS_MAX;

	if (left < 0 || left > X_AXIS_MAX || right < 0 ||
		top > Y_AXIS_MAX || bottom < 0)
		return -EINVAL;

	if (left >= right || top >= bottom) {
		info->aoi_left = 0;
		info->aoi_top = 0;
		info->aoi_right = 0;
		info->aoi_bottom = 0;
		info->aoi_notify_enabled = false;
		return count;
	}

	info->aoi_left = left;
	info->aoi_top = top;
	info->aoi_right = right;
	info->aoi_bottom = bottom;

	info->aoi_notify_enabled = true;
	return count;
}

static ssize_t aoi_set_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct fts_ts_info *info = i2c_get_clientdata(client);
	size_t len = 0;

	len = scnprintf(buf + len, PAGE_SIZE,
				"%d %d %d %d",
				info->aoi_left,
				info->aoi_top,
				info->aoi_right,
				info->aoi_bottom);

	return len;
}

static ssize_t power_set_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int enable;

	if (kstrtoint(buf, 10, &enable))
		return -EINVAL;

	return count;
}

static DEVICE_ATTR_RO(touch_event);
static DEVICE_ATTR_RW(aoi_set);
static DEVICE_ATTR_WO(power_set);

static struct attribute *aoi_cmd_attributes[] = {
	&dev_attr_touch_event.attr,
	&dev_attr_aoi_set.attr,
	&dev_attr_power_set.attr,
	NULL,
};

struct attribute_group aoi_cmd_attr_group = {
	.attrs = aoi_cmd_attributes,
};

static ssize_t enable_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct fts_ts_info *info = dev_get_drvdata(dev);
	int enable;

	if (kstrtoint(buf, 10, &enable))
		return -EINVAL;

	if (!enable && info->aoi_notify_enabled) {
		info->aoi_left = 0;
		info->aoi_top = 0;
		info->aoi_right = 0;
		info->aoi_bottom = 0;
		info->aoi_notify_enabled = false;
	} else {
		info->aoi_left = 0;
		info->aoi_top = 0;
		info->aoi_right = X_AXIS_MAX;
		info->aoi_bottom = Y_AXIS_MAX;
		info->aoi_notify_enabled = true;
	}

	return count;
}

static ssize_t enable_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct fts_ts_info *info = dev_get_drvdata(dev);
	size_t len = 0;

	len = scnprintf(buf, PAGE_SIZE,
				"%d",
				info->aoi_notify_enabled);

	return len;
}

static DEVICE_ATTR_RW(enable);

static struct attribute *aoi_enable_attributes[] = {
	&dev_attr_aoi_set.attr,
	&dev_attr_enable.attr,
	NULL,
};

struct attribute_group aoi_enable_attr_group = {
	.attrs = aoi_enable_attributes,
};
