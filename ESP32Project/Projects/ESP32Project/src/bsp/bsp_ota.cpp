#include <ArduinoOTA.h>
#include "bsp_ota.h"

void bsp_ota_init(void) {
    ArduinoOTA.setHostname("SpiderRobot");
    ArduinoOTA.setPassword("12345678");

    ArduinoOTA.onStart([]() {
        Serial0.println("OTA 开始更新...");
    });
    ArduinoOTA.onEnd([]() {
        Serial0.println("\nOTA 更新完成！");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial0.printf("OTA 进度: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
        Serial0.printf("OTA 错误[%u]: ", error);
        if (error == OTA_AUTH_ERROR) Serial0.println("身份验证失败");
        else if (error == OTA_BEGIN_ERROR) Serial0.println("开始失败");
        else if (error == OTA_CONNECT_ERROR) Serial0.println("连接失败");
        else if (error == OTA_RECEIVE_ERROR) Serial0.println("接收失败");
        else if (error == OTA_END_ERROR) Serial0.println("结束失败");
    });

    ArduinoOTA.begin();
    Serial0.println("✅ OTA 服务已启动 (主机名: SpiderRobot, 密码: 12345678)");
}

void bsp_ota_handle(void) {
    ArduinoOTA.handle();
}
