#include <esp_task_wdt.h>
#include <Arduino.h>
#include <Wire.h>

#include "telegram_bot.h"
#include "config.h"
#include "reset_reason.h"
#include "settings.h"
#include "secret.h"
#include "read_sensor.h"
#include "alarm.h"
#include "wifi_connect.h"
#include "api_handler.h"

#define WATCHDOG_TIMEOUT_MS 60000

void setup() {
  Serial.begin(115200);
  delay(2000);

  Serial.println("Инициализация Watchdog...");
  uint32_t timeout_sec = WATCHDOG_TIMEOUT_MS / 1000;
  esp_task_wdt_init(timeout_sec, true); 
  esp_task_wdt_add(NULL);
  Serial.println("Watchdog инициализирован");
  esp_task_wdt_reset();

  if (!LittleFS.begin()) {
    Serial.println("LittleFS mount failed! Попытка форматирования...");
    LittleFS.format();
    if (!LittleFS.begin()) {
      Serial.println("Форматирование провалилось, дальше без FS");
      return;
    }
    Serial.println("LittleFS успешно отформатирован и смонтирован");
    saveDefaults();
  } else {
    Serial.println("LittleFS OK");
  }

  loadSettings();

  lastResetReason = getResetReason();
  printResetReason();

  pinMode(RELAY0_PIN, OUTPUT);
  pinMode(RELAY1_PIN, OUTPUT);
  pinMode(LIGHT0_PIN, OUTPUT);
  pinMode(LIGHT1_PIN, OUTPUT);
  
  digitalWrite(RELAY0_PIN, relay_0_status);
  digitalWrite(RELAY1_PIN, relay_1_status);
  digitalWrite(LIGHT0_PIN, light_status);
  digitalWrite(LIGHT1_PIN, light_status);
  
  Wire.begin(SDA0_PIN, SCL0_PIN);
  Wire.setTimeOut(250);
  Wire1.begin(SDA1_PIN, SCL1_PIN);
  Wire1.setTimeOut(250);

  if (!sensor0.begin(&Wire)) {
    Serial.println("Ошибка инициализации HTU21DF #0!");
  } else {
    Serial.println("HTU21DF #0 инициализирован");
  }
  if (!sensor1.begin(&Wire1)) {
    Serial.println("Ошибка инициализации HTU21DF #1!");
  } else {
    Serial.println("HTU21DF #1 инициализирован");
  }

  connectToWiFi();
  
  bot.onUpdate(handleUpdate);
}

void loop() {
  esp_task_wdt_reset();
  
  safe_sensor_read();
  
  static unsigned long last_check = 0;
  if (millis() - last_check > 30000) {
    checkSensors();
    last_check = millis();
  }
  
  checkHeating();

  static unsigned long lastWifiCheck = 0;
  if (millis() - lastWifiCheck > 10000) {
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("Переподключение к WiFi...");
      WiFi.disconnect();
      WiFi.reconnect();
    }
    lastWifiCheck = millis();
  }
  
  bot.tick();
  
  delay(10);
}