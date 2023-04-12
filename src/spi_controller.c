//SPDX-License-Identifier: GPL-2.0-or-later
/*
 *
 */

#include <errno.h>
#include <stddef.h>
#include <string.h>

#include <dgputil.h>

#include "spi_controller.h"

#include "spi_controller_log.h"

#define MAX_TRANSFER(len) \
	(spi_controller->max_transfer ? spi_controller->max_transfer(spi_controller, priv) : len)

SPI_CONTROLLER_RTN_T spi_controller_enable_manual_mode(const struct spi_controller *spi_controller,
						       void *priv)
{
	return 0;
}

int spi_controller_cs_assert(const struct spi_controller *spi_controller, void *priv)
{
	assert(spi_controller);

	if(spi_controller->cs_assert)
		spi_controller->cs_assert(spi_controller, priv);
	return 0;
	//return (SPI_CONTROLLER_RTN_T)enable_pins(true);
}

int spi_controller_write1(const struct spi_controller *spi_controller, uint8_t data, void *priv)
{
	assert(spi_controller);

	return spi_controller->send_command(spi_controller, 1, 0, &data, NULL, priv);
}

int spi_controller_cs_release(const struct spi_controller *spi_controller, void *priv)
{
	assert(spi_controller);

	if(spi_controller->cs_release)
		spi_controller->cs_release(spi_controller, priv);

	return 0;
	//return (SPI_CONTROLLER_RTN_T)enable_pins(false);
}

int spi_controller_read(const struct spi_controller *spi_controller,
			uint8_t *ptr_rtn_data, uint32_t len, SPI_CONTROLLER_SPEED_T speed,
			void *priv)
{
	uint32_t chunk_sz = min(len, MAX_TRANSFER(len));

	/*
	 * Handle chunking the transfer when the controller has a smaller max_transfer than the
	 * requested amount.
	 */
	while(len) {
		int read_sz = min(chunk_sz, len);
		int ret;

		ret = spi_controller->send_command(spi_controller, 0, read_sz, NULL, ptr_rtn_data, priv);
		ptr_rtn_data += read_sz;
		len -= read_sz;
		if (ret)
			break;
	}

	return 0;
}

int spi_controller_write(const struct spi_controller *spi_controller,
			 uint8_t *ptr_data, uint32_t len, SPI_CONTROLLER_SPEED_T speed,
			 void *priv)
{
	uint32_t chunk_sz = min(len, MAX_TRANSFER(len));

	/*
	 * Handle chunking the transfer when the controller has a smaller max_transfer than the
	 * requested amount.
	 */
	while(len) {
		int ret;
		int write_sz = min(chunk_sz, len);

		ret = spi_controller->send_command(spi_controller, write_sz, 0,
				ptr_data, NULL, priv);
		ptr_data += write_sz;
		len -= write_sz;
		if (ret)
			return ret;
	}

	return 0;
}

#if 0
SPI_CONTROLLER_RTN_T SPI_CONTROLLER_Xfer_NByte( u8 *ptr_data_in, u32 len_in, u8 *ptr_data_out, u32 len_out, SPI_CONTROLLER_SPEED_T speed )
{
	return (SPI_CONTROLLER_RTN_T)spi_controller->send_command(len_out, len_in, ptr_data_out, ptr_data_in);
}
#endif

int spi_controller_init(const struct spi_controller *spi_controller,
		int(*log_cb)(int level, const char* tag, const char *restrict format,...),
		void *priv)
{
	assert(spi_controller);

	if (!spi_controller->init)
		return -EPERM;

	return spi_controller->init(spi_controller, log_cb, priv);
}

int spi_controller_open(const struct spi_controller *spi_controller,
		int(*log_cb)(int level, const char* tag, const char *restrict format,...),
		const char *connstring, void **priv)
{
	assert(spi_controller);

	if (!spi_controller->open)
		return -EPERM;

	return spi_controller->open(spi_controller, log_cb, connstring, priv);
}

int spi_controller_shutdown(const struct spi_controller *spi_controller, void *priv)
{
	assert(spi_controller);

	if (spi_controller->shutdown)
		return spi_controller->shutdown(spi_controller, priv);

	return 0;
}

int spi_controller_close(const struct spi_controller *spi_controller, void *priv)
{
	assert(spi_controller);

	if (spi_controller->close)
		return spi_controller->close(spi_controller, priv);

	return 0;
}
