#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "capabilities.h"
#include "irmpsystem.h"
#include "stm32.h"

extern struct global_args args;

const struct com_ssize stm32_com_mat[ACCESS_COUNT][COMMAND_COUNT] = {
/*		EMIT	CAPS	FW	ALARM	MACRO	WAKE	*/
/* GET */	{{0,0},	{0,0},	{0,0},	{3,7},	{5,9},	{4,9}},
/* SET */	{{9,3},	{0,0},	{-1,3},	{7,3},	{11,3},	{10,3}},
/* RESET */	{{0,0},	{0,0},	{0,0},	{3,3},	{5,3},	{4,3}}
};

/* +1 --> make sure we really have a NULL termination in all cases */
uint8_t stm32_protocols[PROTO_QUERIES * BYTES_PER_QUERY + 1] = {0};
uint8_t stm32_macro_slots = 0;
uint8_t stm32_macro_depth = 0;
uint8_t stm32_wake_slots = 0;

void
stm32_init(struct rc_driver *drv)
{
	drv->open = stm32_open;
	drv->close = stm32_close;
	drv->get_caps = stm32_get_caps;
	drv->prepare_buf = stm32_prepare_buf;
	drv->parse_buf = stm32_parse_buf;
	drv->read = stm32_read;
	drv->write = stm32_write;

	drv->dev.macro_slots = stm32_macro_slots;
	drv->dev.macro_depth = stm32_macro_depth;
	drv->dev.wake_slots = stm32_wake_slots;
	drv->dev.com_mat = &stm32_com_mat;
	drv->dev.protocols = stm32_protocols;
}

int
stm32_open(struct rc_device *dev, const char *path, int flags)
{
	dev->fd = open(path, flags);
	if (dev->fd == -1)
		perror("opening device failed");
	return dev->fd;
}

int
stm32_close(struct rc_device *dev)
{
	dev->fd = close(dev->fd);
	if (dev->fd == -1)
		perror("closing device failed");
	return dev->fd;
}

void
stm32_get_caps(struct rc_device *dev, uint8_t * const buf, size_t n)
{
	size_t idx;
	uint8_t i;

	/* +1 --> first query is for macro_slots, macro_depth, wake_slots */
	for(i=0; i < PROTO_QUERIES + 1; i++) {
		idx = 0;
		/* ReportID */
		buf[idx++] = 0x03;
		buf[idx++] = STAT_CMD;
		buf[idx++] = ACC_GET;
		buf[idx++] = CMD_CAPS;
		buf[idx++] = i;
		stm32_write(dev, buf, n);
		stm32_read(dev, buf, n);
		if(!i) {
			dev->macro_slots = buf[4];
			dev->macro_depth = buf[5];
			dev->wake_slots = buf[6];
			continue;
		}
		memcpy(&stm32_protocols[BYTES_PER_QUERY * (i-1)], &buf[4], n-4);
		/* stop if \0 in buffer, signifying end of protocol array */
		if (strnlen((const char *) &buf[4], n-4) < n-4)
			break;
	}
	if(args.get_caps) {
		printf("macro_slots: %u\n", dev->macro_slots);
		printf("macro_depth: %u\n", dev->macro_depth);
		printf("wake_slots: %u\n", dev->wake_slots);
		printf("supported protocols:");
		for (idx = 0; idx < strlen((char*) stm32_protocols); idx++)
			printf(" %u", stm32_protocols[idx]);
		printf("\n");
	}
}

