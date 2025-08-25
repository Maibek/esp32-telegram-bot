#pragma once
#include <Adafruit_HTU21DF.h>
#include <Wire.h>

#include "config.h"

Adafruit_HTU21DF sensor0;
Adafruit_HTU21DF sensor1;

void safe_sensor_read() {
    if (millis() - last_sensor_read > 5000) {
        float t0 = sensor0.readTemperature();
        float h0 = sensor0.readHumidity();
        
        float t1 = sensor1.readTemperature();
        float h1 = sensor1.readHumidity();
        
        Serial.print("Датчик 0: Температура = ");
        Serial.print(t0);
        Serial.print(", Влажность = ");
        Serial.println(h0);
        
        Serial.print("Датчик 1: Температура = ");
        Serial.print(t1);
        Serial.print(", Влажность = ");
        Serial.println(h1);
        
        // Сохранение корректных значений
        if (!isnan(t0)) last_temp0 = t0;
        if (!isnan(h0)) last_hum0 = h0;
        if (!isnan(t1)) last_temp1 = t1;
        if (!isnan(h1)) last_hum1 = h1;

        // Сохранение не корректных значений
        if (isnan(t0)) last_temp0 = NAN;
        if (isnan(h0)) last_hum0 = NAN;
        if (isnan(t1)) last_temp1 = NAN;
        if (isnan(h1)) last_hum1 = NAN;
        
        last_sensor_read = millis();
    }
}