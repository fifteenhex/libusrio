//SPDX-License-Identifier: GPL-2.0-or-later
/*
 */

#include <assert.h>
#include <stdint.h>

#include <dgputil.h>

#include "i2c_controller.h"

#include "i2c_controller_log.h"

static struct libusrio_i2c_data *i2c_controller_get_i2c_data(const struct i2c_controller *i2c_controller, void *priv)
{
	assert(i2c_controller->get_libusrio_data);

	struct libusrio_i2c_data *i2c_data = i2c_controller->get_libusrio_data(i2c_controller, priv);

	assert(i2c_data);

	return i2c_data;
}

int i2c_controller_open(
		const struct i2c_controller *i2c_controller,
		int(*log_cb)(int level, const char *tag, const char *restrict format,...),
		const char *connection, void **priv)
{
	int ret;
	struct libusrio_i2c_data tmp_data = { .log_cb = log_cb };


	assert(log_cb);
	assert(i2c_controller->init);

	ret = i2c_controller->open(i2c_controller, log_cb, connection, priv);
	if (ret) {
		i2c_controller_err(&tmp_data, "failed to init i2c controller %s: %d\n",
				i2c_controller->name, ret);
		return ret;
	}

	struct libusrio_i2c_data *i2c_data = i2c_controller_get_i2c_data(i2c_controller, *priv);
	i2c_data->log_cb = log_cb;

	return 0;
}

int i2c_controller_init(
		const struct i2c_controller *i2c_controller,
		int(*log_cb)(int level, const char *tag, const char *restrict format,...),
		void *priv)
{
	int ret;
	struct libusrio_i2c_data tmp_data = { .log_cb = log_cb };

	assert(log_cb);
	assert(priv);
	assert(i2c_controller->init);

	ret = i2c_controller->init(i2c_controller, log_cb, priv);
	if (ret) {
		i2c_controller_err(&tmp_data, "failed to init i2c controller %s: %d\n",
				i2c_controller->name, ret);
		return ret;
	}


	struct libusrio_i2c_data *i2c_data = i2c_controller_get_i2c_data(i2c_controller, priv);
	i2c_data->log_cb = log_cb;

	return 0;
}

int i2c_controller_ping(const struct i2c_controller *i2c_controller, uint8_t addr, void *priv)
{
	struct i2c_rdwr_ioctl_data i2c_data = { 0 };
	struct i2c_msg msg[1] = { 0 };
	int ret;

	i2c_data.nmsgs = array_size(msg);
	i2c_data.msgs = msg;
	i2c_data.msgs[0].addr = addr;
	i2c_data.msgs[0].len = 0;
	i2c_data.msgs[0].buf = NULL;

	ret = i2c_controller_do_transaction(i2c_controller, &i2c_data,
			I2C_CONTROLLER_CMD_RETRIES, priv);

	return ret;
}

int i2c_controller_cmd_simple(const struct i2c_controller *i2c_controller,
		uint8_t addr, uint8_t *cmd, size_t cmdlen,
		void *priv)
{
	struct i2c_rdwr_ioctl_data i2c_data = { 0 };
	struct i2c_msg msg[1] = { 0 };
	int ret;

	i2c_data.nmsgs = array_size(msg);
	i2c_data.msgs = msg;
	i2c_data.msgs[0].addr = addr;
	i2c_data.msgs[0].len = cmdlen;
	i2c_data.msgs[0].buf = cmd;

	ret = i2c_controller_do_transaction(i2c_controller, &i2c_data,
			I2C_CONTROLLER_CMD_RETRIES, priv);

	return ret;
}

static inline int i2c_controller_mangle_needed_messages(const struct i2c_controller *i2c_controller,
		size_t sz)
{
	int max_transfer = i2c_controller_max_transfer(i2c_controller);

	assert(sz);

	return (sz / max_transfer) + ((sz % max_transfer) ? 1 : 0);
}

