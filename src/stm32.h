#ifndef STM32_H
#define STM32_H

void stm32_init(struct rc_device *dev);
int stm32_open(struct rc_device *dev, const char *path, int flags);
void stm32_close(struct rc_device *dev);
size_t stm32_prepare_buf(struct rc_device *dev, uint8_t * const buf, size_t n);

#endif /* STM32_H */
