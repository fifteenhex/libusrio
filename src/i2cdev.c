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
	struct libusrio_i2c_data libusrio_i2c_data;
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

	*priv = i2cdev_priv;

	return 0;
}

static int i2c_controller_i2cdevdo_transaction(const struct i2c_controller *i2c_controller,
		struct i2c_rdwr_ioctl_data *i2c_data,
		void *priv)
{
	struct i2cdev_priv *i2cdev_priv = priv;

	assert(i2c_controller);

	return ioctl(i2cdev_priv->fd, I2C_RDWR, i2c_data);
}

static int i2c_controller_i2cdevdo_shutdown(const struct i2c_controller *i2c_controller, void *priv)
{
	struct i2cdev_priv *i2cdev_priv = priv;

	assert(i2c_controller);

	if (i2cdev_priv->fd >= 0)
		close(i2cdev_priv->fd);

	return 0;
}

const struct i2c_controller i2cdev_i2c = {
	.name = "i2cdev",
	.open = i2c_controller_i2cdev_open,
	.do_transaction =  i2c_controller_i2cdevdo_transaction,
	.shutdown = i2c_controller_i2cdevdo_shutdown,
};

