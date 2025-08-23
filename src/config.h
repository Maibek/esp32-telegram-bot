#pragma once
#include <WiFiClientSecure.h>

#include "ID.h"

#ifdef __cplusplus
extern "C" {
#endif
uint8_t temprature_sens_read();  // Объявление функции из SDK
#ifdef __cplusplus
}
#endif

// Глобальные переменные для датчиков
float last_temp0 = NAN;
float last_hum0 = NAN;
float last_temp1 = NAN;
float last_hum1 = NAN;
const float min_temp = 26;
const float max_temp = 29;
const float alarm_min_temp = min_temp - 2 ;
const float alarm_max_temp = max_temp + 1;
const float min_hum = 40;
const float max_hum = 70;
const String sensor_0 = "Structor";
const String sensor_1 = "Nicobarensis";

//Глобальные переменные для времени
unsigned long last_sensor_read = 0;
unsigned long bot_lasttime = 0;
const unsigned long BOT_MTBS = 500;
const unsigned long ALARM_REPEAT_INTERVAL = 10800000; // 2 часа
unsigned long lastCallbackTime = 0;
const unsigned long CALLBACK_COOLDOWN = 500; // 0.5 секунды

// Переменные для пинов датчиков
const int SDA0_PIN = 18;
const int SCL0_PIN = 19;
const int SDA1_PIN = 22;
const int SCL1_PIN = 23;

// Переменные для подсветки
const int LIGHT0_PIN = 2;
const int LIGHT1_PIN = 4;

// Переменные для пинов реле
const int RELAY0_PIN = 16;
const int RELAY1_PIN = 17;

// Дополнительные переменные и флаги
bool temp_high = false;
bool light = false;
int light_status = LOW;
int light0_status = LOW;
int light1_status = LOW;
int relay0_status = HIGH;
int relay1_status = HIGH;

