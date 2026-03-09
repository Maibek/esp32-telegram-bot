#pragma once

#include "telegram_bot.h"
#include "read_sensor.h"
#include "config.h"


class Throttler {
private:
    struct Entry {
        unsigned long lastTime;
        unsigned long interval;
    };
    std::map<String, Entry> _timers;

public:

    bool canSend(const String& key, unsigned long interval) {
        unsigned long now = millis();
        auto it = _timers.find(key);
        if (it == _timers.end()) {
            _timers[key] = {now, interval};
            return true;
        }

        Entry& e = it->second;
        if (now - e.lastTime >= e.interval) {
            e.lastTime = now;
            e.interval = interval;
            return true;
        }
        return false;
    }

    void reset(const String& key) {
        _timers.erase(key);
    }
};

Throttler notificationThrottler;

void checkSensors() {
    String key0 = "sensor_error_" + sensor_0_name;
    String key1 = "sensor_error_" + sensor_1_name;

    if (isnan(last_temp_0) || isnan(last_hum_0)) {
        if (notificationThrottler.canSend(key0, alarm_sensor_interval * 60000)) {
            String message = "🔧 Датчик неисправен у " + sensor_0_name + "\n";
            sendAdminMessage(message);
        }
    } 
    else {
        notificationThrottler.reset(key0);
    }

    if (isnan(last_temp_1) || isnan(last_hum_1)) {
        if (notificationThrottler.canSend(key1, ERROR_SENSOR_INTERVAL * 60000)) {
            String message = "🔧 Датчик неисправен у " + sensor_1_name + "\n";
            sendAdminMessage(message);
        }
    } 
    else {
        notificationThrottler.reset(key1);
    }
}

// Автоматическое управление подогревом + уведомления
void checkHeating() {
    if (digitalRead(RELAY0_PIN) == HIGH && last_temp_0 < min_temp_0) {
        digitalWrite(RELAY0_PIN, LOW);
        String message = "✅ Автоматически включён подогрев у " + sensor_0_name + "\n";
        message += "🌡️ Темп. " + sensor_0_name + ": " + String(last_temp_0, 2) + " °C\n";
        message += "🌡️ Темп. " + sensor_1_name + ": " + String(last_temp_1, 2) + " °C\n";
        sendAdminMessage(message);
    }
    else if (digitalRead(RELAY1_PIN) == HIGH && last_temp_1 < min_temp_1) {
        digitalWrite(RELAY1_PIN, LOW);
        String message = "✅ Автоматически включён подогрев у " + sensor_1_name + "\n";
        message += "🌡️ Темп. " + sensor_0_name + ": " + String(last_temp_0, 2) + " °C\n";
        message += "🌡️ Темп. " + sensor_1_name + ": " + String(last_temp_1, 2) + " °C\n";
        sendAdminMessage(message);
    }

    else if (last_temp_0 >= max_temp_0 && last_temp_1 >= max_temp_1) {
        digitalWrite(RELAY0_PIN, HIGH);
        digitalWrite(RELAY1_PIN, HIGH);
        String message = "❌ Подогрев отключён по превышению температуры!\n";
        message += "🌡️ " + sensor_0_name + ": " + String(last_temp_0, 2) + " °C\n";
        message += "🌡️ " + sensor_1_name + ": " + String(last_temp_1, 2) + " °C\n";
        sendAdminMessage(message);
    }

    else if (digitalRead(RELAY0_PIN) == LOW && last_temp_0 >= max_temp_0) {
        digitalWrite(RELAY0_PIN, HIGH);
        String message = "❌ Подогрев отключён у " + sensor_0_name + " (превышение)\n";
        message += "🌡️ " + sensor_0_name + ": " + String(last_temp_0, 2) + " °C\n";
        message += "🌡️ " + sensor_1_name + ": " + String(last_temp_1, 2) + " °C\n";
        sendAdminMessage(message);
    }
    else if (digitalRead(RELAY1_PIN) == LOW && last_temp_1 >= max_temp_1) {
        digitalWrite(RELAY1_PIN, HIGH);
        String message = "❌ Подогрев отключён у " + sensor_1_name + " (превышение)\n";
        message += "🌡️ " + sensor_0_name + ": " + String(last_temp_0, 2) + " °C\n";
        message += "🌡️ " + sensor_1_name + ": " + String(last_temp_1, 2) + " °C\n";
        sendAdminMessage(message);
    }
}

