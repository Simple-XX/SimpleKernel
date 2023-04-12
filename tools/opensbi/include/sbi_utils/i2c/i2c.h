/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2021 YADRO
 *
 * Authors:
 *   Nikita Shubin <n.shubin@yadro.com>
 */

#ifndef __I2C_H__
#define __I2C_H__

#include <sbi/sbi_types.h>
#include <sbi/sbi_list.h>

/** Representation of a I2C adapter */
struct i2c_adapter {
	/** Pointer to I2C driver owning this I2C adapter */
	void *driver;

	/** Unique ID of the I2C adapter assigned by the driver */
	int id;

	/**
	 * Send buffer to given address, register
	 *
	 * @return 0 on success and negative error code on failure
	 */
	int (*write)(struct i2c_adapter *ia, uint8_t addr, uint8_t reg,
		    uint8_t *buffer, int len);

	/**
	 * Read buffer from given address, register
	 *
	 * @return 0 on success and negative error code on failure
	 */
	int (*read)(struct i2c_adapter *ia, uint8_t addr, uint8_t reg,
		    uint8_t *buffer, int len);

	/** List */
	struct sbi_dlist node;
};

static inline struct i2c_adapter *to_i2c_adapter(struct sbi_dlist *node)
{
	return container_of(node, struct i2c_adapter, node);
}

/** Find a registered I2C adapter */
struct i2c_adapter *i2c_adapter_find(int id);

/** Register I2C adapter */
int i2c_adapter_add(struct i2c_adapter *ia);

/** Un-register I2C adapter */
void i2c_adapter_remove(struct i2c_adapter *ia);

/** Send to device on I2C adapter bus */
int i2c_adapter_write(struct i2c_adapter *ia, uint8_t addr, uint8_t reg,
		      uint8_t *buffer, int len);

/** Read from device on I2C adapter bus */
int i2c_adapter_read(struct i2c_adapter *ia, uint8_t addr, uint8_t reg,
		     uint8_t *buffer, int len);

static inline int i2c_adapter_reg_write(struct i2c_adapter *ia, uint8_t addr,
				 uint8_t reg, uint8_t val)
{
	return i2c_adapter_write(ia, addr, reg, &val, 1);
}

static inline int i2c_adapter_reg_read(struct i2c_adapter *ia, uint8_t addr,
				       uint8_t reg, uint8_t *val)
{
	uint8_t buf;
	int ret = i2c_adapter_read(ia, addr, reg, &buf, 1);

	if (ret)
		return ret;

	*val = buf;
	return 0;
}

#endif
