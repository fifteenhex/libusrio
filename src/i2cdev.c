//SPDX-License-Identifier: GPL-2.0-or-later
/*
 * i2cdev.c
 *
 *  Created on: 14 Apr 2023
 *      Author: daniel
 */

#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "i2c_controller.h"

struct i2cdev_priv {
	int fd;
};

static int i2c_controller_i2cdev_open(const struct i2c_controller *i2c_controller,
		int(*log_cb)(int level, const char *tag, const char *restrict format,...),
		const char *connection, void **priv)
{
	struct i2cdev_priv *i2cdev_priv;
	int fd;

	fd = open(connection, O_RDWR);

	if (fd < 0) {
		switch (errno) {
		case EACCES:
			printf("Error opening %s: Permission denied.\n"
				 "Please use sudo or run as root.\n",
				 connection);
			break;
		case ENOENT:
			printf("Error opening %s: No such file.\n"
				 "Please check you specified the correct device.\n",
				 connection);
			break;
		default:
			printf("Error opening %s: %s.\n", connection, strerror(errno));
		}

		return -errno;
	}

	i2cdev_priv = malloc(sizeof(*i2cdev_priv));
	if (!i2cdev_priv)
		return -ENOMEM;

	i2cdev_priv->fd = fd;

	i2c_controller_set_priv(i2c_controller, i2cdev_priv);

	*priv = i2cdev_priv;

	return 0;
}

static int i2c_controller_i2cdevdo_set_addr(const struct i2c_controller *i2c_controller, uint8_t addr)
{
	struct i2cdev_priv *priv;

	assert(i2c_controller);

	priv = i2c_controller_get_priv(i2c_controller);

	if (ioctl(priv->fd, I2C_SLAVE, addr) < 0) {
		printf("Error setting slave address 0x%02x: errno %d.\n",
				addr, errno);
		return -errno;
	}

	return 0;
}

static int i2c_controller_i2cdevdo_transaction(const struct i2c_controller *i2c_controller,
		struct i2c_rdwr_ioctl_data *i2c_data)
{
	struct i2cdev_priv *priv;

	assert(i2c_controller);

	priv = i2c_controller_get_priv(i2c_controller);

	return ioctl(priv->fd, I2C_RDWR, i2c_data);
}

static int i2c_controller_i2cdevdo_shutdown(const struct i2c_controller *i2c_controller)
{
	struct i2cdev_priv *priv;

	assert(i2c_controller);

	priv = i2c_controller_get_priv(i2c_controller);

	if (priv->fd >= 0)
		close(priv->fd);

	return 0;
}

static struct i2c_client i2cdev_client;

static struct libusrio_i2c_data i2cdev_libusrio_data;

const struct i2c_controller i2cdev_i2c = {
	.name = "i2cdev",
	.open = i2c_controller_i2cdev_open,
	.set_addr = i2c_controller_i2cdevdo_set_addr,
	.do_transaction =  i2c_controller_i2cdevdo_transaction,
	.shutdown = i2c_controller_i2cdevdo_shutdown,

	.client = &i2cdev_client,
	._data = &i2cdev_libusrio_data,
};

