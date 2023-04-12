//SPDX-License-Identifier: GPL-2.0-or-later
/*
 *
 */
#ifndef __I2C_CONTROLLER_H__
#define __I2C_CONTROLLER_H__

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>

struct i2c_client {
	uint8_t addr;
	void *priv;
};

struct libusrio_i2c_data {
	int(*log_cb)(int level, const char* tag, const char *restrict format,...);
};

struct i2c_controller {
	const char* name;
	int (*open)(const struct i2c_controller *i2c_controller,
			int(*log_cb)(int level, const char* tag, const char *restrict format,...),
			const char* connection, void **priv);
	int (*init)(const struct i2c_controller *i2c_controller,
			int(*log_cb)(int level, const char* tag, const char *restrict format,...),
			void *priv);
	int (*get_func)(const struct i2c_controller *i2c_controller);
	int (*set_addr)(const struct i2c_controller *i2c_controller, uint8_t addr);
	int (*set_speed)(const struct i2c_controller *i2c_controller);
	/* Return a negative on an error, otherwise return the number of messages transmitted */
	int (*do_transaction)(const struct i2c_controller *i2c_controller, struct i2c_rdwr_ioctl_data *i2c_data);
	int (*shutdown)(const struct i2c_controller *i2c_controller);

	/* quirk handling for shitty i2c interfaces */
	int (*max_transfer)(const struct i2c_controller *i2c_controller);
	/* The controller doesn't stop if it gets a NAK during tx */
	bool (*does_not_stop_on_nak)(const struct i2c_controller *i2c_controller);

	struct i2c_client *client;

	/* NOT YOURS DON'T TOUCH!! */
	struct libusrio_i2c_data *_data;
};

static inline void i2c_controller_set_addr(const struct i2c_controller *i2c_controller, uint8_t addr)
{
	i2c_controller->client->addr = addr;

	if (i2c_controller->set_addr)
		i2c_controller->set_addr(i2c_controller, addr);
}

static inline uint8_t i2c_controller_get_addr(const struct i2c_controller *i2c_controller)
{
	assert(i2c_controller);

	return i2c_controller->client->addr;
}

static inline void i2c_controller_set_priv(const struct i2c_controller *i2c_controller, void *priv)
{
	assert(!i2c_controller->client->priv);

	i2c_controller->client->priv = priv;
}

static inline int i2c_controller_max_transfer(const struct i2c_controller *i2c_controller)
{
	assert(i2c_controller);

	if (i2c_controller->max_transfer)
		return i2c_controller->max_transfer(i2c_controller);

	return -ENOTSUP;
}

static inline void *i2c_controller_get_priv(const struct i2c_controller *i2c_controller)
{
	assert(i2c_controller->client->priv);

	return i2c_controller->client->priv;
}

int i2c_controller_open(
		const struct i2c_controller *i2c_controller,
		int(*log_cb)(int level, const char *tag, const char *restrict format,...),
		const char *connection, void **priv);

int i2c_controller_init(
		const struct i2c_controller *i2c_controller,
		int(*log_cb)(int level, const char *tag, const char *restrict format,...),
		void *priv);

int i2c_controller_ping(const struct i2c_controller *i2c_controller, uint8_t addr);

static inline int i2c_controller_do_transaction(const struct i2c_controller *i2c_controller,
		struct i2c_rdwr_ioctl_data *i2c_data, int tries)
{
	int ret;

	assert(i2c_controller);

	for(; tries; tries--) {
		ret = i2c_controller->do_transaction(i2c_controller, i2c_data);
		if (ret <= 0) {
			//printf("Error sending write command: %d, tries left %d\n", ret, tries);
			ret = -1;
		}
		else
			return 0;
	}

	return ret;
}

static inline int i2c_controller_shutdown(const struct i2c_controller *i2c_controller)
{
	int ret = 0;

	if (i2c_controller->shutdown)
		ret = i2c_controller->shutdown(i2c_controller);

	return ret;
}

static inline bool i2c_controller_can_mangle(const struct i2c_controller *i2c_controller)
{
	int ret;

	assert(i2c_controller);

	if (!i2c_controller->get_func)
		return false;

	ret = i2c_controller->get_func(i2c_controller);
	if (ret > 0)
		return ret & I2C_FUNC_NOSTART;

	return false;
}

#define I2C_CONTROLLER_CMD_RETRIES 1

int i2c_controller_cmd_simple(const struct i2c_controller *i2c_controller, uint8_t *cmd, size_t cmdlen);
int i2c_controller_write_then_read_simple(const struct i2c_controller *i2c_controller,
					  uint8_t *writebuf, size_t writesz, uint8_t* readbuf, size_t readsz);
bool i2c_controller_does_not_stop_on_nak(const struct i2c_controller *i2c_controller);
int i2c_controller_cmd(const struct i2c_controller *i2c_controller, uint8_t *cmd, size_t cmdlen);
int i2c_controller_write_then_read(const struct i2c_controller *i2c_controller,
				   uint8_t *writebuf, size_t writesz, uint8_t* readbuf, size_t readsz);
int i2c_controller_write_then_write(const struct i2c_controller *i2c_controller,
				    uint8_t *writebuf0, size_t writesz0, uint8_t* writebuf1, size_t writesz1);
int i2c_controller_write_then_write_simple(const struct i2c_controller *i2c_controller,
					   uint8_t *writebuf0, size_t writesz0, uint8_t* writebuf1, size_t writesz1);
int i2c_controller_cmd_onebyone(const struct i2c_controller *i2c_controller, uint8_t *cmd, size_t cmdlen);

extern const struct i2c_controller i2cdev_i2c;

#endif
