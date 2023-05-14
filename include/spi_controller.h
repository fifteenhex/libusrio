//SPDX-License-Identifier: GPL-2.0-or-later
/*
 *
 */
#ifndef __SPI_CONTROLLER_H__
#define __SPI_CONTROLLER_H__

#include <assert.h>
#include <stdint.h>
#include <stdbool.h>

#include <linux/spi/spidev.h>

typedef enum{
	SPI_CONTROLLER_SPEED_SINGLE = 0,
	SPI_CONTROLLER_SPEED_DUAL,
	SPI_CONTROLLER_SPEED_QUAD

} SPI_CONTROLLER_SPEED_T;

typedef enum{
	SPI_CONTROLLER_RTN_NO_ERROR = 0,
	SPI_CONTROLLER_RTN_SET_OPFIFO_ERROR,
	SPI_CONTROLLER_RTN_READ_DATAPFIFO_ERROR,
	SPI_CONTROLLER_RTN_WRITE_DATAPFIFO_ERROR,
	SPI_CONTROLLER_RTN_DEF_NO
} SPI_CONTROLLER_RTN_T;

typedef enum{
	SPI_CONTROLLER_MODE_AUTO = 0,
	SPI_CONTROLLER_MODE_MANUAL,
	SPI_CONTROLLER_MODE_NO
} SPI_CONTROLLER_MODE_T;

struct spi_controller {
	const char *name;

	/* Lifecycle stuff */
	int (*open)(const struct spi_controller *spi_controller,
		    int(*log_cb)(int level, const char* tag, const char *restrict format,...),
		    const char *connection_string, void **priv);
	int (*init)(const struct spi_controller *spi_controller,
		    int(*log_cb)(int level, const char* tag, const char *restrict format,...),
		    void *priv);
	int (*shutdown)(const struct spi_controller *spi_controller, void *priv);
	int (*close)(const struct spi_controller *spi_controller, void *priv);

	/* Do stuff stuff */
	/* todo this is garbage api */
	int (*send_command)(const struct spi_controller *spi_controller,
			    unsigned int write_size, unsigned int read_size,
			    const unsigned char *write_buff, unsigned char *read_buff, void *priv);
	int (*cs_assert)(const struct spi_controller *spi_controller, void *priv);
	int (*cs_release)(const struct spi_controller *spi_controller, void *priv);
	int (*max_transfer)(const struct spi_controller *spi_controller, void *priv);

	/* Does this SPI controller need a special connection string? */
	bool (*need_connstring)(const struct spi_controller *spi_controller);
	const char *(*connstring_format)(const struct spi_controller *spi_controller);
};

SPI_CONTROLLER_RTN_T spi_controller_enable_manual_mode(const struct spi_controller *spi_controller,
						       void *priv);
int spi_controller_write1(const struct spi_controller *spi_controller,
			  uint8_t data, void *priv);
int spi_controller_write(const struct spi_controller *spi_controller,
			 uint8_t *ptr_data, uint32_t len, SPI_CONTROLLER_SPEED_T speed,
			 void *priv);
int spi_controller_read(const struct spi_controller *spi_controller,
			uint8_t *ptr_rtn_data, uint32_t len, SPI_CONTROLLER_SPEED_T speed,
			void *priv);
int spi_controller_cs_assert(const struct spi_controller *spi_controller, void *priv);
int spi_controller_cs_release(const struct spi_controller *spi_controller, void *priv);

#if 0
SPI_CONTROLLER_RTN_T SPI_CONTROLLER_Xfer_NByte( u8 *ptr_data_in, u32 len_in, u8 *ptr_data_out, u32 len_out, SPI_CONTROLLER_SPEED_T speed );
#endif

int spi_controller_open(const struct spi_controller *spi_controller,
		int(*log_cb)(int level, const char* tag, const char *restrict format,...),
		const char *connstring, void **priv);

int spi_controller_init(const struct spi_controller *spi_controller,
		int(*log_cb)(int level, const char* tag, const char *restrict format,...),
		void *priv);
int spi_controller_shutdown(const struct spi_controller *spi_controller, void *priv);
int spi_controller_close(const struct spi_controller *spi_controller, void *priv);

static inline bool spi_controller_need_connstring(const struct spi_controller *spi_controller)
{
	assert(spi_controller);

	if (spi_controller->need_connstring)
		return spi_controller->need_connstring(spi_controller);

	return false;
}

extern const struct spi_controller spidev_spi;

#endif
