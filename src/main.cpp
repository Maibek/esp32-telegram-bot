#include <Arduino.h>
#include <Wire.h>

#include "telegram_bot.h"
#include "config.h"
#include "wifi_connect.h"
#include "read_sensor.h"
#include "alarm.h"


void setup() {
  Serial.begin(115200);
  delay(2000);

  pinMode(RELAY0_PIN, OUTPUT);
  pinMode(RELAY1_PIN, OUTPUT);
  pinMode(LIGHT0_PIN, OUTPUT);
  pinMode(LIGHT1_PIN, OUTPUT);

  digitalWrite(RELAY0_PIN, HIGH);
  digitalWrite(RELAY1_PIN, HIGH);

  digitalWrite(LIGHT0_PIN, LOW);
  digitalWrite(LIGHT1_PIN, LOW);
  
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
  safe_sensor_read(); // Безопасное чтение датчика
  static unsigned long last_check = 0;
  if (millis() - last_check > 30000) { // Проверка каждые 30 секунд
    checkSensors();
    last_check = millis();
  }
  alarm_high_temp();
  alarm_low_temp();

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

  // Обработка сообщений Telegram. В будущем лучше реализовать webhook. Так как иногда бот не сбрасывает значения сообщений и начинает спамить в ответ на запрос.
  if (millis() - bot_lasttime > BOT_MTBS) {
    Serial.println("Проверка сообщений Telegram...");
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    if (numNewMessages > 0) {
      Serial.println("Найдено сообщений: " + String(numNewMessages));
      handleNewMessages(numNewMessages);
    }
    
    // Нужно для отладки. После окончания проекта можно будет удалить.
    bot_lasttime = millis();
    Serial.println("Свободная память: " + String(ESP.getFreeHeap()));
  }  
}