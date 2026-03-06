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
    static bool errorSent0 = false;
    static bool errorSent1 = false;

    if (isnan(last_temp_0) || isnan(last_hum_0)) {
        if (!errorSent0) {
            String message = "🔧 *Датчик неисправен!* у " + sensor_0_name + "\n";
            sendAdminMessage(message);
            errorSent0 = true;
        }
    } else {
        errorSent0 = false;
    }

    if (isnan(last_temp_1) || isnan(last_hum_1)) {
        if (!errorSent1) {
            String message = "🔧 *Датчик неисправен!* у " + sensor_1_name + "\n";
            sendAdminMessage(message);
            errorSent1 = true;
        }
    } else {
        errorSent1 = false;
    }
}

// Автоматическое управление подогревом + уведомления
void checkHeating() {
    if (digitalRead(RELAY0_PIN) == HIGH && last_temp_0 < min_temp_0) {
        digitalWrite(RELAY0_PIN, LOW);
        String message = "✅ *Автоматически включён* подогрев у " + sensor_0_name + "\n";
        message += "🌡️ Темп. " + sensor_0_name + ": " + String(last_temp_0, 2) + " °C\n";
        message += "🌡️ Темп. " + sensor_1_name + ": " + String(last_temp_1, 2) + " °C\n";
        sendAdminMessage(message);
    }
    else if (digitalRead(RELAY1_PIN) == HIGH && last_temp_1 < min_temp_1) {
        digitalWrite(RELAY1_PIN, LOW);
        String message = "✅ *Автоматически включён* подогрев у " + sensor_1_name + "\n";
        message += "🌡️ Темп. " + sensor_0_name + ": " + String(last_temp_0, 2) + " °C\n";
        message += "🌡️ Темп. " + sensor_1_name + ": " + String(last_temp_1, 2) + " °C\n";
        sendAdminMessage(message);
    }

    else if (last_temp_0 >= max_temp_0 && last_temp_1 >= max_temp_1) {
        digitalWrite(RELAY0_PIN, HIGH);
        digitalWrite(RELAY1_PIN, HIGH);
        String message = "❌ *Подогрев отключён по превышению температуры!*\n";
        message += "🌡️ " + sensor_0_name + ": " + String(last_temp_0, 2) + " °C\n";
        message += "🌡️ " + sensor_1_name + ": " + String(last_temp_1, 2) + " °C\n";
        sendAdminMessage(message);
        temp_high = false;
    }

    else if (digitalRead(RELAY0_PIN) == LOW && last_temp_0 >= max_temp_0) {
        digitalWrite(RELAY0_PIN, HIGH);
        String message = "❌ *Подогрев отключён* у " + sensor_0_name + " (превышение)\n";
        message += "🌡️ " + sensor_0_name + ": " + String(last_temp_0, 2) + " °C\n";
        message += "🌡️ " + sensor_1_name + ": " + String(last_temp_1, 2) + " °C\n";
        sendAdminMessage(message);
    }
    else if (digitalRead(RELAY1_PIN) == LOW && last_temp_1 >= max_temp_1) {
        digitalWrite(RELAY1_PIN, HIGH);
        String message = "❌ *Подогрев отключён* у " + sensor_1_name + " (превышение)\n";
        message += "🌡️ " + sensor_0_name + ": " + String(last_temp_0, 2) + " °C\n";
        message += "🌡️ " + sensor_1_name + ": " + String(last_temp_1, 2) + " °C\n";
        sendAdminMessage(message);
    }
}

void alarm_high_temp() {
    static unsigned long alert_high_time_0 = 0;
    static unsigned long alert_high_time_1 = 0;

    if (last_temp_0 > alarm_max_temp_0) {
        if (millis() - alert_high_time_0 > ALARM_REPEAT_INTERVAL) {
            digitalWrite(RELAY0_PIN, HIGH);
            String message = "⚠️ *Тревога! Высокая температура*\n";
            message += "🔥 У " + sensor_0_name + "!\n";
            message += "🌡️ Текущая: " + String(last_temp_0, 1) + " °C\n";
            sendAdminMessage(message);
            alert_high_time_0 = millis();
        }
    } else if (last_temp_0 <= alarm_max_temp_0) {
        alert_high_time_0 = 0;
    }

    if (last_temp_1 > alarm_max_temp_1) {
        if (millis() - alert_high_time_1 > ALARM_REPEAT_INTERVAL) {
            digitalWrite(RELAY1_PIN, HIGH);
            String message = "⚠️ *Тревога! Высокая температура*\n";
            message += "🔥 У " + sensor_1_name + "!\n";
            message += "🌡️ Текущая: " + String(last_temp_1, 1) + " °C\n";
            sendAdminMessage(message);
            alert_high_time_1 = millis();
        }
    } else if (last_temp_1 <= alarm_max_temp_1) {
        alert_high_time_1 = 0;
    }
}