int
stm32_parse_buf(struct rc_device *dev, const uint8_t *buf, size_t n)
{
	/* 0 --> ReportID */
	unsigned int idx = 1;
	IRMP_DATA* ir_ptr;
	(void)dev; (void)n;

	switch((enum status) buf[idx++]) {
	case STAT_CMD:
		/* we should not get a command from the device, only an answer */
		fprintf(stderr, "unexpected data received (command instead of answer)\n");
		return EXIT_FAILURE;
	case STAT_SUCCESS:
		fprintf(stderr, "command succeeded\n");
		break;
	case STAT_FAILURE:
		fprintf(stderr, "command failed\n");
		break;
	default:
		fprintf(stderr, "unexpected data received\n");
		return EXIT_FAILURE;
	}

	if (buf[idx++] != args.acc || buf[idx++] != args.cmd) {
		fprintf(stderr, "unexpected data received\n");
		return EXIT_FAILURE;
	}

	if(args.acc != ACC_GET)
		return EXIT_SUCCESS;

	switch(args.cmd) {
	case CMD_ALARM:
		printf("%u\n", *((uint32_t *) &buf[idx]));
		break;
	case CMD_MACRO:
	case CMD_WAKE:
		ir_ptr = (IRMP_DATA*) &buf[idx];
		printf("0x%02x%04x%04x%02x\n",
			ir_ptr->protocol,
			ir_ptr->address,
			ir_ptr->command,
			ir_ptr->flags);
		break;
	default:
		break;
	}
	return EXIT_SUCCESS;
}

ssize_t
stm32_prepare_buf(struct rc_device *dev, uint8_t * const buf, size_t n)
{
	size_t idx = 0, exp;
	int arg;
	IRMP_DATA ir;

	/* expected number of bytes to transmit + ReportID */
	exp = (*dev->com_mat)[args.acc][args.cmd].tx + 1;
	if(n < exp) {
		fprintf(stderr, "buffer size not sufficient\n");
		return -1;
	}

	/* ReportID */
	buf[idx++] = 0x03;
	buf[idx++] = STAT_CMD;
	buf[idx++] = args.acc;
	buf[idx++] = args.cmd;

	/* process sub argument if available*/
	if (args.sub_arg) {
		arg = atoi(args.sub_arg);
		if (arg < 1 ||
		    (args.cmd == CMD_MACRO && arg > dev->macro_slots) ||
		    (args.cmd == CMD_WAKE && arg > dev->wake_slots)) {
			fprintf(stderr, "sub argument out of range\n");
			return -1;
		}
		buf[idx++] = arg - 1;
	}

	/* process if available, this should only be possible with CMD_MACRO */
	if (args.ir) {
		arg = atoi(args.ir);
		if (arg < 0 || arg > dev->macro_depth) {
			fprintf(stderr, "ir argument out of range\n");
			return -1;
		}
		buf[idx++] = arg;
	}

	/* process set parameter if available */
	if (args.set) {
		switch((enum command) args.cmd) {
		case CMD_ALARM:
			if (sscanf(args.set, "%i", (int32_t *) &buf[idx]) != 1) {
				fprintf(stderr, "error scanning set argument\n");
				return -1;
			}
			idx += sizeof(uint32_t);
			break;
		case CMD_EMIT:
		case CMD_MACRO:
		case CMD_WAKE:
			if (sscanf(args.set, "0x%02x%04x%04x%02x",
			   (unsigned int *) &ir.protocol,
			   (unsigned int *) &ir.address,
			   (unsigned int *) &ir.command,
			   (unsigned int *) &ir.flags) != 4) {
				fprintf(stderr, "error scanning set argument, "
					"should be in hex format (e.g. 0x112233445566)\n");
				return -1;
			}
			if (!strchr((const char *) &stm32_protocols, ir.protocol)) {
				fprintf(stderr, "protocol NOT suported\n");
				return -1;
			}

			memcpy(&buf[idx], &ir, sizeof(ir));
			idx += sizeof(ir);
			break;
		default:
			break;
		}
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
	ret = read(dev->fd, buf, n);
	if (ret == -1) {
		perror("reading from device failed");
	}
	return ret;
}

ssize_t
stm32_write(struct rc_device *dev, const void *buf, size_t n)
{
	ssize_t ret;
	ret = write(dev->fd, buf, n);
	if (ret == -1)
		perror("writing to device failed");
	return ret;
}
