#pragma once
#include "telegram_bot.h"
#include "read_sensor.h"
#include "config.h"

void checkSensors(){
    static bool errorSent0 = false;
    static bool errorSent1 = false;
    if (isnan(last_temp0) || isnan(last_hum0)){
        if (!errorSent0){
            String message = "🔧 Датчик у " + sensor_0 + " не исправен\n";
            bot.sendMessage(CHAT_ID, message, "Markdown");
            errorSent0 = true;
        }
    }
    else{
        errorSent0 = false;
    }
    if (isnan(last_temp1) || isnan(last_hum1)){ 
        if (!errorSent1){
            String message = "🔧 Датчик у " + sensor_1 + " не исправен\n";
            bot.sendMessage(CHAT_ID, message, "Markdown");
            errorSent1 = true;
        }
    }
    else {
        errorSent1 = false;
    }
}
void checkHeating(){
    if (digitalRead(RELAY0_PIN) == HIGH && last_temp0 < min_temp){
        digitalWrite(RELAY0_PIN, LOW);
        String message = "✅ Подогрев автоматически включен у " + sensor_0 + "\n";
        message += "🌡️ Температура у " + sensor_0 + ": " + String(last_temp0, 2) + " °C\n";
        message += "🌡️ Температура у " + sensor_1 + ": " + String(last_temp1, 2) + " °C\n";
        bot.sendMessage(CHAT_ID, message, "Markdown");
    }
    else if (digitalRead(RELAY1_PIN) == HIGH && last_temp1 < min_temp){
        digitalWrite(RELAY1_PIN, LOW);
        String message = "✅ Подогрев автоматически включен у " + sensor_1 + "\n";
        message += "🌡️ Температура у " + sensor_0 + ": " + String(last_temp0, 2) + " °C\n";
        message += "🌡️ Температура у " + sensor_1 + ": " + String(last_temp1, 2) + " °C\n";
        bot.sendMessage(CHAT_ID, message, "Markdown");
    }
    else if (temp_high == true && (last_temp0 >= max_temp && last_temp1 >= max_temp)){
        digitalWrite(RELAY0_PIN, HIGH);
        digitalWrite(RELAY1_PIN, HIGH);
        String message = "❌ *Подогрев отключен по превышению температуры!*\n";
        message += "🌡️ Температура у " + sensor_0 + ": " + String(last_temp0, 2) + " °C\n";
        message += "🌡️ Температура у " + sensor_1 + ": " + String(last_temp1, 2) + " °C\n";
        bot.sendMessage(CHAT_ID, message, "Markdown");
        temp_high = false;
    }
    else if (digitalRead(RELAY0_PIN) == LOW && last_temp0 >= max_temp){
        digitalWrite(RELAY0_PIN, HIGH);
        String message = "❌ *Подогрев отключен у " + sensor_0 + " по превышению температуры!*\n";
        message += "🌡️ Температура у " + sensor_0 + ": " + String(last_temp0, 2) + " °C\n";
        message += "🌡️ Температура у " + sensor_1 + ": " + String(last_temp1, 2) + " °C\n";
        bot.sendMessage(CHAT_ID, message, "Markdown");
    }
    else if (digitalRead(RELAY1_PIN) == LOW && last_temp1 >= max_temp){
        digitalWrite(RELAY1_PIN, HIGH);
        String message = "❌ *Подогрев отключен у " + sensor_1 + " по превышению температуры!*\n";
        message += "🌡️ Температура у " + sensor_0 + ": " + String(last_temp0, 2) + " °C\n";
        message += "🌡️ Температура у " + sensor_1 + ": " + String(last_temp1, 2) + " °C\n";
        bot.sendMessage(CHAT_ID, message, "Markdown");
    }
}
void alarm_high_temp(){
    static unsigned long alert_high_time0 = 0;
    static unsigned long alert_high_time1 = 0;

    if (last_temp0 > alarm_max_temp){
        if (millis() - alert_high_time0 > ALARM_REPEAT_INTERVAL){
            digitalWrite(RELAY0_PIN, HIGH);        
            String message = "⚠️ Тревога\n";
            message += "🔥 Высокая температура у Structor!";
            message += "🌡️ Текущая температура: " + String(last_temp0, 1) + "°C\n";
            bot.sendMessage(CHAT_ID, message, "Markdown");

            alert_high_time0 = millis();
        }
    }
    else if (last_temp0 < alarm_max_temp) {
        alert_high_time0 = 0;
    }

    if (last_temp1 > alarm_max_temp){
        if (millis() - alert_high_time1 > ALARM_REPEAT_INTERVAL){
            digitalWrite(RELAY1_PIN, HIGH);
            String message = "⚠️ Тревога!\n";
            message += "🔥 Высокая температура у Nicobarensis!";
            message += "🌡️ Текущая температура: " + String(last_temp1, 1) + "°C\n";
            bot.sendMessage(CHAT_ID, message, "Markdown");

            alert_high_time1 = millis();
        }
    }
    else if (last_temp1 < alarm_max_temp) {
        alert_high_time1 = 0;
    }

}