void alarm_high_temp() {    
    String key0 = "high_temp_" + sensor_0_name;
    String key1 = "high_temp_" + sensor_1_name;

    if (last_temp_0 > alarm_max_temp_0) {
        if (notificationThrottler.canSend(key0, alarm_high_temp_interval * 60000)) {
            digitalWrite(RELAY0_PIN, HIGH);
            String message = "⚠️ Тревога! Высокая температура\n";
            message += "🔥 У " + sensor_0_name + "!\n";
            message += "🌡️ Текущая: " + String(last_temp_0, 1) + " °C\n";
            sendAdminMessage(message);
        }
    } 
    else {
        notificationThrottler.reset(key0);
    }

    if (last_temp_1 > alarm_max_temp_1) {
        if (notificationThrottler.canSend(key1, alarm_high_temp_interval * 60000)) {
            digitalWrite(RELAY1_PIN, HIGH);
            String message = "⚠️ Тревога! Высокая температура\n";
            message += "🔥 У " + sensor_1_name + "!\n";
            message += "🌡️ Текущая: " + String(last_temp_1, 1) + " °C\n";
            sendAdminMessage(message);
        }
    } 
    else {
        notificationThrottler.reset(key1);
    }
}

void alarm_low_temp() {
    String key0 = "low_temp_" + sensor_0_name;
    String key1 = "low_temp_" + sensor_1_name;

    if (last_temp_0 < alarm_min_temp_0) {
        if (notificationThrottler.canSend(key0, alarm_low_temp_interval * 60000)) {
            String message = "⚠️ Тревога! Низкая температура\n";
            message += "❄️ У " + sensor_0_name + "!\n";
            message += "🌡️ Текущая: " + String(last_temp_0, 1) + " °C\n";
            sendAdminMessage(message);
        }
    } 
    else {
        notificationThrottler.reset(key0);
    }

    if (last_temp_1 < alarm_min_temp_1) {
        if (notificationThrottler.canSend(key1, alarm_low_temp_interval * 60000)) {
            String message = "⚠️ Тревога! Низкая температура\n";
            message += "❄️ У " + sensor_1_name + "!\n";
            message += "🌡️ Текущая: " + String(last_temp_1, 1) + " °C\n";
            sendAdminMessage(message);
        }
    } 
    else {
        notificationThrottler.reset(key1);
    }
}

void alarm_high_hum() {
    String key0 = "high_hum_" + sensor_0_name;
    String key1 = "high_hum_" + sensor_1_name;

    if (last_hum_0 > alarm_max_hum_0) {
        if (notificationThrottler.canSend(key0, alarm_high_hum_interval * 60000)) {
            String message = "⚠️ Тревога! Высокая влажность\n";
            message += "💧 У " + sensor_0_name + "!\n";
            message += "Текущая: " + String(last_hum_0, 1) + " %\n";
            sendAdminMessage(message);
        }
    } 
    else {
        notificationThrottler.reset(key0);
    }

    if (last_hum_1 > alarm_max_hum_1) {
        if (notificationThrottler.canSend(key1, alarm_high_hum_interval * 60000)) {
            String message = "⚠️ Тревога! Высокая влажность\n";
            message += "💧 У " + sensor_1_name + "!\n";
            message += "Текущая: " + String(last_hum_1, 1) + " %\n";
            sendAdminMessage(message);
        }
    } 
    else {
        notificationThrottler.reset(key1);
    }
}

void alarm_low_hum() {
    String key0 = "low_hum_" + sensor_0_name;
    String key1 = "low_hum_" + sensor_1_name;

    if (last_hum_0 < alarm_min_hum_0) {
        if (notificationThrottler.canSend(key0, alarm_low_hum_interval * 60000)) {
            String message = "⚠️ Тревога! Низкая влажность\n";
            message += "❄️ У " + sensor_0_name + "!\n";
            message += "Текущая: " + String(last_hum_0, 1) + " %\n";
            sendAdminMessage(message);
        }
    } 
    else {
        notificationThrottler.reset(key0);
    }

    if (last_hum_1 < alarm_min_hum_1) {
        if (notificationThrottler.canSend(key1, alarm_low_hum_interval * 60000)) {
            String message = "⚠️ Тревога! Низкая влажность\n";
            message += "❄️ У " + sensor_1_name + "!\n";
            message += "Текущая: " + String(last_hum_1, 1) + " %\n";
            sendAdminMessage(message);
        }
    } 
    else {
        notificationThrottler.reset(key1);
    }
}