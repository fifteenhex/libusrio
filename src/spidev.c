//SPDX-License-Identifier: GPL-2.0-or-later
/*
 *
 */

#include <dgputil.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>

#include "spi_controller.h"

struct spidev_priv {
	int fd;
};

static int spidev_send_command(const struct spi_controller *spi_controller,
			       unsigned int write_sz, unsigned int read_sz,
			       const unsigned char *write_buf, unsigned char *read_buf,
			       void *priv)
{
	struct spidev_priv *spidev_priv = priv;

	struct spi_ioc_transfer msgs[] = {
		{
			.tx_buf = write_buf,
			.len = write_sz,
		},
		{
			.rx_buf = read_buf,
			.len = read_sz,
		},
	};

	int status = ioctl(spidev_priv->fd, SPI_IOC_MESSAGE(array_size(msgs)), msgs);

	return 0;
}

static int spidev_open(const struct spi_controller *spi_controller,
		int(*log_cb)(int level, const char* tag, const char *restrict format,...),
		const char *connection_string, void **priv)
{
	struct spidev_priv *spidev_priv;
	int fd;

	fd = open(connection_string, O_RDWR);

	if (fd < 0)
		return fd;

	spidev_priv = malloc(sizeof(*spidev_priv));
	if (!spidev_priv)
		return -ENOMEM;

	spidev_priv->fd = fd;

	*priv = spidev_priv;

	return 0;
}

const struct spi_controller spidev_spi = {
	.name = "spidev",
	.open = spidev_open,
	.send_command = spidev_send_command,
};
