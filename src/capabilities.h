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
#define COMMAND_COUNT 6
enum __attribute__ ((__packed__)) command {
	CMD_EMIT,
	CMD_CAPS,
	CMD_FW,
	CMD_ALARM,
	CMD_MACRO,
	CMD_WAKE
};

enum __attribute__ ((__packed__)) status {
	STAT_CMD,
	STAT_SUCCESS,
	STAT_FAILURE
};

struct global_args {
	char *drv_name, *fw, *ir, *path, *set, *sub_arg;
	uint8_t main_cmd, sub_cmd, get_caps;
	enum access acc;
	enum command cmd;
};

struct com_ssize {
	ssize_t tx;
	ssize_t rx;
};

struct rc_device {
	int fd;
	uint8_t macro_slots;
	uint8_t macro_depth;
	uint8_t wake_slots;

	const struct com_ssize (*com_mat)[ACCESS_COUNT][COMMAND_COUNT];
	uint8_t *protocols;
};

struct rc_driver {
	struct rc_device dev;

	void (*init) (struct rc_driver *drv);
	int (*open) (struct rc_device *dev, const char *path, int flags);
	int (*close) (struct rc_device *dev);

	ssize_t (*read) (struct rc_device *dev, void *buf, size_t n);
	ssize_t (*write) (struct rc_device *dev, const void *buf, size_t n);

	void (*get_caps) (struct rc_device *dev, uint8_t * const buf, size_t n);
	int (*parse_buf) (struct rc_device *dev, const uint8_t *buf, size_t n);
	ssize_t (*prepare_buf) (struct rc_device *dev, uint8_t * const buf, size_t n);
};

#endif /* CAPABILITIES_H */
