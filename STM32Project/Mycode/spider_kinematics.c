#include "spider_kinematics.h"
#include <math.h>

// 机械尺寸（单位：cm，后续根据你的3D打印实际尺寸修改）
#define L1  5.0f   // 大腿长度
#define L2  5.0f   // 小腿长度

void Kinematics_Init(void)
{
    // 这里可以放一些初始化参数，目前留空
}

// 单腿逆运动学计算：输入足端坐标(x, y)，输出大腿和小腿角度(0~180)
Leg_Angles_t Kinematics_Calculate(float target_x, float target_y)
{
    Leg_Angles_t result = {90, 90}; // 默认中位
    
    float cos_theta2 = (target_x*target_x + target_y*target_y - L1*L1 - L2*L2) / (2.0f * L1 * L2);
    // 限制范围，防止 acos 出现域错误
    if (cos_theta2 > 1.0f) cos_theta2 = 1.0f;
    if (cos_theta2 < -1.0f) cos_theta2 = -1.0f;
    
    float theta2 = acos(cos_theta2);                    // 小腿角度（弧度）
    float theta1 = atan2(target_y, target_x) - atan2(L2 * sin(theta2), L1 + L2 * cos(theta2)); // 大腿角度（弧度）
    
    // 将弧度转为舵机PWM对应的角度(0~180)，并做方向映射（根据你的机械安装可能加负号）
    result.hip   = (uint8_t)(theta1 * 180.0f / 3.14159f + 90.0f);
    result.knee  = (uint8_t)(theta2 * 180.0f / 3.14159f + 90.0f);
    
    return result;
}
