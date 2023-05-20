/*
 *
 */

#include "i2c_controller.h"
#include "spi_controller.h"
#include "gpio_controller.h"
#include "mfd.h"

const struct i2c_controller dummy_i2c = {
	.name = "dummy_i2c",
};

const struct spi_controller dummy_spi = {
	.name = "dummy_spi",
};

const struct gpio_controller dummy_gpio = {

};

struct libusrio_mfd dummy_mfd = {

};
