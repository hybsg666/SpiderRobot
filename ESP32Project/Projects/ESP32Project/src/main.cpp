#include <Arduino.h>
#include <WiFi.h>                // ← 必须加！
#include <esp_task_wdt.h>
#include <esp_sleep.h>
#include <driver/uart.h>

// ===== WiFi 配置 =====
const char* ssid = "SpiderRobot";
const char* password = "12345678";

// ===== 低功耗配置 =====
#define IDLE_TIMEOUT_MS   (60 * 1000)   // 60 秒无语音数据则休眠

// ===== 引入各模块头文件 =====
#include "bsp/bsp_uart.h"
#include "bsp/bsp_can.h"
#include "bsp/bsp_wifi.h"
#include "bsp/bsp_ota.h"
#include "app/app_web.h"
#include "app/app_voice.h"

// ===== 全局变量 =====
unsigned long lastActivityTime = 0;
bool isLightSleeping = false;

// ============================================================
//  更新最后活动时间（语音或网页任一有指令即刷新）
// ============================================================
void updateActivity() {
    lastActivityTime = millis();
}

// ============================================================
//  重新初始化 WiFi（唤醒后调用）
// ============================================================
void reinitWiFi() {
    Serial0.println("📶 重新初始化 WiFi...");
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid, password);
    Serial0.print("📶 热点: ");
    Serial0.println(ssid);
    Serial0.print("🔑 密码: ");
    Serial0.println(password);
    Serial0.print("🌐 请访问: http://");
    Serial0.println(WiFi.softAPIP());
}

// ============================================================
//  进入轻度睡眠
// ============================================================
void enterLightSleep() {
    if (isLightSleeping) return;
    isLightSleeping = true;

    Serial0.println("💤 60秒无数据，进入轻度睡眠...");
    Serial0.println("📢 说唤醒指令即可恢复");
    delay(100);

    // ===== 1. 暂停看门狗 =====
    esp_task_wdt_deinit();

    // ===== 2. 配置唤醒源：GPIO18 低电平唤醒 =====
    // EXT0 是电平触发：0=低电平唤醒。
    // UART 空闲时 SU-03T 的 TX 保持高电平 → 不会误唤醒；
    // 起始位是低电平 → 有数据来了才唤醒。
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_18, 0);

    // ===== 3. 进入轻度睡眠 =====
    esp_light_sleep_start();

    // ===== 4. 唤醒后恢复 =====
    // 打印唤醒原因（2=EXT0 低电平唤醒，5=UART，4=定时器）便于定位
    Serial0.print("🔊 轻度睡眠唤醒！原因代码: ");
    Serial0.println((int)esp_sleep_get_wakeup_cause());

    // 重新初始化看门狗
    esp_task_wdt_init(10, true);
    esp_task_wdt_add(NULL);
    Serial0.println("✅ 看门狗已重新启用");

    // 重新初始化 UART：先 end 再 begin，确保驱动干净重建
    Serial1.end();
    delay(10);
    bsp_uart_init();

    // 重新初始化 WiFi 和 Web 服务器
    reinitWiFi();
    app_web_init();

    lastActivityTime = millis();
    isLightSleeping = false;
}

// ============================================================
//  Setup
// ============================================================
void setup() {
    Serial0.begin(115200);
    delay(100);

    // ===== 打印唤醒原因 =====
    esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
    if (cause == ESP_SLEEP_WAKEUP_EXT0) {
        Serial0.println("🔊 GPIO 唤醒");
    } else if (cause == ESP_SLEEP_WAKEUP_TIMER) {
        Serial0.println("⏰ 定时器唤醒");
    } else {
        Serial0.println("🔌 正常启动");
    }

    // ===== 打印复位原因 =====
    esp_reset_reason_t reason = esp_reset_reason();
    Serial0.print("🔄 复位原因代码: ");
    Serial0.println(reason);
    if (reason == ESP_RST_WDT) {
        Serial0.println("⚠️ 上次复位是看门狗触发！");
    } else if (reason == ESP_RST_POWERON) {
        Serial0.println("✅ 正常上电启动");
    } else if (reason == ESP_RST_SW) {
        Serial0.println("🔄 软件复位");
    } else {
        Serial0.println("其他复位原因");
    }

    Serial0.println("🕷️ 蜘蛛机器人启动...");

    // ===== 看门狗 =====
    esp_task_wdt_init(10, true);
    esp_task_wdt_add(NULL);
    Serial0.println("✅ 硬件看门狗已启用 (10s)");

    // ===== 初始化各模块 =====
    bsp_uart_init();
    bsp_can_init();
    bsp_wifi_init();
    bsp_ota_init();
    app_web_init();

    Serial0.println("📢 语音控制已就绪，请对 SU-03T 说话...");
    Serial0.println("🕐 60秒无语音数据将自动休眠");

    lastActivityTime = millis();
    isLightSleeping = false;
}

// ============================================================
//  Loop
// ============================================================
void loop() {
    esp_task_wdt_reset();   // 喂狗

    bsp_ota_handle();
    app_web_handle();
    app_voice_handle();

    if (millis() - lastActivityTime > IDLE_TIMEOUT_MS) {
        enterLightSleep();
    }

    delay(10);
}

// OTA命令行：pio run -e ota -t upload