void alarm_low_temp() {
    static unsigned long alert_low_time_0 = 0;
    static unsigned long alert_low_time_1 = 0;

    if (last_temp_0 < alarm_min_temp_0) {
        if (millis() - alert_low_time_0 > ALARM_REPEAT_INTERVAL) {
            String message = "⚠️ *Тревога! Низкая температура*\n";
            message += "❄️ У " + sensor_0_name + "!\n";
            message += "🌡️ Текущая: " + String(last_temp_0, 1) + " °C\n";
            sendAdminMessage(message);
            alert_low_time_0 = millis();
        }
    } else if (last_temp_0 >= alarm_min_temp_0) {
        alert_low_time_0 = 0;
    }

    if (last_temp_1 < alarm_min_temp_1) {
        if (millis() - alert_low_time_1 > ALARM_REPEAT_INTERVAL) {
            String message = "⚠️ *Тревога! Низкая температура*\n";
            message += "❄️ У " + sensor_1_name + "!\n";
            message += "🌡️ Текущая: " + String(last_temp_1, 1) + " °C\n";
            sendAdminMessage(message);
            alert_low_time_1 = millis();
        }
    } else if (last_temp_1 >= alarm_min_temp_1) {
        alert_low_time_1 = 0;
    }
}

void alarm_high_hum() {
    static unsigned long alert_high_time_hum_0 = 0;
    static unsigned long alert_high_time_hum_1 = 0;

    if (last_hum_0 > alarm_max_hum_0) {
        if (millis() - alert_high_time_hum_0 > ALARM_REPEAT_INTERVAL) {
            String message = "⚠️ *Тревога! Высокая влажность*\n";
            message += "💧 У " + sensor_0_name + "!\n";
            message += "Текущая: " + String(last_hum_0, 1) + " %\n";
            sendAdminMessage(message);
            alert_high_time_hum_0 = millis();
        }
    } else if (last_hum_0 <= alarm_max_hum_0) {
        alert_high_time_hum_0 = 0;
    }

    if (last_hum_1 > alarm_max_hum_1) {
        if (millis() - alert_high_time_hum_1 > ALARM_REPEAT_INTERVAL) {
            String message = "⚠️ *Тревога! Высокая влажность*\n";
            message += "💧 У " + sensor_1_name + "!\n";
            message += "Текущая: " + String(last_hum_1, 1) + " %\n";
            sendAdminMessage(message);
            alert_high_time_hum_1 = millis();
        }
    } else if (last_hum_1 <= alarm_max_hum_1) {
        alert_high_time_hum_1 = 0;
    }
}

void alarm_low_hum() {
    static unsigned long alert_low_time_hum_0 = 0;
    static unsigned long alert_low_time_hum_1 = 0;

    if (last_hum_0 < alarm_min_hum_0) {
        if (millis() - alert_low_time_hum_0 > ALARM_REPEAT_INTERVAL) {
            String message = "⚠️ *Тревога! Низкая влажность*\n";
            message += "❄️ У " + sensor_0_name + "!\n";
            message += "Текущая: " + String(last_hum_0, 1) + " %\n";
            sendAdminMessage(message);
            alert_low_time_hum_0 = millis();
        }
    } else if (last_hum_0 >= alarm_min_hum_0) {
        alert_low_time_hum_0 = 0;
    }

    if (last_hum_1 < alarm_min_hum_1) {
        if (millis() - alert_low_time_hum_1 > ALARM_REPEAT_INTERVAL) {
            String message = "⚠️ *Тревога! Низкая влажность*\n";
            message += "❄️ У " + sensor_1_name + "!\n";
            message += "Текущая: " + String(last_hum_1, 1) + " %\n";
            sendAdminMessage(message);
            alert_low_time_hum_1 = millis();
        }
    } else if (last_hum_1 >= alarm_min_hum_1) {
        alert_low_time_hum_1 = 0;
    }
}