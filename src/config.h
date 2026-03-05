#pragma once
#include <WiFiClientSecure.h>

#include "secret.h"

#ifdef __cplusplus
extern "C" {
#endif
uint8_t temprature_sens_read();  // Объявление функции из SDK
#ifdef __cplusplus
}
#endif

// Структура данных для json
float min_temp_0;
float max_temp_0;
float min_temp_1;
float max_temp_1;
float alarm_min_temp_0;
float alarm_max_temp_0;
float alarm_min_hum_0;
float alarm_max_hum_0;
float alarm_min_temp_1;
float alarm_max_temp_1;
float alarm_min_hum_1;
float alarm_max_hum_1;
String sensor_0_name;
String sensor_1_name;


// Глобальные переменные для датчиков
float last_temp_0 = NAN;
float last_hum_0 = NAN;
float last_temp_1 = NAN;
float last_hum_1 = NAN;

//Глобальные переменные для времени
unsigned long last_sensor_read = 0;
unsigned long bot_lasttime = 0;
const unsigned long BOT_MTBS = 1000;
const unsigned long ALARM_REPEAT_INTERVAL = 10800000; // 2 часа
unsigned long lastCallbackTime = 0;
const unsigned long CALLBACK_COOLDOWN = 500; // 0.5 секунды

// Переменные для пинов датчиков
const int SDA0_PIN = 23;
const int SCL0_PIN = 22;
const int SDA1_PIN = 19;
const int SCL1_PIN = 18;

// Переменные для подсветки
const int LIGHT0_PIN = 2;
const int LIGHT1_PIN = 4;

// Переменные для пинов реле
const int RELAY0_PIN = 16;
const int RELAY1_PIN = 17;

// Дополнительные переменные и флаги
bool temp_high = false;
bool light = false;
int light_status = HIGH;
int light_0_status = HIGH;
int light_1_status = HIGH;
int relay_0_status = HIGH;
int relay_1_status = HIGH;

