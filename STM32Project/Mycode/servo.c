#include "servo.h"
#include "pca9685.h"

// 声明 CubeMX 生成的 I2C2 句柄
extern I2C_HandleTypeDef hi2c2;

// ========== 校准参数数组（完全保留你的数据）==========
static Servo_Calib_t servo_calib[SERVO_MAX] = {
    // ===== 小腿 (通道 0~3) =====
    [SERVO_KNEE_LF] = {.pulse_min = 500, .pulse_max = 2500, .direction =  1},
    [SERVO_KNEE_RF] = {.pulse_min = 300, .pulse_max = 2300, .direction = -1},
    [SERVO_KNEE_RB] = {.pulse_min = 600, .pulse_max = 2600, .direction =  1},
    [SERVO_KNEE_LB] = {.pulse_min = 300, .pulse_max = 2300, .direction = -1},

    // ===== 大腿 (通道 4~7) =====
    [SERVO_HIP_LF] = {.pulse_min = 500, .pulse_max = 2500, .direction = -1},
    [SERVO_HIP_RF] = {.pulse_min = 500, .pulse_max = 2500, .direction =  1},
    [SERVO_HIP_RB] = {.pulse_min = 500, .pulse_max = 2500, .direction = -1},
    [SERVO_HIP_LB] = {.pulse_min = 500, .pulse_max = 2500, .direction =  1},
};

const Servo_Calib_t* Servo_GetCalib(Servo_Channel_t ch)
{
    if (ch >= SERVO_MAX) return NULL;
    return &servo_calib[ch];
}

void Servo_Init(void)
{
    PCA9685_Init(&hi2c2, PCA9685_I2C_ADDR_DEFAULT, 50.0f);
    HAL_Delay(10);

    for (int i = 0; i < 4; i++) {
        Servo_SetAngle((Servo_Channel_t)i, 90);
    }
    for (int i = 4; i < 8; i++) {
        Servo_SetAngle((Servo_Channel_t)i, 60);
    }
    HAL_Delay(100);
}

void Servo_SetAngle(Servo_Channel_t ch, uint8_t angle)
{
    if (ch >= SERVO_MAX) return;
    if (angle > 180) angle = 180;

    Servo_Calib_t *calib = &servo_calib[ch];

    uint16_t pulse;
    if (calib->direction == 1) {
        pulse = calib->pulse_min + (calib->pulse_max - calib->pulse_min) * angle / 180;
    } else {
        pulse = calib->pulse_max - (calib->pulse_max - calib->pulse_min) * angle / 180;
    }

    // ===== 恢复你原来的限制范围（500~2500）=====
    if (pulse < 500) pulse = 500;
    if (pulse > 2500) pulse = 2500;

    uint16_t off_time = (uint16_t)((uint32_t)pulse * 4096 / 20000);
    PCA9685_SetChannelPWM((uint8_t)ch, 0, off_time);
}

void Servo_Calibrate(Servo_Channel_t ch, uint16_t pulse_min, uint16_t pulse_max, int8_t direction)
{
    if (ch >= SERVO_MAX) return;
    servo_calib[ch].pulse_min = pulse_min;
    servo_calib[ch].pulse_max = pulse_max;
    servo_calib[ch].direction = direction;
}
