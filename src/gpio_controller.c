//SPDX-License-Identifier: GPL-2.0-or-later
/*
 * gpiocontroller.c
 *
 */

#include <assert.h>
#include <errno.h>
#include <stdbool.h>

#include "gpio_controller.h"

#include "gpio_controller_log.h"

int gpio_controller_init(const struct gpio_controller *gpio_controller, void **priv)
{
	return 0;
}

int libusrio_gpio_controller_get_info(const struct gpio_controller *gpio_controller, void *priv, struct gpio_controller_info **info)
{
	assert(gpio_controller);

	if (!gpio_controller->get_info)
		return -EPERM;

	return gpio_controller->get_info(gpio_controller, priv, info);
}

int gpio_controller_set_value(const struct gpio_controller *gpio_controller,
		void *priv, int line, bool value)
{
	assert(gpio_controller);

	if (!gpio_controller->set_value)
		return -EPERM;

	return gpio_controller->set_value(gpio_controller, priv, line, value);
}

int gpio_controller_get_value(const struct gpio_controller *gpio_controller,
		void *priv, int line)
{
	assert(gpio_controller);

	if (!gpio_controller->get_value)
		return -EPERM;

	return gpio_controller->get_value(gpio_controller, priv, line);
}

void gpio_controller_shutdown(const struct gpio_controller *gpio_controller, void *priv)
{
	assert(gpio_controller);

	if (!gpio_controller->shutdown)
		return;

	gpio_controller->shutdown(gpio_controller, priv);
}
