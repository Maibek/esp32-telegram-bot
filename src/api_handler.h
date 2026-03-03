#ifndef API_HANDLER_H
#define API_HANDLER_H

#include <Arduino.h>
#include <ArduinoJson.h>

#include "config.h"
#include "read_sensor.h"


void parseJsonCommand(
    const String& json,
    String& out_action,
    int& out_id,
    String& out_state,
    int& out_sensor_id
) {

    JsonDocument doc;

    DeserializationError error = deserializeJson(doc, json);

    if (error) {
        Serial.print("Ошибка парсинга JSON: ");
        Serial.println(error.c_str());
        return;
    }

    out_action      = doc["action"] | "";
    out_id          = doc["id"] | -1;
    out_state       = doc["state"] | "";
    out_sensor_id   = doc["sensor_id"] | -1;

    Serial.print("Parsed: action=");
    Serial.print(out_action);
    if (out_sensor_id >= 0) {
        Serial.print(", sensor_id=");
        Serial.println(out_sensor_id);
    }
    if (out_id >= 0){
        Serial.print(", id=");
        Serial.print(out_id);
        Serial.print(", state=");
        Serial.println(out_state);
    }
    Serial.println();
}

bool readSingleSensor(
    int sensor_id,
    float& out_temp,
    float& out_hum
) {

    if (sensor_id < 0 || sensor_id > 1) {
        Serial.println("❌ Неверный номер датчика: " + String(sensor_id));
        return false;
    }

    if (sensor_id == 0) {
        out_temp    = sensor0.readTemperature();
        out_hum     = sensor0.readHumidity();

        Serial.print("📊 Датчик 0: Температура=");
        Serial.print(out_temp);
        Serial.print(", Влажность=");
        Serial.println(out_hum);
    }

    if (sensor_id == 1) {
        out_temp    = sensor1.readTemperature();
        out_hum     = sensor1.readHumidity();

        Serial.print("📊 Датчик 1: Температура=");
        Serial.print(out_temp);
        Serial.print(", Влажность=");
        Serial.println(out_hum);
    }

    if (isnan(out_temp) || isnan(out_hum)) {
        Serial.println("Ошибка чтения датчика " + String(sensor_id));
        return false;
    }

    return true;
}

String buildSensorJson(
    int sensor_id,
    float temperature,
    float humidity
) {

    JsonDocument doc;

    doc["action"]       = "sensor_data";
    doc["sensor_id"]    = sensor_id;
    doc["temperature"] = temperature;
    doc["humidity"]     = humidity;

    String output;
    serializeJson(doc, output);

    Serial.print("Отправляем JSON");
    Serial.println(output);

    return output;
}

void executeApiCommand(
    const String& action,
    int relay_id,
    const String& state,
    int sensor_id,
    String chat_id
) {
    if (action == "relay") {

        if (relay_id == 0) {
            if (state == "on") {
                digitalWrite(RELAY0_PIN, LOW);
                Serial.println("✅ Реле 0 включено");
            }
            else if (state == "off") {
                digitalWrite(RELAY0_PIN, HIGH);
                Serial.println("❌ Реле 0 выключено");
            }
        }
        if (relay_id == 1) {
            if (state == "on") {
                digitalWrite(RELAY1_PIN, LOW);
                Serial.println("✅ Реле 1 включено");
            }
            else if (state == "off") {
                digitalWrite(RELAY1_PIN, HIGH);
                Serial.println("❌ Реле 1 выключено");
            }
        }
    }
    else if (action == "sensor") {
        if (sensor_id < 0 || sensor_id > 1) {
            Serial.println("❌ Неверный sensor_id: " + String(sensor_id));
            return;
        }
        
        float temp = 0;
        float hum = 0;
        
        if (readSingleSensor(sensor_id, temp, hum)) {
            String jsonResponse = buildSensorJson(sensor_id, temp, hum);
            
            // ВРЕМЕННО: вывод в Serial для теста
            Serial.print("📤 Готов к отправке: ");
            Serial.println(jsonResponse);
        }
        else {
            Serial.println("❌ Ошибка чтения датчика");
        }
    }
}

#endif