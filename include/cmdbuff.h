/*
 * cmdbuff.h
 */

#ifndef SRC_CMDBUFF_H_
#define SRC_CMDBUFF_H_

#include <assert.h>
#include <stdint.h>

struct cmdbuff {
	uint8_t buff[32];
	unsigned int pos;
};

#define CMDBUFF(_name) struct cmdbuff _name = { 0 }
#define cmdbuff_ptr(_b) ((_b)->buff)
#define cmdbuff_size(_b) ((_b)->pos)

static inline void cmdbuff_push(struct cmdbuff *cmdbuff, uint8_t value)
{
	assert(cmdbuff->pos < sizeof(cmdbuff->buff));

	cmdbuff->buff[cmdbuff->pos++] = value;
}

#endif /* SRC_CMDBUFF_H_ */
