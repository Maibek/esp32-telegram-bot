#pragma once
#include <Adafruit_HTU21DF.h>
#include <Wire.h>

#include "config.h"

Adafruit_HTU21DF sensor0;
Adafruit_HTU21DF sensor1;

void safe_sensor_read() {
    if (millis() - last_sensor_read > 5000) {
        // Чтение первого датчика
        float temp0 = sensor0.readTemperature();
        float hum0 = sensor0.readHumidity();
        
        // Чтение второго датчика
        float temp1 = sensor1.readTemperature();
        float hum1 = sensor1.readHumidity();
        
        Serial.print("Датчик 0: Температура = ");
        Serial.print(temp0);
        Serial.print(", Влажность = ");
        Serial.println(hum0);
        
        Serial.print("Датчик 1: Температура = ");
        Serial.print(temp1);
        Serial.print(", Влажность = ");
        Serial.println(hum1);
        
        // Сохранение корректных значений
        if (!isnan(temp0)) last_temp_0 = temp0;
        if (!isnan(hum0)) last_hum_0 = hum0;
        if (!isnan(temp1)) last_temp_1 = temp1;
        if (!isnan(hum1)) last_hum_1 = hum1;

        // Сохранение не корректных значений
        if (isnan(temp0)) last_temp_0 = NAN;
        if (isnan(hum0)) last_hum_0 = NAN;
        if (isnan(temp1)) last_temp_1 = NAN;
        if (isnan(hum1)) last_hum_1 = NAN;
        
        last_sensor_read = millis();
    }
}