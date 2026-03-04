#pragma once
#include "telegram_bot.h"
#include "read_sensor.h"
#include "config.h"

void checkSensors(){
    static bool errorSent0 = false;
    static bool errorSent1 = false;
    if (isnan(last_temp_0) || isnan(last_hum_0)){
        if (!errorSent0){
            String message = "🔧 Датчик у " + sensor_0_name + " не исправен\n";
            bot.sendMessage(CHAT_ADMIN, message, "Markdown");
            errorSent0 = true;
        }
    }
    else{
        errorSent0 = false;
    }
    if (isnan(last_temp_1) || isnan(last_hum_1)){ 
        if (!errorSent1){
            String message = "🔧 Датчик у " + sensor_1_name + " не исправен\n";
            bot.sendMessage(CHAT_ADMIN, message, "Markdown");
            errorSent1 = true;
        }
    }
    else {
        errorSent1 = false;
    }
}
void checkHeating(){
    if (digitalRead(RELAY0_PIN) == HIGH && last_temp_0 < min_temp_0){
        digitalWrite(RELAY0_PIN, LOW);
        String message = "✅ Подогрев автоматически включен у " + sensor_0_name + "\n";
        message += "🌡️ Температура у " + sensor_0_name + ": " + String(last_temp_0, 2) + " °C\n";
        message += "🌡️ Температура у " + sensor_1_name + ": " + String(last_temp_1, 2) + " °C\n";
        bot.sendMessage(CHAT_ADMIN, message, "Markdown");
    }
    else if (digitalRead(RELAY1_PIN) == HIGH && last_temp_1 < min_temp_1){
        digitalWrite(RELAY1_PIN, LOW);
        String message = "✅ Подогрев автоматически включен у " + sensor_1_name + "\n";
        message += "🌡️ Температура у " + sensor_0_name + ": " + String(last_temp_0, 2) + " °C\n";
        message += "🌡️ Температура у " + sensor_1_name + ": " + String(last_temp_1, 2) + " °C\n";
        bot.sendMessage(CHAT_ADMIN, message, "Markdown");
    }
    else if (temp_high == true && (last_temp_0 >= max_temp_0 && last_temp_1 >= max_temp_1)){
        digitalWrite(RELAY0_PIN, HIGH);
        digitalWrite(RELAY1_PIN, HIGH);
        String message = "❌ *Подогрев отключен по превышению температуры!*\n";
        message += "🌡️ Температура у " + sensor_0_name + ": " + String(last_temp_0, 2) + " °C\n";
        message += "🌡️ Температура у " + sensor_1_name + ": " + String(last_temp_1, 2) + " °C\n";
        bot.sendMessage(CHAT_ADMIN, message, "Markdown");
        temp_high = false;
    }
    else if (digitalRead(RELAY0_PIN) == LOW && last_temp_0 >= max_temp_0){
        digitalWrite(RELAY0_PIN, HIGH);
        String message = "❌ *Подогрев отключен у " + sensor_0_name + " по превышению температуры!*\n";
        message += "🌡️ Температура у " + sensor_0_name + ": " + String(last_temp_0, 2) + " °C\n";
        message += "🌡️ Температура у " + sensor_1_name + ": " + String(last_temp_1, 2) + " °C\n";
        bot.sendMessage(CHAT_ADMIN, message, "Markdown");
    }
    else if (digitalRead(RELAY1_PIN) == LOW && last_temp_1 >= max_temp_1){
        digitalWrite(RELAY1_PIN, HIGH);
        String message = "❌ *Подогрев отключен у " + sensor_1_name + " по превышению температуры!*\n";
        message += "🌡️ Температура у " + sensor_0_name + ": " + String(last_temp_0, 2) + " °C\n";
        message += "🌡️ Температура у " + sensor_1_name + ": " + String(last_temp_1, 2) + " °C\n";
        bot.sendMessage(CHAT_ADMIN, message, "Markdown");
    }
}
void alarm_high_temp(){
    static unsigned long alert_high_time_0 = 0;
    static unsigned long alert_high_time_1 = 0;

    if (last_temp_0 > alarm_max_temp_0){
        if (millis() - alert_high_time_0 > ALARM_REPEAT_INTERVAL){
            digitalWrite(RELAY0_PIN, HIGH);        
            String message = "⚠️ Тревога\n";
            message += "🔥 Высокая температура у Structor!";
            message += "🌡️ Текущая температура: " + String(last_temp_0, 1) + "°C\n";
            bot.sendMessage(CHAT_ADMIN, message, "Markdown");

            alert_high_time_0 = millis();
        }
    }
    else if (last_temp_0 < alarm_max_temp_0) {
        alert_high_time_0 = 0;
    }

    if (last_temp_1 > alarm_max_temp_1){
        if (millis() - alert_high_time_1 > ALARM_REPEAT_INTERVAL){
            digitalWrite(RELAY1_PIN, HIGH);
            String message = "⚠️ Тревога!\n";
            message += "🔥 Высокая температура у Nicobarensis!";
            message += "🌡️ Текущая температура: " + String(last_temp_1, 1) + "°C\n";
            bot.sendMessage(CHAT_ADMIN, message, "Markdown");

            alert_high_time_1 = millis();
        }
    }
    else if (last_temp_1 < alarm_max_temp_1) {
        alert_high_time_1 = 0;
    }

}

