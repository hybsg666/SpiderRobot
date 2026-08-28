#include "pca9685.h"
#include <math.h>

static I2C_HandleTypeDef *pca_i2c = NULL;
static uint8_t pca_addr = 0;

// ========== 私有函数：写寄存器 ==========
static void PCA9685_WriteReg(uint8_t reg, uint8_t data)
{
    HAL_I2C_Mem_Write(pca_i2c, pca_addr, reg, I2C_MEMADD_SIZE_8BIT, &data, 1, HAL_MAX_DELAY);
}

// ========== 私有函数：读寄存器 ==========
static uint8_t PCA9685_ReadReg(uint8_t reg)
{
    uint8_t data;
    HAL_I2C_Mem_Read(pca_i2c, pca_addr, reg, I2C_MEMADD_SIZE_8BIT, &data, 1, HAL_MAX_DELAY);
    return data;
}

// ========== 初始化 PCA9685 ==========
void PCA9685_Init(I2C_HandleTypeDef *hi2c, uint8_t addr, float freq)
{
    pca_i2c = hi2c;
    pca_addr = (addr << 1);  // 7位地址左移1位变为8位地址

    // 1. 复位芯片 (MODE1 写 0x01)
    PCA9685_WriteReg(PCA9685_MODE1, 0x01);
    HAL_Delay(10);

    // 2. 读取当前 MODE1
    uint8_t oldmode = PCA9685_ReadReg(PCA9685_MODE1);

    // 3. 进入休眠模式 (设置 SLEEP 位)
    uint8_t newmode = (oldmode & 0x7F) | 0x10;
    PCA9685_WriteReg(PCA9685_MODE1, newmode);

    // 4. 计算预分频器: prescale = round(25MHz / (4096 * freq)) - 1
    float prescaleval = 25000000.0f;  // 内部振荡器 25MHz
    prescaleval /= 4096.0f;
    prescaleval /= freq;
    prescaleval -= 1.0f;
    uint8_t prescale = (uint8_t)floor(prescaleval + 0.5f);

    PCA9685_WriteReg(PCA9685_PRE_SCALE, prescale);

    // 5. 退出休眠模式 (清除 SLEEP 位)，启用自动递增
    PCA9685_WriteReg(PCA9685_MODE1, oldmode | 0x01 | 0x20);
    HAL_Delay(5);

    // 6. 设置 MODE2: 输出驱动模式 (totem pole)
    PCA9685_WriteReg(PCA9685_MODE2, 0x04);  // 0x04 = OUTDRV
    HAL_Delay(5);
}

// ========== 设置指定通道的 PWM ==========
void PCA9685_SetChannelPWM(uint8_t channel, uint16_t on_time, uint16_t off_time)
{
    if (channel > 15) return;

    uint8_t reg = PCA9685_LED0_ON_L + 4 * channel;
    uint8_t buf[4];

    buf[0] = on_time & 0xFF;
    buf[1] = (on_time >> 8) & 0xFF;
    buf[2] = off_time & 0xFF;
    buf[3] = (off_time >> 8) & 0xFF;

    HAL_I2C_Mem_Write(pca_i2c, pca_addr, reg, I2C_MEMADD_SIZE_8BIT, buf, 4, HAL_MAX_DELAY);
}

// ========== 便捷函数：设置舵机角度 (0~180°) ==========
void PCA9685_SetServoAngle(uint8_t channel, uint8_t angle)
{
    if (angle > 180) angle = 180;

    // 脉宽映射: 0°→0.5ms, 180°→2.5ms
    // 50Hz: 周期20ms = 20000us
    // 12位分辨率: 4096 对应 20000us
    // 所以 1us = 4096/20000 = 0.2048
    uint16_t pulse_us = 500 + (uint16_t)((float)angle * 2000.0f / 180.0f);
    uint16_t off_time = (uint16_t)((uint32_t)pulse_us * 4096 / 20000);

    // on_time 设为 0，从周期起始点开始
    PCA9685_SetChannelPWM(channel, 0, off_time);
}

// ========== 设置所有通道的 PWM ==========
void PCA9685_SetAllPWM(uint16_t on_time, uint16_t off_time)
{
    uint8_t buf[4];
    buf[0] = on_time & 0xFF;
    buf[1] = (on_time >> 8) & 0xFF;
    buf[2] = off_time & 0xFF;
    buf[3] = (off_time >> 8) & 0xFF;

    HAL_I2C_Mem_Write(pca_i2c, pca_addr, 0xFA, I2C_MEMADD_SIZE_8BIT, buf, 4, HAL_MAX_DELAY);
}

// ========== 复位 PCA9685 ==========
void PCA9685_Reset(void)
{
    PCA9685_WriteReg(PCA9685_MODE1, 0x01);
    HAL_Delay(10);
}