/* Write two buffers using mangling to chop them up into multiple i2c messages */
static inline int i2c_controller_write_then_write_mangled(const struct i2c_controller *i2c_controller,
		uint8_t addr, uint8_t *writebuf0, size_t writesz0, uint8_t* writebuf1, size_t writesz1,
		void *priv)
{
	int msgs_for_first, msgs_for_second, msgs_total, max_transfer;
	struct i2c_rdwr_ioctl_data i2c_data;
	struct i2c_msg *msg;
	size_t remainder;
	int ret;

	assert(i2c_controller);

	msgs_for_first = i2c_controller_mangle_needed_messages(i2c_controller, writesz0);
	msgs_for_second = i2c_controller_mangle_needed_messages(i2c_controller, writesz1);
	msgs_total = msgs_for_first + msgs_for_second;
	max_transfer = i2c_controller_max_transfer(i2c_controller);

	msg = malloc(sizeof(*msg) * msgs_total);
	if (!msg)
		return -ENOMEM;

	memset(&i2c_data, 0, sizeof(i2c_data));
	memset(msg, 0, sizeof(msg) * msgs_total);

	i2c_data.nmsgs = msgs_total;
	i2c_data.msgs = msg;

	printf("using %d + %d txfrs\n", msgs_for_first, msgs_for_second);

	remainder = writesz0;
	for(int i = 0; i < msgs_for_first; i++) {
		struct i2c_msg *msg = &i2c_data.msgs[i];

		msg->addr = addr;
		msg->buf = writebuf0 + (max_transfer * i);
		msg->len = min(remainder, max_transfer);
		if (i != 0)
			msg->flags = I2C_M_NOSTART;
		remainder -= max_transfer;
	}

	remainder = writesz1;
	for(int i = 0; i < msgs_for_second; i++) {
		struct i2c_msg *msg = &i2c_data.msgs[msgs_for_first + i];

		msg->addr = addr;
		msg->buf = writebuf1 + (max_transfer * i);
		msg->len = min(remainder, max_transfer);
		msg->flags = I2C_M_NOSTART;
		remainder -= max_transfer;
	}

	ret = i2c_controller_do_transaction(i2c_controller, &i2c_data, 10, priv);

	free(msg);

	return ret;
}

/* Simple write for two buffers, allocate some memory, concat and send */
int i2c_controller_write_then_write_simple(const struct i2c_controller *i2c_controller,
		uint8_t addr, uint8_t *writebuf0, size_t writesz0, uint8_t* writebuf1, size_t writesz1,
		void *priv)
{
	struct i2c_rdwr_ioctl_data i2c_data;
	struct i2c_msg msg[1];
	int totalsize = writesz0 + writesz1;
	uint8_t *combinedbuffer;
	int ret;

	combinedbuffer = malloc(totalsize);
	if (!combinedbuffer)
		return -ENOMEM;

	memcpy(combinedbuffer, writebuf0, writesz0);
	memcpy(combinedbuffer + writesz0, writebuf1, writesz1);

	memset(&i2c_data, 0, sizeof(i2c_data));
	memset(&msg, 0, sizeof(msg));

	i2c_data.nmsgs = array_size(msg);
	i2c_data.msgs = msg;
	i2c_data.msgs[0].addr = addr;
	i2c_data.msgs[0].buf = combinedbuffer;
	i2c_data.msgs[0].len = totalsize;

	ret = i2c_controller_do_transaction(i2c_controller, &i2c_data, 10, priv);

	free(combinedbuffer);

	return ret;
}

int i2c_controller_write_then_write(const struct i2c_controller *i2c_controller,
		uint8_t addr, uint8_t *writebuf0, size_t writesz0, uint8_t* writebuf1, size_t writesz1,
		void *priv)
{
	int max_transfer = i2c_controller_max_transfer(i2c_controller);

	assert(writebuf0);
	assert(writebuf1);

	if ((writesz0 + writesz1) <= max_transfer)
		return i2c_controller_write_then_write_simple(i2c_controller, addr,
				writebuf0, writesz0, writebuf1, writesz1, priv);
	else if (i2c_controller_can_mangle(i2c_controller))
		return i2c_controller_write_then_write_mangled(i2c_controller, addr,
				writebuf0, writesz0, writebuf1, writesz1, priv);
	else
		return -EINVAL;
}

static inline int i2c_controller_write_then_read_mangled(const struct i2c_controller *i2c_controller,
		uint8_t addr, uint8_t *writebuf, size_t writesz, uint8_t* readbuf, size_t readsz,
		void *priv)
{
	int msgs_for_first, msgs_for_second, msgs_total, max_transfer;
	struct i2c_rdwr_ioctl_data i2c_data;
	struct i2c_msg *msg;
	size_t remainder;
	int ret;

	assert(i2c_controller);

	msgs_for_first = i2c_controller_mangle_needed_messages(i2c_controller, writesz);
	msgs_for_second = i2c_controller_mangle_needed_messages(i2c_controller, readsz);
	msgs_total = msgs_for_first + msgs_for_second;
	max_transfer = i2c_controller_max_transfer(i2c_controller);

	msg = malloc(sizeof(*msg) * msgs_total);
	if (!msg)
		return -ENOMEM;

	memset(&i2c_data, 0, sizeof(i2c_data));
	memset(msg, 0, sizeof(msg) * msgs_total);

	i2c_data.nmsgs = msgs_total;
	i2c_data.msgs = msg;

	i2c_controller_dbg("using %d + %d txfrs for mangled read, write %d, read %d\n",
			msgs_for_first, msgs_for_second, writesz, readsz);

	remainder = writesz;
	for(int i = 0; i < msgs_for_first; i++) {
		struct i2c_msg *msg = &i2c_data.msgs[i];

		msg->addr = addr;
		msg->buf = writebuf + (max_transfer * i);
		msg->len = min(remainder, max_transfer);
		if (i != 0)
			msg->flags = I2C_M_NOSTART;
		msg->flags |= I2C_M_IGNORE_NAK;
		remainder -= max_transfer;
	}

	remainder = readsz;
	for(int i = 0; i < msgs_for_second; i++) {
		struct i2c_msg *msg = &i2c_data.msgs[msgs_for_first + i];

		msg->addr = addr;
		msg->buf = readbuf + (max_transfer * i);
		msg->len = min(remainder, max_transfer);
		msg->flags = I2C_M_RD;
		if (i != 0)
			msg->flags |= I2C_M_NOSTART;
		remainder -= max_transfer;
	}

	ret = i2c_controller_do_transaction(i2c_controller, &i2c_data, 10, priv);

	free(msg);

	return ret;
}

