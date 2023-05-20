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

/* This is a container for data libusrio needs internally */
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
	int (*set_speed)(const struct i2c_controller *i2c_controller);
	/* Return a negative on an error, otherwise return the number of messages transmitted */
	int (*do_transaction)(const struct i2c_controller *i2c_controller, struct i2c_rdwr_ioctl_data *i2c_data, void *priv);
	int (*shutdown)(const struct i2c_controller *i2c_controller, void *priv);

	/* quirk handling for shitty i2c interfaces */
	int (*max_transfer)(const struct i2c_controller *i2c_controller);
	/* The controller doesn't stop if it gets a NAK during tx */
	bool (*does_not_stop_on_nak)(const struct i2c_controller *i2c_controller);

	/* Internal API */
	struct libusrio_i2c_data *(*get_libusrio_data)(const struct i2c_controller *i2c_controller, void *priv);
};

static inline int i2c_controller_max_transfer(const struct i2c_controller *i2c_controller)
{
	assert(i2c_controller);

	if (i2c_controller->max_transfer)
		return i2c_controller->max_transfer(i2c_controller);

	return -ENOTSUP;
}

int i2c_controller_open(
		const struct i2c_controller *i2c_controller,
		int(*log_cb)(int level, const char *tag, const char *restrict format,...),
		const char *connection, void **priv);

int i2c_controller_init(
		const struct i2c_controller *i2c_controller,
		int(*log_cb)(int level, const char *tag, const char *restrict format,...),
		void *priv);

int i2c_controller_ping(const struct i2c_controller *i2c_controller,
		uint8_t addr,
		void *priv);

static inline int i2c_controller_do_transaction(const struct i2c_controller *i2c_controller,
		struct i2c_rdwr_ioctl_data *i2c_data, int tries, void *priv)
{
	int ret;

	assert(i2c_controller);

	for(; tries; tries--) {
		ret = i2c_controller->do_transaction(i2c_controller, i2c_data, priv);
		if (ret <= 0) {
			//printf("Error sending write command: %d, tries left %d\n", ret, tries);
			ret = -1;
		}
		else
			return 0;
	}

	return ret;
}

int i2c_controller_shutdown(const struct i2c_controller *i2c_controller, void *priv);

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

int i2c_controller_cmd_simple(const struct i2c_controller *i2c_controller,
			      uint8_t addr, uint8_t *cmd, size_t cmdlen,
			      void *priv);
int i2c_controller_write_then_read_simple(const struct i2c_controller *i2c_controller,
					  uint8_t addr, uint8_t *writebuf, size_t writesz, uint8_t* readbuf, size_t readsz,
					  void *priv);
bool i2c_controller_does_not_stop_on_nak(const struct i2c_controller *i2c_controller);
int i2c_controller_cmd(const struct i2c_controller *i2c_controller,
		       uint8_t addr, uint8_t *cmd, size_t cmdlen,
		       void *priv);
int i2c_controller_write_then_read(const struct i2c_controller *i2c_controller,
				   uint8_t addr, uint8_t *writebuf, size_t writesz, uint8_t* readbuf, size_t readsz,
				   void *priv);
int i2c_controller_write_then_write(const struct i2c_controller *i2c_controller,
				    uint8_t addr, uint8_t *writebuf0, size_t writesz0, uint8_t* writebuf1, size_t writesz1,
				    void *priv);
int i2c_controller_write_then_write_simple(const struct i2c_controller *i2c_controller,
					   uint8_t addr, uint8_t *writebuf0, size_t writesz0, uint8_t* writebuf1, size_t writesz1,
					   void *priv);
int i2c_controller_cmd_onebyone(const struct i2c_controller *i2c_controller,
		uint8_t addr, uint8_t *cmd, size_t cmdlen,
		void *priv);

extern const struct i2c_controller i2cdev_i2c;

#endif
