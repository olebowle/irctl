#ifndef CAPABILITIES_H
#define CAPABILITIES_H

#include <inttypes.h>
#include <sys/types.h>
#include "irmpprotocols.h"

/*
#define DEBUG_PRINT(fmt, ...) \
	do { if (DEBUG) fprintf(stderr, fmt, __VA_ARGS__); } while (0)
*/
#define ACCESS_COUNT 3
enum __attribute__ ((__packed__)) access {
	ACC_GET,
	ACC_SET,
	ACC_RESET
};
#define COMMAND_COUNT 7
enum __attribute__ ((__packed__)) command {
	CMD_EMIT,
	CMD_CAPS,
	CMD_FW,
	CMD_ALARM,
	CMD_TRIG_CMD,
	CMD_TRIG_IR,
	CMD_WAKE
};

struct global_args {
	char *drv_name, *emit, *fw, *path, *set, *sub_arg;
	unsigned int main_cmd, sub_cmd;
	enum access acc;
	enum command cmd;
};

struct com_ssize {
	ssize_t tx;
	ssize_t rx;
};

struct rc_device {
	int fd;
	const uint8_t *trig_slots;
	const uint8_t *wake_slots;

	const struct com_ssize (*com_mat)[ACCESS_COUNT][COMMAND_COUNT];
	const uint8_t *rx_protocols;
	const uint8_t *tx_protocols;
};

struct rc_driver {
	struct rc_device dev;

	void (*init) (struct rc_device *dev);
	int (*open) (struct rc_device *dev, const char *path, int flags);
	void (*close) (struct rc_device *dev);

	int (*read) (struct rc_device *dev, const void *buf, size_t n);
	ssize_t (*write) (struct rc_device *dev, const void *buf, size_t n);

	size_t (*parse_buf) (struct rc_device *dev, uint8_t * const buf, size_t n);
	ssize_t (*prepare_buf) (struct rc_device *dev, uint8_t * const buf, size_t n);
};

#endif /* CAPABILITIES_H */
