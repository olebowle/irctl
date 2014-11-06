#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "capabilities.h"
#include "stm32.h"

extern struct global_args args;

const bool stm32_feat_mat[ACCESS_COUNT][COMMAND_COUNT] = {
/*		EMIT=1	CAPS	FW	ALARM	TRIGCMD	TRIGIR	WAKEIR	*/
/* GET=0 */	{false,	true,	false,	true,	true,	true,	true},
/* SET */	{true,	false,	true,	true,	true,	true,	true},
/* RESET */	{false,	false,	false,	true,	true,	true,	true}

};

const uint8_t stm32_rx_protocols[IRMP_N_PROTOCOLS] = {
	IRMP_SIRCS_PROTOCOL,
	IRMP_NEC_PROTOCOL,
	IRMP_SAMSUNG_PROTOCOL,
	IRMP_KASEIKYO_PROTOCOL,
	IRMP_JVC_PROTOCOL,
	IRMP_NEC16_PROTOCOL,
	IRMP_NEC42_PROTOCOL,
	IRMP_MATSUSHITA_PROTOCOL,
	IRMP_DENON_PROTOCOL,
	IRMP_RC5_PROTOCOL,
	IRMP_RC6_PROTOCOL,
	IRMP_IR60_PROTOCOL,
	IRMP_GRUNDIG_PROTOCOL,
	IRMP_SIEMENS_PROTOCOL,
	IRMP_NOKIA_PROTOCOL,
	0
};

const uint8_t stm32_tx_protocols[IRMP_N_PROTOCOLS] = {
	IRMP_SIRCS_PROTOCOL,
	IRMP_NEC_PROTOCOL,
	IRMP_SAMSUNG_PROTOCOL,
	IRMP_KASEIKYO_PROTOCOL,
	IRMP_JVC_PROTOCOL,
	IRMP_NEC16_PROTOCOL,
	IRMP_NEC42_PROTOCOL,
	IRMP_MATSUSHITA_PROTOCOL,
	IRMP_DENON_PROTOCOL,
	IRMP_RC5_PROTOCOL,
	IRMP_RC6_PROTOCOL,
	IRMP_IR60_PROTOCOL,
	IRMP_GRUNDIG_PROTOCOL,
	IRMP_SIEMENS_PROTOCOL,
	IRMP_NOKIA_PROTOCOL,
	0
};

const uint8_t stm32_trig_slots = 8;
const uint8_t stm32_wake_slots = 1;

void
stm32_init(struct rc_device *dev)
{
	dev->trig_slots = &stm32_trig_slots;
	dev->wake_slots = &stm32_wake_slots;
	dev->feat_mat = &stm32_feat_mat;
	dev->rx_protocols = stm32_rx_protocols;
	dev->tx_protocols = stm32_tx_protocols;
}

int
stm32_open(struct rc_device *dev, const char *path, int flags)
{
	dev->fd = open(path, flags);
	if (dev->fd == -1) {
		perror("opening device failed");
		exit(EXIT_FAILURE);
	}
	return dev->fd;
}

void
stm32_close(struct rc_device *dev)
{
	dev->fd = close(dev->fd);
	if (dev->fd == -1) {
		perror("closing device failes");
		exit(EXIT_FAILURE);
	}
}

ssize_t
stm32_prepare_buf(struct rc_device *dev, uint8_t * const buf, size_t n)
{
	uint64_t code;
	uint8_t *code_ptr;
	size_t idx = 0, rem = 0;
	int tmp;

	if(n < 9) {
		fprintf(stderr, "buffer size not sufficient\n");
		return -1;
	}

	buf[idx++] = args.acc;
	buf[idx++] = args.cmd;

	/* process sub argument if available*/
	if (args.sub_arg) {
		tmp = atoi(args.sub_arg);
		if (tmp < 1 ||
		    (args.cmd == CMD_TRIG_CMD && tmp > dev->trig_slots[0]) ||
		    (args.cmd == CMD_TRIG_IR && tmp > dev->trig_slots[0]) ||
		    (args.cmd == CMD_WAKE && tmp > dev->wake_slots[0])) {
			fprintf(stderr, "slot number out of range\n");
			return -1;
		}

		buf[idx++] = tmp - 1;
	}

	/* process set parameter if available */
	if (args.set) {
		code = strtoul(args.set, NULL, 16);
		if (code > 0xffffffffffff) {
			fprintf(stderr, "set parameter too long\n");
			return -1;
		}
		code = htobe64(code);
		code_ptr = (uint8_t*) &code;
		rem = 6;
	}

	memcpy(&buf[idx], code_ptr + 2, rem);
	idx += rem;
	return idx;
}

ssize_t
stm32_write(struct rc_device *dev, const void *buf, size_t n)
{
	return write(dev->fd, buf, n);
}
