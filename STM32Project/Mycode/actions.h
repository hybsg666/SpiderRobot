#ifndef ACTIONS_H
#define ACTIONS_H

#include "servo.h"

// ===== 基础动作 =====
void Action_Center(void);           // 全部 90°（归中调试）
void Action_Stand(void);            // 自然站立（大腿 60°，小腿 90°）
void Action_Squat(void);            // 蹲下
void Action_Spread(void);           // 展开姿态（降低重心：大腿70°，小腿110°）

// ===== 单腿控制 =====
void Action_LiftLeg(Servo_Channel_t knee_ch, Servo_Channel_t hip_ch);
void Action_LowerLeg(Servo_Channel_t knee_ch, Servo_Channel_t hip_ch);
void Action_RaiseHand(Servo_Channel_t knee_ch, Servo_Channel_t hip_ch);  // 举手（展开→抬腿→放下）

// ===== 腿部摆动 =====
void Action_MoveLegForward(Servo_Channel_t hip_ch);
void Action_MoveLegBackward(Servo_Channel_t hip_ch);

// ===== 行走步态 =====
void Walk_Forward(int steps);
void Walk_Backward(int steps);
void Turn_Left(int steps);
void Turn_Right(int steps);

#endif