int i2c_controller_write_then_read_simple(const struct i2c_controller *i2c_controller,
		uint8_t addr, uint8_t *writebuf, size_t writesz, uint8_t* readbuf, size_t readsz,
		void *priv)
{
	struct i2c_rdwr_ioctl_data i2c_data = { 0 };
	struct i2c_msg msg[2] = { 0 };

	i2c_data.nmsgs = array_size(msg);
	i2c_data.msgs = msg;
	i2c_data.msgs[0].addr = addr;
	i2c_data.msgs[0].buf = writebuf;
	i2c_data.msgs[0].len = writesz;
	i2c_data.msgs[0].flags = I2C_M_IGNORE_NAK;

	i2c_data.msgs[1].addr = addr;
	i2c_data.msgs[1].buf = readbuf;
	i2c_data.msgs[1].len = readsz;
	i2c_data.msgs[1].flags = I2C_M_RD;

	return i2c_controller_do_transaction(i2c_controller, &i2c_data, 10, priv);
}


int i2c_controller_write_then_read(const struct i2c_controller *i2c_controller,
		uint8_t addr, uint8_t *writebuf, size_t writesz, uint8_t* readbuf, size_t readsz,
		void *priv)
{
	int max_transfer = i2c_controller_max_transfer(i2c_controller);

	if (writesz <= max_transfer && readsz <= max_transfer)
		return i2c_controller_write_then_read_simple(i2c_controller, addr,
				writebuf, writesz, readbuf, readsz, priv);
	else if (i2c_controller_can_mangle(i2c_controller))
		return i2c_controller_write_then_read_mangled(i2c_controller, addr,
				writebuf, writesz, readbuf, readsz, priv);
	else
		return -EINVAL;
}

bool i2c_controller_does_not_stop_on_nak(const struct i2c_controller *i2c_controller)
{
	assert(i2c_controller);

	if (i2c_controller->does_not_stop_on_nak)
		return i2c_controller->does_not_stop_on_nak(i2c_controller);

	return false;
}

int i2c_controller_cmd(const struct i2c_controller *i2c_controller, uint8_t addr,
		uint8_t *cmd, size_t cmdlen, void *priv)
{
	assert(i2c_controller);

	if (i2c_controller_does_not_stop_on_nak(i2c_controller)) {
		return i2c_controller_cmd_onebyone(i2c_controller, addr, cmd, cmdlen, priv);
	}
	else
		return i2c_controller_cmd_simple(i2c_controller, addr, cmd, cmdlen, priv);
}

int i2c_controller_cmd_onebyone(const struct i2c_controller *i2c_controller,
		uint8_t addr, uint8_t *cmd, size_t cmdlen,
		void *priv)
{
	struct i2c_rdwr_ioctl_data i2c_data = { 0 };
	int nummsgs = cmdlen + 1;
	struct i2c_msg *msgs;
	int ret;

	assert(i2c_controller);

	msgs = malloc(sizeof(*msgs) * nummsgs);
	if (!msgs)
		return -ENOMEM;

	memset(&i2c_data, 0, sizeof(i2c_data));
	memset(msgs, 0, sizeof(msgs) * nummsgs);

	i2c_data.nmsgs = nummsgs;
	i2c_data.msgs = msgs;

	/* For the first message just send the address */
	{
		struct i2c_msg *msg = &i2c_data.msgs[0];

		msg->addr = addr;
	}

	for (int i = 0; i < cmdlen; i++) {
		struct i2c_msg *msg = &i2c_data.msgs[i + 1];

		msg->addr = addr;
		msg->len = 1;
		msg->buf = &cmd[i];
		msg->flags = I2C_M_NOSTART;
	}

	ret = i2c_controller_do_transaction(i2c_controller, &i2c_data,
			I2C_CONTROLLER_CMD_RETRIES, priv);

	return ret;
}

int i2c_controller_shutdown(const struct i2c_controller *i2c_controller, void *priv)
{
	int ret = 0;

	if (i2c_controller->shutdown)
		ret = i2c_controller->shutdown(i2c_controller, priv);

	return ret;
}
