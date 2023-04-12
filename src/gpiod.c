//SPDX-License-Identifier: GPL-2.0-or-later
/*
 *
 */

#define _XOPEN_SOURCE 700
#include <time.h>
#include <gpiod.h>

#include "gpio_controller.h"

static int libusrio_gpiod_init(const struct gpio_controller *gpio_controller, void **priv)
{
	return 0;
}

static int libusrio_gpiod_set_dir(const struct gpio_controller *gpio_controller, void *priv,
		int line, bool out)
{
	return 0;
}

static int libusrio_gpiod_set_value(const struct gpio_controller *gpio_controller, void *priv,
		int line, bool value)
{
	return 0;
}

static int libusrio_gpiod_get_value(const struct gpio_controller *gpio_controller, void *priv,
		int line)
{
	return 0;
}

static void libusrio_gpiod_shutdown(const struct gpio_controller *gpio_controller, void *priv)
{

}

const struct gpio_controller gpiod_gpio_controller = {
	.init = libusrio_gpiod_init,
	.set_dir = libusrio_gpiod_set_dir,
	.set_value = libusrio_gpiod_set_value,
	.get_value = libusrio_gpiod_get_value,
	.shutdown = libusrio_gpiod_shutdown,
};
