#include "app_voice.h"
#include "bsp/bsp_can.h"
#include "protocol/voice_protocol.h"

// ===== 外部函数声明：通知 main.cpp 有活动（语音/网页） =====
extern void updateActivity();

enum ParseState {
    WAIT_HEADER1,
    WAIT_HEADER2,
    WAIT_DATA,
    WAIT_FOOTER1,
    WAIT_FOOTER2
};

static ParseState state = WAIT_HEADER1;
static byte rxBuffer[FRAME_LENGTH];
static int bufferIndex = 0;

void onFrameReceived(byte frame[]);
void processByte(byte c) {
    // ===== 【新增】只要收到任何一个字节，就重置计时器 =====
    updateActivity();

    switch (state) {
        case WAIT_HEADER1:
            if (c == FRAME_HEADER1) {
                rxBuffer[0] = c;
                bufferIndex = 1;
                state = WAIT_HEADER2;
            }
            break;
        case WAIT_HEADER2:
            if (c == FRAME_HEADER2) {
                rxBuffer[1] = c;
                bufferIndex = 2;
                state = WAIT_DATA;
            } else {
                state = WAIT_HEADER1;
            }
            break;
        case WAIT_DATA:
            rxBuffer[2] = c;
            bufferIndex = 3;
            state = WAIT_FOOTER1;
            break;
        case WAIT_FOOTER1:
            if (c == FRAME_FOOTER1) {
                rxBuffer[3] = c;
                bufferIndex = 4;
                state = WAIT_FOOTER2;
            } else {
                state = WAIT_HEADER1;
                if (c == FRAME_HEADER1) {
                    rxBuffer[0] = c;
                    bufferIndex = 1;
                    state = WAIT_HEADER2;
                }
            }
            break;
        case WAIT_FOOTER2:
            if (c == FRAME_FOOTER2) {
                rxBuffer[4] = c;
                onFrameReceived(rxBuffer);
            }
            state = WAIT_HEADER1;
            break;
    }
}

void onFrameReceived(byte frame[]) {
    byte command = frame[2];
    bsp_can_send(command, "🎤 语音");
}

void app_voice_handle(void) {
    while (Serial1.available()) {
        byte c = Serial1.read();
        processByte(c);
    }
}
