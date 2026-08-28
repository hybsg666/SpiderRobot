#include <SPI.h>
#include <mcp2515.h>
#include "bsp_can.h"

#define CAN_CS_PIN   5
#define CAN_SCK_PIN  12
#define CAN_MISO_PIN 13
#define CAN_MOSI_PIN 11

MCP2515 mcp2515(CAN_CS_PIN);

void bsp_can_init(void) {
    SPI.begin(CAN_SCK_PIN, CAN_MISO_PIN, CAN_MOSI_PIN, CAN_CS_PIN);
    mcp2515.reset();
    if (mcp2515.setBitrate(CAN_500KBPS, MCP_8MHZ) == MCP2515::ERROR_OK) {
        mcp2515.setNormalMode();
        Serial0.println("MCP2515 初始化成功 (500kbps)");
    } else {
        Serial0.println("MCP2515 初始化失败！");
        Serial0.println("  -> SPI 读不到芯片：先查接线/供电/共地，WiFi+语音仍继续运行");
    }
}

void bsp_can_send(byte command, const char* source) {
    struct can_frame frame;
    frame.can_id = 0x100;
    frame.can_dlc = 1;
    frame.data[0] = command;
    if (mcp2515.sendMessage(&frame) == MCP2515::ERROR_OK) {
        Serial0.print(source);
        Serial0.print(" CAN 发送成功，指令: 0x");
        if (command < 0x10) Serial0.print("0");
        Serial0.println(command, HEX);
    } else {
        Serial0.print(source);
        Serial0.println(" CAN 发送失败！");
    }
}
