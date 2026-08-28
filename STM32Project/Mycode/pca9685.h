#ifndef PCA9685_H
#define PCA9685_H

#include "stm32f1xx_hal.h"
#include <stdint.h>

// PCA9685 默认 I2C 地址 (7位)
#define PCA9685_I2C_ADDR_DEFAULT  0x40

// 寄存器地址
#define PCA9685_MODE1             0x00
#define PCA9685_MODE2             0x01
#define PCA9685_PRE_SCALE         0xFE
#define PCA9685_LED0_ON_L         0x06
#define PCA9685_LED0_ON_H         0x07
#define PCA9685_LED0_OFF_L        0x08
#define PCA9685_LED0_OFF_H        0x09

// 函数声明
void PCA9685_Init(I2C_HandleTypeDef *hi2c, uint8_t addr, float freq);
void PCA9685_SetChannelPWM(uint8_t channel, uint16_t on_time, uint16_t off_time);
void PCA9685_SetServoAngle(uint8_t channel, uint8_t angle);
void PCA9685_SetAllPWM(uint16_t on_time, uint16_t off_time);
void PCA9685_Reset(void);

#endif
