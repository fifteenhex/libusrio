//SPDX-License-Identifier: GPL-2.0-or-later
/*
 * gpiocontroller.c
 *
 */

#include <errno.h>
#include <stdbool.h>

#include "gpio_controller.h"

#include "gpio_controller_log.h"

int gpio_controller_init(const struct gpio_controller *gpio_controller, void **priv)
{
	return 0;
}

int gpio_controller_set_value(const struct gpio_controller *gpio_controller,
		void *priv, int line, bool value)
{
	if (!gpio_controller->set_value)
		return -EPERM;

	return gpio_controller->set_value(gpio_controller, priv, line, value);
}

int gpio_controller_get_value(const struct gpio_controller *gpio_controller,
		void *priv, int line)
{
	if (!gpio_controller->get_value)
		return -EPERM;

	return gpio_controller->get_value(gpio_controller, priv, line);
}
