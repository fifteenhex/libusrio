//SPDX-License-Identifier: GPL-2.0-or-later
/*
 *
 */
#ifndef __LIBUSRIO_MFD_H__
#define __LIBUSRIO_MFD_H__

#include "gpio_controller.h"
#include "i2c_controller.h"
#include "pinctrl.h"
#include "spi_controller.h"

struct libusrio_mfd {
	/* public api */
	int (*open)(const struct libusrio_mfd *mfd, int(*log_cb)(int level, const char* tag, const char *restrict format,...),
		const char *connection_string, void **priv);
	const struct gpio_controller *(*get_gpio)(const struct libusrio_mfd *mfd, void *priv);
	const struct i2c_controller *(*get_i2c)(const struct libusrio_mfd *mfd, void *priv);
	const struct libusrio_pinctrl *(*get_pinctrl)(const struct libusrio_mfd *mfd, void *priv);
	const struct spi_controller *(*get_spi)(const struct libusrio_mfd *mfd, void *priv);

	/* private api */
	int (*set_flags)(const struct libusrio_mfd *mfd, void *priv, unsigned int flags);
	int (*get_flags)(const struct libusrio_mfd *mfd, void *priv, unsigned int *flags);
};

#define LIBUSRIO_MFD_WANTI2C	1
#define LIBUSRIO_MFD_WANTGPIO	(1 << 1)
#define LIBUSRIO_MFD_WANTSPI	(1 << 2)

int libusrio_mfd_open(const struct libusrio_mfd *mfd, int(*log_cb)(int level, const char* tag, const char *restrict format,...),
		      const char *connection_string, unsigned int flag, void **priv);

int libusrio_mfd_get_gpio(const struct libusrio_mfd *mfd, void *priv, const struct gpio_controller **gpio);
int libusrio_mfd_get_i2c(const struct libusrio_mfd *mfd, void *priv, const struct i2c_controller **i2c);
int libusrio_mfd_get_pinctrl(const struct libusrio_mfd *mfd, void *priv, const struct libusrio_pinctrl **pinctrl);
int libusrio_mfd_get_spi(const struct libusrio_mfd *mfd, void *priv, const struct spi_controller **spi);

int libusrio_mfd_close(const struct libusrio_mfd *mfd, void *priv);

#endif
