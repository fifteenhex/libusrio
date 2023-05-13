//SPDX-License-Identifier: GPL-2.0-or-later
/* */

#include <assert.h>
#include <dgputil.h>

#include "mfd.h"

int libusrio_mfd_open(const struct libusrio_mfd *mfd, int(*log_cb)(int level, const char* tag, const char *restrict format,...),
	const char *connection_string, unsigned int flags, void **priv)
{
	const struct i2c_controller *i2c = NULL;
	const struct gpio_controller *gpio = NULL;
	const struct spi_controller *spi = NULL;

	assert(mfd->open);
	assert(mfd->set_flags);
	assert(mfd->get_flags);

	int ret = mfd->open(mfd, log_cb, connection_string, priv);
	if (ret)
		return ret;

	if (flags & LIBUSRIO_MFD_WANTI2C) {
		if (mfd->get_i2c) {
			i2c = mfd->get_i2c(mfd, *priv);
			ret = i2c_controller_init(i2c, log_cb, *priv);
			if (ret)
				goto err_i2c;
		}
		else {
			ret = -ENODEV;
			goto err_i2c;
		}
	}

	if (flags & LIBUSRIO_MFD_WANTGPIO) {
		if (mfd->get_gpio) {
			gpio = mfd->get_gpio(mfd, *priv);
			ret = gpio_controller_init(gpio, *priv);
			if (ret)
				goto err_gpio;
		}
		else {
			ret = -ENODEV;
			goto err_gpio;
		}
	}

	if (flags & LIBUSRIO_MFD_WANTSPI) {
		if (mfd->get_spi) {
			spi = mfd->get_spi(mfd, *priv);
			ret = spi_controller_init(spi, log_cb, *priv);
			if (ret)
				goto err_spi;
		}
		else {
			ret = -ENODEV;
			goto err_spi;
		}
	}

	mfd->set_flags(mfd, *priv, flags);

	return 0;

err_spi:
err_gpio:
err_i2c:
	return ret;
}

int libusrio_mfd_get_gpio(const struct libusrio_mfd *mfd, void *priv, const struct gpio_controller **gpio)
{
	assert(gpio);

	if (mfd->get_gpio) {
		*gpio = mfd->get_gpio(mfd, priv);
		return 0;
	}

	return -ENODEV;
}

int libusrio_mfd_get_i2c(const struct libusrio_mfd *mfd, void *priv, const struct i2c_controller **i2c)
{
	assert(i2c);

	if (mfd->get_i2c) {
		*i2c = mfd->get_i2c(mfd, priv);
		return 0;
	}

	return -ENODEV;
}

int libusrio_mfd_get_spi(const struct libusrio_mfd *mfd, void *priv, const struct spi_controller **spi)
{
	assert(spi);

	if (mfd->get_spi) {
		*spi = mfd->get_spi(mfd, priv);
		return 0;
	}

	return -ENODEV;
}

int libusrio_mfd_close(const struct libusrio_mfd *mfd, void *priv)
{
	const struct i2c_controller *i2c = NULL;
	const struct gpio_controller *gpio = NULL;
	const struct spi_controller *spi = NULL;
	unsigned int flags;
	int ret;

	mfd->get_flags(mfd, priv, &flags);

	if (flags & LIBUSRIO_MFD_WANTI2C) {
		if (mfd->get_i2c) {
			i2c = mfd->get_i2c(mfd, priv);
			ret = i2c_controller_shutdown(i2c, priv);
			if (ret)
				goto err_i2c;
		}
		else {
			ret = -ENODEV;
			goto err_i2c;
		}
	}

	if (flags & LIBUSRIO_MFD_WANTGPIO) {
		if (mfd->get_gpio) {
			gpio = mfd->get_gpio(mfd, priv);
			gpio_controller_shutdown(gpio, priv);
		}
		else {
			ret = -ENODEV;
			goto err_gpio;
		}
	}

	if (flags & LIBUSRIO_MFD_WANTSPI) {
		if (mfd->get_spi) {
			spi = mfd->get_spi(mfd, priv);
			ret = spi_controller_shutdown(spi, priv);
			if (ret)
				goto err_spi;
		}
		else {
			ret = -ENODEV;
			goto err_spi;
		}
	}

err_spi:
err_gpio:
err_i2c:
	return 0;
}
