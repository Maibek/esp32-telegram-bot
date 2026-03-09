#pragma once
#include <LittleFS.h>
#include <ArduinoJson.h>

#include "config.h"

// ИСПРАВЛЕНО: используем fs::File для избежания конфликта с fb::File
void saveSettings() {
    fs::File file = LittleFS.open("/settings.json", "w");
    if (!file) {
        Serial.println("Ошибка создания файла settings.json");
        return;
    }

    JsonDocument doc;
    doc["min_temp_0"]         = min_temp_0;
    doc["max_temp_0"]         = max_temp_0;
    doc["min_temp_1"]         = min_temp_1;
    doc["max_temp_1"]         = max_temp_1;
    doc["alarm_min_temp_0"]   = alarm_min_temp_0;
    doc["alarm_max_temp_0"]   = alarm_max_temp_0;
    doc["alarm_min_hum_0"]    = alarm_min_hum_0;
    doc["alarm_max_hum_0"]    = alarm_max_hum_0;
    doc["alarm_min_temp_1"]   = alarm_min_temp_1;
    doc["alarm_max_temp_1"]   = alarm_max_temp_1;
    doc["alarm_min_hum_1"]    = alarm_min_hum_1;
    doc["alarm_max_hum_1"]    = alarm_max_hum_1;
    doc["sensor_0_name"]      = sensor_0_name;
    doc["sensor_1_name"]      = sensor_1_name;

    serializeJsonPretty(doc, file);

    if (serializeJson(doc, file) == 0) {
        Serial.println("Ошибка записи JSON");
    } 
    else {
        Serial.println("Настройки сохранены в /settings.json");
    }
    file.close();
}

void saveDefaults() {
    min_temp_0                  = 24.0f;
    max_temp_0                  = 28.0f;
    min_temp_1                  = 26.0f;
    max_temp_1                  = 30.0f;

    alarm_min_temp_0            = 22.0f;
    alarm_max_temp_0            = 30.0f;
    alarm_min_hum_0             = 40.0f;
    alarm_max_hum_0             = 80.0f;

    alarm_min_temp_1            = 24.0f;
    alarm_max_temp_1            = 32.0f;
    alarm_min_hum_1             = 30.0f;
    alarm_max_hum_1             = 70.0f;

    alarm_all_interval          =180;
    alarm_sensor_interval       =60;
    alarm_high_temp_interval    =60;
    alarm_low_temp_interval     =60;
    alarm_high_hum_interval     =180;
    alarm_low_hum_interval      =180;
    
    sensor_0_name               = "Structor";
    sensor_1_name               = "Nicobarensis";

    saveSettings();
}

void loadSettings() {
    fs::File file = LittleFS.open("/settings.json", "r");
    if (!file) {
        Serial.println("Файл settings.json не найден, создаём дефолт");
        saveDefaults();
        return;
    }
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();

    if (error) {
        Serial.println("Ошибка парсинга JSON: " + String(error.c_str()));
        saveDefaults();
        return;
    }

    // Загрузка значений
    min_temp_0                  = doc["min_temp_0"]                 | 24.0f;
    max_temp_0                  = doc["max_temp_0"]                 | 28.0f;
    min_temp_1                  = doc["min_temp_1"]                 | 26.0f;
    max_temp_1                  = doc["max_temp_1"]                 | 30.0f;
    alarm_min_temp_0            = doc["alarm_min_temp_0"]           | 22.0f;
    alarm_max_temp_0            = doc["alarm_max_temp_0"]           | 30.0f;
    alarm_min_hum_0             = doc["alarm_min_hum_0"]            | 40.0f;
    alarm_max_hum_0             = doc["alarm_max_hum_0"]            | 80.0f;
    alarm_min_temp_1            = doc["alarm_min_temp_1"]           | 24.0f;
    alarm_max_temp_1            = doc["alarm_max_temp_1"]           | 32.0f;
    alarm_min_hum_1             = doc["alarm_min_hum_1"]            | 30.0f;
    alarm_max_hum_1             = doc["alarm_max_hum_1"]            | 70.0f;
    alarm_all_interval          = doc["alarm_all_interval"]         | 180;
    alarm_sensor_interval       = doc["alarm_sensor_interval"]      | 60;
    alarm_high_temp_interval    = doc["alarm_high_temp_interval"]   | 60;
    alarm_low_temp_interval     = doc["alarm_low_temp_interval"]    | 60;
    alarm_high_hum_interval     = doc["alarm_high_hum_interval"]    | 180;
    alarm_low_hum_interval      = doc["alarm_low_hum_interval"]     | 180;
    sensor_0_name               = doc["sensor_0_name"]              | "Structor";
    sensor_1_name               = doc["sensor_1_name"]              | "Nicobarensis";

    Serial.println("Настройки загружены:");
    Serial.print("min_temp_0: ");               Serial.println(min_temp_0);
    Serial.print("max_temp_0: ");               Serial.println(max_temp_0);
    Serial.print("min_temp_1: ");               Serial.println(min_temp_1);
    Serial.print("max_temp_1: ");               Serial.println(max_temp_1);
    Serial.print("alarm_min_temp_0: ");         Serial.println(alarm_min_temp_0);
    Serial.print("alarm_max_temp_0: ");         Serial.println(alarm_max_temp_0);
    Serial.print("alarm_min_hum_0: ");          Serial.println(alarm_min_hum_0);
    Serial.print("alarm_max_hum_0: ");          Serial.println(alarm_max_hum_0);
    Serial.print("alarm_min_temp_1: ");         Serial.println(alarm_min_temp_1);
    Serial.print("alarm_max_temp_1: ");         Serial.println(alarm_max_temp_1);
    Serial.print("alarm_min_hum_1: ");          Serial.println(alarm_min_hum_1);
    Serial.print("alarm_max_hum_1: ");          Serial.println(alarm_max_hum_1);
    Serial.print("alarm_all_interval: ");       Serial.println(alarm_all_interval);
    Serial.print("alarm_sensor_interval: ");    Serial.println(alarm_sensor_interval);
    Serial.print("alarm_high_temp_interval: "); Serial.println(alarm_high_temp_interval);
    Serial.print("alarm_low_temp_interval: ");  Serial.println(alarm_low_temp_interval);
    Serial.print("alarm_high_hum_interval: ");  Serial.println(alarm_high_hum_interval);
    Serial.print("alarm_low_hum_interval: ");   Serial.println(alarm_low_hum_interval);
    Serial.print("sensor_0_name: ");            Serial.println(sensor_0_name);
    Serial.print("sensor_1_name: ");            Serial.println(sensor_1_name);
}