void alarm_low_temp(){
    static unsigned long alert_low_time0 = 0;
    static unsigned long alert_low_time1 = 0;

    if (last_temp0 < alarm_min_temp){
        if (millis() - alert_low_time0 > ALARM_REPEAT_INTERVAL){
            String message = "⚠️ Тревога\n";
            message += "❄️ Низкая температура у Structor!";
            message += "🌡️ Текущая температура: " + String(last_temp0, 1) + "°C\n";
            bot.sendMessage(CHAT_ID, message, "Markdown");

            alert_low_time0 = millis();
        }
    }
    else if (last_temp0 > alarm_min_temp) {
        alert_low_time0 = 0;
    }


    if (last_temp1 < alarm_min_temp){
        if (millis() - alert_low_time1 > ALARM_REPEAT_INTERVAL){
            String message = "⚠️ Тревога\n";
            message += "❄️ Низкая температура у Nicobarensis!";
            message += "🌡️ Текущая температура: " + String(last_temp1, 1) + "°C\n";
            bot.sendMessage(CHAT_ID, message, "Markdown");

            alert_low_time1 = millis();
        }
    }
    else if (last_temp1 > alarm_min_temp) {
        alert_low_time1 = 0;
    }

}

void alarm_high_hum(){
    static unsigned long alert_high_time_hum0 = 0;
    static unsigned long alert_high_time_hum1 = 0;

    if (last_hum0 > max_hum){
        if (millis() - alert_high_time_hum0 > ALARM_REPEAT_INTERVAL){
            String message = "⚠️ Тревога\n";
            message += "🔥 Высокая влажность у Structor!";
            message += "💧 Текущая влажность: " + String(last_hum0, 1) + "°C\n";
            bot.sendMessage(CHAT_ID, message, "Markdown");

            alert_high_time_hum0 = millis();
        }
    }
    else if (last_hum0 < max_hum) {
        alert_high_time_hum0 = 0;
    }

    if (last_hum1 > max_hum){
        if (millis() - alert_high_time_hum1 > ALARM_REPEAT_INTERVAL){
            String message = "⚠️ Тревога!\n";
            message += "🔥 Высокая влажность у Nicobarensis!";
            message += "💧 Текущая влажность: " + String(last_hum1, 1) + "°C\n";
            bot.sendMessage(CHAT_ID, message, "Markdown");

            alert_high_time_hum1 = millis();
        }
    }
    else if (last_hum1 < max_hum) {
        alert_high_time_hum1 = 0;
    }

}

void alarm_low_hum(){
    static unsigned long alert_low_time_hum0 = 0;
    static unsigned long alert_low_time_hum1 = 0;

    if (last_hum0 < min_hum){
        if (millis() - alert_low_time_hum0 > ALARM_REPEAT_INTERVAL){
            String message = "⚠️ Тревога\n";
            message += "❄️ Низкая влажность у Structor!";
            message += "💧 Текущая влажность: " + String(last_hum0, 1) + "°C\n";
            bot.sendMessage(CHAT_ID, message, "Markdown");

            alert_low_time_hum0 = millis();
        }
    }
    else if (last_hum0 > min_hum) {
        alert_low_time_hum0 = 0;
    }


    if (last_hum1 < min_hum){
        if (millis() - alert_low_time_hum1 > ALARM_REPEAT_INTERVAL){
            String message = "⚠️ Тревога\n";
            message += "❄️ Низкая влажность у Nicobarensis!";
            message += "💧 Текущая влажность: " + String(last_hum1, 1) + "°C\n";
            bot.sendMessage(CHAT_ID, message, "Markdown");

            alert_low_time_hum1 = millis();
        }
    }
    else if (last_hum1 > min_hum) {
        alert_low_time_hum1 = 0;
    }

}