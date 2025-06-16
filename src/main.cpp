#include <Arduino.h>
#include <WiFi.h>

// Подключаем наши учетные данные
#include "wifi_data.h"

void connectToWiFi() {
  Serial.println("Подключение к Wi-Fi...");
  
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  
  Serial.print("SSID: ");
  Serial.println(WIFI_SSID);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    attempts++;
    
    if (attempts > 30) { // 15 секунд
      Serial.println("\nОшибка подключения! Перезагрузка...");
      delay(2000);
      ESP.restart();
    }
  }
  
  Serial.println("\nУспешное подключение!");
  Serial.print("IP адрес: ");
  Serial.println(WiFi.localIP());
}

void setup() {
  Serial.begin(115200);
  delay(2000); // Даем время для инициализации
  
  connectToWiFi();
}

void loop() {
  // Проверка соединения каждые 10 секунд
  static unsigned long lastCheck = 0;
  
  if (millis() - lastCheck > 10000) {
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("Wi-Fi подключение активно");
    } else {
      Serial.println("Потеряно соединение Wi-Fi! Переподключение...");
      WiFi.reconnect();
    }
    lastCheck = millis();
  }
  
  delay(1000);
}