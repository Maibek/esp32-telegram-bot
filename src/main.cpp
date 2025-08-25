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

    digitalWrite(RELAY0_PIN, relay0_status);
    digitalWrite(RELAY1_PIN, relay1_status);
    digitalWrite(LIGHT0_PIN, light_status);
    digitalWrite(LIGHT1_PIN, light_status);
    
    client.setCACert(TELEGRAM_CERTIFICATE_ROOT);

    Wire.begin(SDA0_PIN, SCL0_PIN);
    Wire.setTimeOut(250);
    Wire1.begin(SDA1_PIN, SCL1_PIN);
    Wire1.setTimeOut(250);

    // Инициализация датчиков
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

    // Подключение к WiFi
    connectToWiFi();

    // Настройка OTA после подключения к WiFi
    setupOTA();
}

void loop() {
    safe_sensor_read();
    
    static unsigned long last_check = 0;
    if (millis() - last_check > 30000) {
        checkSensors();
        last_check = millis();
    }
    
    checkHeating();  
    alarm_high_temp();
    alarm_low_temp();
    alarm_high_hum();
    alarm_low_hum();

    static unsigned long lastStatusUpdate = 0;
    if (millis() - lastStatusUpdate > 5000) { // Обновлять каждые 5 секунд
        updateRelayStatus();
        lastStatusUpdate = millis();
    }

    // Проверка соединения WiFi
    static unsigned long lastWifiCheck = 0;
    if (millis() - lastWifiCheck > 10000) {
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("Переподключение к WiFi...");
            WiFi.disconnect();
            WiFi.reconnect();
        }
        lastWifiCheck = millis();
    }

    // Обработка сообщений Telegram
    if (millis() - bot_lasttime > BOT_MTBS) {
        int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
        
        if (numNewMessages) {
            Serial.println("Получены новые сообщения: " + String(numNewMessages));
            handleNewMessages(numNewMessages);
        }
        
        bot_lasttime = millis();
    }
}