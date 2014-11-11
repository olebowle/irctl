#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "capabilities.h"
#include "stm32.h"

extern struct global_args args;

const struct com_ssize stm32_com_mat[ACCESS_COUNT][COMMAND_COUNT] = {
/*		EMIT	CAPS	FW	ALARM	TRIGCMD	TRIGIR	WAKEIR	*/
/* GET */	{{0,0},	{0,0},	{0,0},	{3,7},	{4,9},	{4,9},	{4,9}},
/* SET */	{{9,3},	{0,0},	{-1,3},	{7,3},	{10,3},	{10,3},	{10,3}},
/* RESET */	{{0,0},	{0,0},	{0,0},	{3,3},	{4,3},	{4,3},	{4,3}}
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

/* https://stackoverflow.com/questions/8774567 */
static inline uint64_t
bit_mask(size_t x)
{
	return (x >= sizeof(uint64_t) * CHAR_BIT) ? (uint64_t) -1 : (1UL << x) - 1;
}

void
stm32_init(struct rc_device *dev)
{
	dev->trig_slots = &stm32_trig_slots;
	dev->wake_slots = &stm32_wake_slots;
	dev->com_mat = &stm32_com_mat;
	dev->rx_protocols = stm32_rx_protocols;
	dev->tx_protocols = stm32_tx_protocols;
}

int
stm32_open(struct rc_device *dev, const char *path, int flags)
{
	dev->fd = open(path, flags);
	if (dev->fd == -1)
		perror("opening device failed\n");
	return dev->fd;
}

int
stm32_close(struct rc_device *dev)
{
	dev->fd = close(dev->fd);
	if (dev->fd == -1)
		perror("closing device failed\n");
	return dev->fd;
}

int
stm32_parse_buf(struct rc_device *dev, const uint8_t *buf, size_t n)
{
	unsigned int idx = 0;
	(void)dev;

	switch((enum status) buf[idx++]) {
	case STAT_CMD:
		/* we should not get a command from the device, only an answer*/
		fprintf(stderr, "unexpected data received\n");
		return EXIT_FAILURE;
	case STAT_SUCCESS:
		printf("command succeeded\n");
		break;
	case STAT_FAILURE:
		printf("command failed\n");
		break;
	}

	if (buf[idx++] != args.acc || buf[idx++] != args.cmd) {
		fprintf(stderr, "unexpected data received\n");
		return EXIT_FAILURE;
	}

	printf("0x");
	for (; idx < n; idx++)
		printf("%02x", buf[idx]);
	printf("\n");
	return EXIT_SUCCESS;
}

ssize_t
stm32_prepare_buf(struct rc_device *dev, uint8_t * const buf, size_t n)
{
	uint64_t code;
	uint8_t *code_ptr;
	size_t idx = 0, exp, rem;
	int arg;

	/* expected number of bytes to transmit */
	exp = (*dev->com_mat)[args.acc][args.cmd].tx;
	if(n < exp) {
		fprintf(stderr, "buffer size not sufficient\n");
		return -1;
	}

	buf[idx++] = STAT_CMD;
	buf[idx++] = args.acc;
	buf[idx++] = args.cmd;

	/* process sub argument if available*/
	if (args.sub_arg) {
		arg = atoi(args.sub_arg);
		if (arg < 1 ||
		    (args.cmd == CMD_TRIG_CMD && arg > dev->trig_slots[0]) ||
		    (args.cmd == CMD_TRIG_IR && arg > dev->trig_slots[0]) ||
		    (args.cmd == CMD_WAKE && arg > dev->wake_slots[0])) {
			fprintf(stderr, "argument out of range\n");
			return -1;
		}

		buf[idx++] = arg - 1;
	}

	/* process set parameter if available */
	if (args.set) {
		rem = exp - idx;
		code = strtoul(args.set, NULL, 16);
		if (code > bit_mask(rem * CHAR_BIT)) {
			fprintf(stderr, "set parameter too long\n");
			return -1;
		}
		code = htobe64(code);
		code_ptr = (uint8_t *) &code;
		memcpy(&buf[idx], code_ptr + sizeof(code) - rem, rem);
		idx += rem;
	}

	if (idx != exp) {
		fprintf(stderr, "unexpected number of bytes to transmit\n");
		return -1;
	}
	return idx;
}

ssize_t
stm32_read(struct rc_device *dev, void *buf, size_t n)
{
	ssize_t ret;
	ssize_t exp;
	ssize_t idx = 0;

	/* expected number of bytes to receive
	 * -1 signifies an undefined number of bytes, so read only once
	 */
	exp = (*dev->com_mat)[args.acc][args.cmd].rx;
	if (exp >= 0 && (size_t) exp > n)
		fprintf(stderr, "buffer size not sufficient\n");
		return -1;

	do {
		ret = read(dev->fd, buf, n);
		if (ret == -1 && errno == EINTR) {
			printf("interrupted by a signal while reading, continuing\n");
			continue;
		}
		if (ret == -1) {
			perror("reading from device failed\n");
			break;
		}
		idx += ret;
	} while (idx < exp || exp < 0);
	return idx;
}

ssize_t
stm32_write(struct rc_device *dev, const void *buf, size_t n)
{
	ssize_t ret;
	ret = write(dev->fd, buf, n);
	if (ret == -1)
		perror("writing to device failed\n");
	return ret;
}
