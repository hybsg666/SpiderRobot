#ifndef __SPIDER_KINEMATICS_H
#define __SPIDER_KINEMATICS_H

#include <stdint.h>

// 腿编号（后续扩展）
typedef enum {
    LEG_RF = 0,
    LEG_LF,
    LEG_RB,
    LEG_LB
} Leg_ID_t;

// 大腿+小腿角度结构体
typedef struct {
    uint8_t hip;   // 大腿角度 0~180
    uint8_t knee;  // 小腿角度 0~180
} Leg_Angles_t;

void Kinematics_Init(void);
Leg_Angles_t Kinematics_Calculate(float target_x, float target_y);

#endif
