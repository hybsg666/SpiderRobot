#ifndef BSP_CAN_H
#define BSP_CAN_H

#include <Arduino.h>
#include <mcp2515.h>

extern MCP2515 mcp2515;

void bsp_can_init(void);
void bsp_can_send(byte command, const char* source);

#endif
