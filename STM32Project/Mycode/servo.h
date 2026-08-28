#ifndef __SERVO_H
#define __SERVO_H

#include "stm32f1xx_hal.h"
#include <stdint.h>

// ========== 舵机通道映射到 PCA9685 (0~15) ==========
// 根据你的实际接线修改等号右边的数字
// servo.h
typedef enum {
    SERVO_KNEE_LF = 0,   // 左前腿 小腿
    SERVO_KNEE_RF = 1,   // 右前腿 小腿
    SERVO_KNEE_RB = 2,   // 右后腿 小腿
    SERVO_KNEE_LB = 3,   // 左后腿 小腿
    SERVO_HIP_LF = 4,    // 左前腿 大腿
    SERVO_HIP_RF = 5,    // 右前腿 大腿
    SERVO_HIP_RB = 6,    // 右后腿 大腿
    SERVO_HIP_LB = 7,    // 左后腿 大腿
    SERVO_MAX
} Servo_Channel_t;

// ========== 舵机校准参数结构体 ==========
typedef struct {
    uint16_t pulse_min;     // 0° 对应的脉宽 (us)
    uint16_t pulse_max;     // 180° 对应的脉宽 (us)
    int8_t direction;       // 1: 正向, -1: 反向
} Servo_Calib_t;

// ========== 函数声明 ==========
void Servo_Init(void);
void Servo_SetAngle(Servo_Channel_t ch, uint8_t angle);
void Servo_Calibrate(Servo_Channel_t ch, uint16_t pulse_min, uint16_t pulse_max, int8_t direction);

// ========== 获取校准参数（供调试使用） ==========
const Servo_Calib_t* Servo_GetCalib(Servo_Channel_t ch);

#endif