void alarm_low_temp(){
    static unsigned long alert_low_time_0 = 0;
    static unsigned long alert_low_time_1 = 0;

    if (last_temp_0 < alarm_min_temp_0){
        if (millis() - alert_low_time_0 > ALARM_REPEAT_INTERVAL){
            String message = "⚠️ Тревога\n";
            message += "❄️ Низкая температура у Structor!";
            message += "🌡️ Текущая температура: " + String(last_temp_0, 1) + "°C\n";
            bot.sendMessage(CHAT_ADMIN, message, "Markdown");

            alert_low_time_0 = millis();
        }
    }
    else if (last_temp_0 > alarm_min_temp_0) {
        alert_low_time_0 = 0;
    }


    if (last_temp_1 < alarm_min_temp_1){
        if (millis() - alert_low_time_1 > ALARM_REPEAT_INTERVAL){
            String message = "⚠️ Тревога\n";
            message += "❄️ Низкая температура у Nicobarensis!";
            message += "🌡️ Текущая температура: " + String(last_temp_1, 1) + "°C\n";
            bot.sendMessage(CHAT_ADMIN, message, "Markdown");

            alert_low_time_1 = millis();
        }
    }
    else if (last_temp_1 > alarm_min_temp_1) {
        alert_low_time_1 = 0;
    }

}

void alarm_high_hum(){
    static unsigned long alert_high_time_hum_0 = 0;
    static unsigned long alert_high_time_hum_1 = 0;

    if (last_hum_0 > alarm_max_hum_0){
        if (millis() - alert_high_time_hum_0 > ALARM_REPEAT_INTERVAL){
            String message = "⚠️ Тревога\n";
            message += "🔥 Высокая влажность у Structor!";
            message += "💧 Текущая влажность: " + String(last_hum_0, 1) + "°C\n";
            bot.sendMessage(CHAT_ADMIN, message, "Markdown");

            alert_high_time_hum_0 = millis();
        }
    }
    else if (last_hum_0 < alarm_max_hum_0) {
        alert_high_time_hum_0 = 0;
    }

    if (last_hum_1 > alarm_max_hum_1){
        if (millis() - alert_high_time_hum_1 > ALARM_REPEAT_INTERVAL){
            String message = "⚠️ Тревога!\n";
            message += "🔥 Высокая влажность у Nicobarensis!";
            message += "💧 Текущая влажность: " + String(last_hum_1, 1) + "°C\n";
            bot.sendMessage(CHAT_ADMIN, message, "Markdown");

            alert_high_time_hum_1 = millis();
        }
    }
    else if (last_hum_1 < alarm_max_hum_1) {
        alert_high_time_hum_1 = 0;
    }

}

void alarm_low_hum(){
    static unsigned long alert_low_time_hum_0 = 0;
    static unsigned long alert_low_time_hum_1 = 0;

    if (last_hum_0 < alarm_min_hum_0){
        if (millis() - alert_low_time_hum_0 > ALARM_REPEAT_INTERVAL){
            String message = "⚠️ Тревога\n";
            message += "❄️ Низкая влажность у Structor!";
            message += "💧 Текущая влажность: " + String(last_hum_0, 1) + "°C\n";
            bot.sendMessage(CHAT_ADMIN, message, "Markdown");

            alert_low_time_hum_0 = millis();
        }
    }
    else if (last_hum_0 > alarm_min_hum_0) {
        alert_low_time_hum_0 = 0;
    }


    if (last_hum_1 < alarm_min_hum_1){
        if (millis() - alert_low_time_hum_1 > ALARM_REPEAT_INTERVAL){
            String message = "⚠️ Тревога\n";
            message += "❄️ Низкая влажность у Nicobarensis!";
            message += "💧 Текущая влажность: " + String(last_hum_1, 1) + "°C\n";
            bot.sendMessage(CHAT_ADMIN, message, "Markdown");

            alert_low_time_hum_1 = millis();
        }
    }
    else if (last_hum_1 > alarm_min_hum_1) {
        alert_low_time_hum_1 = 0;
    }

}