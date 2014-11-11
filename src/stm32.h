#ifndef STM32_H
#define STM32_H

void stm32_init(struct rc_device *dev);
int stm32_open(struct rc_device *dev, const char *path, int flags);
int stm32_close(struct rc_device *dev);
int stm32_parse_buf(struct rc_device *dev, const uint8_t *buf, size_t n);
ssize_t stm32_prepare_buf(struct rc_device *dev, uint8_t * const buf, size_t n);
ssize_t stm32_read(struct rc_device *dev, void *buf, size_t n);
ssize_t stm32_write(struct rc_device *dev, const void *buf, size_t n);

#endif /* STM32_H */
