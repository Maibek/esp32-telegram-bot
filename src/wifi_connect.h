#pragma once
#include <WiFi.h>
#include <WiFiClientSecure.h>

#include "config.h"
#include "wifi_data.h"
#include "telegram_bot.h"



void connectToWiFi() {
    Serial.println("Подключение к Wi-Fi...");
    WiFi.disconnect(true);
    delay(1000);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    WiFi.setSleep(false); // Отключение энергосбережения WiFi
  
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
    Serial.println(WIFI_SSID);
    Serial.print("IP адрес: ");
    Serial.println(WiFi.localIP());

    // Синхронизация времени для SSL. Временное решение, без него не всегда бот получает сообщения. Необходимо подумать. Иногда синхронизация не проходит, возможно сделать асинхронную синхронизацию.
    configTime(0, 0, "pool.ntp.org");
    Serial.println("Ожидание синхронизации времени...");
    time_t now = time(nullptr);
    while (now < 24 * 3600) {
        delay(500);
        Serial.print(".");
        now = time(nullptr);
    }
    Serial.println("\nВремя синхронизировано!");
}