#ifndef CAN_PROTOCOL_H
#define CAN_PROTOCOL_H

#include <Arduino.h>

// CAN 通信协议
#define CAN_ID_COMMAND  0x100   // 指令帧 ID
#define CAN_DLC         1       // 数据长度 1 字节

// 指令码映射
#define CMD_STAND       0x01
#define CMD_SQUAT       0x02
#define CMD_WAVE        0x03
#define CMD_FORWARD     0x10
#define CMD_BACKWARD    0x11
#define CMD_LEFT        0x12
#define CMD_RIGHT       0x13
#define CMD_STOP        0x00

#endif
