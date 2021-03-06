#ifndef STM32_H
#define STM32_H

#include "irmpprotocols.h"

#define HID_IN_BUFFER_SIZE	17
/* first 4 bytes: ReportID STAT_CMD ACC_GET CMD_CAPS
 * not useable to transmit information */
#define BYTES_PER_QUERY		(HID_IN_BUFFER_SIZE - 4)
/* +1 --> ceil(int_dev) */
#define PROTO_QUERIES		(IRMP_N_PROTOCOLS / BYTES_PER_QUERY + 1)

void stm32_init(struct rc_driver *drv);
int stm32_open(struct rc_device *dev, const char *path, int flags);
int stm32_close(struct rc_device *dev);
void stm32_get_caps(struct rc_device *dev, uint8_t * const buf, size_t n);
int stm32_parse_buf(struct rc_device *dev, const uint8_t *buf, size_t n);
ssize_t stm32_prepare_buf(struct rc_device *dev, uint8_t * const buf, size_t n);
ssize_t stm32_read(struct rc_device *dev, void *buf, size_t n);
ssize_t stm32_write(struct rc_device *dev, const void *buf, size_t n);

#endif /* STM32_H */
