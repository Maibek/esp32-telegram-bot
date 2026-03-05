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
  
  // Простейшая инициализация watchdog
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
  } 
  else {
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
  
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT);

  Wire.begin(SDA0_PIN, SCL0_PIN);
  Wire.setTimeOut(250);
  Wire1.begin(SDA1_PIN, SCL1_PIN);
  Wire1.setTimeOut(250);

  // Инициализация датчика 0 на первой шине
  if (!sensor0.begin(&Wire)) {
    Serial.println("Ошибка инициализации HTU21DF #0!");
  } 
  else {
    Serial.println("HTU21DF #0 инициализирован");
  }
  // Инициализация датчика 1 на второй шине
  if (!sensor1.begin(&Wire1)) {
    Serial.println("Ошибка инициализации HTU21DF #1!");
  } else {
    Serial.println("HTU21DF #1 инициализирован");
  }

  connectToWiFi();
}

void loop() {
  esp_task_wdt_reset();
  safe_sensor_read(); // Безопасное чтение датчика
  static unsigned long last_check = 0;
  if (millis() - last_check > 30000) { // Проверка каждые 30 секунд
    checkSensors();
    last_check = millis();
  }
  checkHeating();  
  alarm_high_temp();
  alarm_low_temp();
  alarm_high_hum();
  alarm_low_hum();

  // Проверка соединения WiFi
  static unsigned long lastCheck = 0;
  if (millis() - lastCheck > 10000) {
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("Переподключение к WiFi...");
      WiFi.disconnect();
      WiFi.reconnect();
    }
    lastCheck = millis();
  }
  esp_task_wdt_reset();
  if (millis() - bot_lasttime > BOT_MTBS) {
    Serial.println("Проверка новых сообщений");
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    Serial.println("Сообщений: " + String(numNewMessages));
    
    if (millis() - bot_lasttime > BOT_MTBS) {
      Serial.println("Проверка новых сообщений");
      
      int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
      Serial.println("Сообщений: " + String(numNewMessages));
      
      if (numNewMessages > 0) {
        // Диагностика
        Serial.println("=== ДИАГНОСТИКА СООБЩЕНИЙ ===");
        for (int i = 0; i < numNewMessages; i++) {
            Serial.println("Сообщение " + String(i) + ": ID=" + String(bot.messages[i].update_id));
        }
        
        // Сохраняем ID последнего сообщения ПРАВИЛЬНО
        long lastUpdateId = bot.messages[numNewMessages - 1].update_id;
        Serial.println("Последний ID: " + String(lastUpdateId));
        
        // Обрабатываем
        handleNewMessages(numNewMessages);
        
        // Обновляем указатель
        bot.last_message_received = lastUpdateId;
        
        // Проверка
        Serial.println("Новый last_message_received: " + String(bot.last_message_received));
        Serial.println("================================");
      }
    }
    
    bot_lasttime = millis();
    Serial.println("Обработка сообщений закончина"); 
  }  
}