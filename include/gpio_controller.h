/*
 */

#ifndef _GPIO_CONTROLLER_H_
#define _GPIO_CONTROLLER_H_

#include <stdbool.h>

struct gpio_controller_line {
	int num;
	const char *name;
};

struct gpio_controller_info {
	int num_lines;
	struct gpio_controller_line lines[];
};

struct gpio_controller {
	int (*set_dir)(const struct gpio_controller *gpio_controller, void *priv,
		       int line, bool out);
	int (*set_value)(const struct gpio_controller *gpio_controller, void *priv,
			 int line, bool value);
	int (*get_value)(const struct gpio_controller *gpio_controller, void *priv,
			 int line);

	/* private api, no touchy! */
	int (*init)(const struct gpio_controller *gpio_controller, void **priv);
	int (*get_info)(const struct gpio_controller *gpio_controller, void *priv, struct gpio_controller_info **info);
	void (*shutdown)(const struct gpio_controller *gpio_controller, void *priv);
};

int gpio_controller_init(const struct gpio_controller *gpio_controller, void **priv);
int libusrio_gpio_controller_get_info(const struct gpio_controller *gpio_controller, void *priv, struct gpio_controller_info **info);
int gpio_controller_set_dir(const struct gpio_controller *gpio_controller, void *priv,
			    int line, int dir);
int gpio_controller_set_value(const struct gpio_controller *gpio_controller, void *priv,
			      int line, bool value);
int gpio_controller_get_value(const struct gpio_controller *gpio_controller, void *priv,
			      int line);
void gpio_controller_shutdown(const struct gpio_controller *gpio_controller, void *priv);

extern const struct gpio_controller gpiod_gpio_controller;

#endif
