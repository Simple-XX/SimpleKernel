/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2021 YADRO
 *
 * Authors:
 *   Nikita Shubin <n.shubin@yadro.com>
 *
 * derivate: lib/utils/gpio/gpio.c
 * Authors:
 *   Anup Patel <anup.patel@wdc.com>
 */

#include <sbi/sbi_error.h>
#include <sbi_utils/i2c/i2c.h>

static SBI_LIST_HEAD(i2c_adapters_list);

struct i2c_adapter *i2c_adapter_find(int id)
{
	struct sbi_dlist *pos;

	sbi_list_for_each(pos, &(i2c_adapters_list)) {
		struct i2c_adapter *adap = to_i2c_adapter(pos);

		if (adap->id == id)
			return adap;
	}

	return NULL;
}

int i2c_adapter_add(struct i2c_adapter *ia)
{
	if (!ia)
		return SBI_EINVAL;

	if (i2c_adapter_find(ia->id))
		return SBI_EALREADY;

	sbi_list_add(&(ia->node), &(i2c_adapters_list));

	return 0;
}

void i2c_adapter_remove(struct i2c_adapter *ia)
{
	if (!ia)
		return;

	sbi_list_del(&(ia->node));
}

int i2c_adapter_write(struct i2c_adapter *ia, uint8_t addr, uint8_t reg,
		     uint8_t *buffer, int len)
{
	if (!ia)
		return SBI_EINVAL;
	if (!ia->write)
		return SBI_ENOSYS;

	return ia->write(ia, addr, reg, buffer, len);
}


int i2c_adapter_read(struct i2c_adapter *ia, uint8_t addr, uint8_t reg,
		     uint8_t *buffer, int len)
{
	if (!ia)
		return SBI_EINVAL;
	if (!ia->read)
		return SBI_ENOSYS;

	return ia->read(ia, addr, reg, buffer, len);
}
