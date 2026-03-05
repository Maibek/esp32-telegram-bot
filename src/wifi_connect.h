#pragma once
#include <WiFi.h>
#include "config.h"
#include "secret.h"

void connectToWiFi() {
    Serial.println("Подключение к Wi-Fi...");
    WiFi.disconnect(true);
    delay(1000);
    WiFi.begin(WIFI_SSID_1, WIFI_PASS_1);
    WiFi.setSleep(false);
  
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        attempts++;
    
        if (attempts > 30) {
            Serial.println("\nПревышено ожидание. Перезапуск...");
            delay(2000);
            ESP.restart();
        }
    }
  
    Serial.println("\nУспешное подключение к точке доступа ");
    Serial.println(WIFI_SSID_1);
    Serial.print("IP адрес: ");
    Serial.println(WiFi.localIP());
}