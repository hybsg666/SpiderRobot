#include "bsp_uart.h"

#include <Arduino.h>

#define SU03T_RX_PIN 18
#define SU03T_TX_PIN 17

void bsp_uart_init(void) {
    Serial1.begin(9600, SERIAL_8N1, SU03T_RX_PIN, SU03T_TX_PIN);
    Serial0.println("SU-03T 串口已初始化 (UART1, 9600)");
}
