#include <WiFi.h>
#include "bsp_wifi.h"

extern const char* ssid;
extern const char* password;

void bsp_wifi_init(void) {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid, password);
    Serial0.println("\n✅ AP 热点已开启");
    Serial0.print("📶 热点: ");
    Serial0.println(ssid);
    Serial0.print("🔑 密码: ");
    Serial0.println(password);
    Serial0.print("🌐 请访问: http://");
    Serial0.println(WiFi.softAPIP());
}
