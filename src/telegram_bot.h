#pragma once

#include <FastBot2.h>
#include <Arduino.h>
#include <map>

#include "secret.h"
#include "config.h"
#include "read_sensor.h"
#include "api_handler.h"
#include "settings.h"

using namespace fb;

FastBot2 bot(TOKEN_BOT);

struct UserState {
    String menu = "main";
};
std::map<String, UserState> userStates;


String escapeMarkdown(String text) {
    text.replace("_", "\\_");
    text.replace("*", "\\*");
    text.replace("[", "\\[");
    text.replace("]", "\\]");
    text.replace("(", "\\(");
    text.replace(")", "\\)");
    text.replace("~", "\\~");
    text.replace("`", "\\`");
    text.replace(">", "\\>");
    text.replace("#", "\\#");
    text.replace("+", "\\+");
    text.replace("-", "\\-");
    text.replace("=", "\\=");
    text.replace("|", "\\|");
    text.replace("{", "\\{");
    text.replace("}", "\\}");
    text.replace(".", "\\.");
    text.replace("!", "\\!");
    return text;
}


Keyboard getMainKeyboard() {
    Keyboard kb;
    kb.addButton("Статус системы");
    kb.newRow();
    kb.addButton("Датчики");
    kb.newRow();
    kb.addButton("Управление");
    kb.newRow();
    kb.addButton("Настройки оповещений");
    return kb;
}

Keyboard getControlKeyboard() {
    Keyboard kb;

    String r0 = String("Подогрев у ") + sensor_0_name + ": " + (!relay_0_status ? "🟢" : "🔴");
    String r1 = String("Подогрев у ") + sensor_1_name + ": " + (!relay_1_status ? "🟢" : "🔴");
    String lt = String("Свет: ") + (!light_status ? "🟢" : "🔴");

    kb.addButton(r0);
    kb.newRow();
    kb.addButton(r1);
    kb.newRow();
    kb.addButton(lt);
    kb.newRow();
    kb.addButton("Назад");
    return kb;
}

Keyboard getSensorKeyboard() {
    Keyboard kb;
    kb.addButton(sensor_0_name);
    kb.newRow();
    kb.addButton(sensor_1_name);
    kb.newRow();
    kb.addButton("Назад");
    return kb;
}

Keyboard getStatusKeyboard() {
    Keyboard kb;
    kb.addButton("Обновить");
    kb.newRow();
    kb.addButton("Назад");
    return kb;
}

// Нужно добавить все действия для этой клавиатуры!!!!
Keyboard getIntervalKeyboard() {
    Keyboard kb;
    kb.addButton("Сброс настроек оповещения");
    kb.newRow();
    kb.addButton("Настройка всех оповещений");
    kb.newRow();
    kb.addButton("Настройка оповещений по высокой температуре");
    kb.newRow();
    kb.addButton("Настройка оповещений по низкой температуре");
    kb.newRow();
    kb.addButton("Настройка оповещений по высокой влажности");
    kb.newRow();
    kb.addButton("Настройка оповещений по низкой влажности");
    kb.newRow();
    kb.addButton("Назад");
    return kb;
}



void sendStatus(String chat_id) {
    String status = "🖥️ Статус системы\n";
    status += "📶 Сигнал: " + String(WiFi.RSSI()) + " dBm\n";
    status += "💽 Память: " + String(ESP.getFreeHeap() / 1024.0, 1) + " KB свободно\n";
    status += "🕒 Аптайм: " + String(millis() / 60000) + " мин\n";
    status += "🌡️ Температура ядра: " + String((temprature_sens_read() - 32) / 1.8f, 1) + " °C\n\n";
    status += "🔥 *Подогрев:*\n";
    status += "☀️ " + sensor_0_name + ": " + (relay_0_status == LOW ? "включён" : "выключен") + "\n";
    status += "☀️ " + sensor_1_name + ": " + (relay_1_status == LOW ? "включён" : "выключен") + "\n";
    status += "💡 Освещение: ";
    status += "🌟 " + String(light_status == LOW ? "включено" : "выключено") + "\n";

    fb::Message m(status, chat_id);
    fb::Keyboard kb = getStatusKeyboard();
    m.setKeyboard(&kb);
    bot.sendMessage(m);
}

void sendInterval(String chat_id) {
    String status = "⏱️ Настройки оповещений\n";
    status += "Оповещение по датчикам: " + String(alarm_sensor_interval) + " минут\n";
    status += "Оповещение по высокой температуре: \n";
    status += String(alarm_high_temp_interval) + " мин\n";
    status += "Оповещение по низкой температуре: \n";
    status += String(alarm_low_temp_interval) + " мин\n";
    status += "Оповещение по высокой влажности: \n";
    status += String(alarm_high_hum_interval) + " мин\n";
    status += "Оповещение по низкой  влажности: \n";
    status += String(alarm_low_hum_interval) + " мин\n";

    fb::Message m(status, chat_id);
    fb::Keyboard kb = getIntervalKeyboard();
    m.setKeyboard(&kb);
    bot.sendMessage(m);
}

// Основной обработчик
void handleUpdate(fb::Update& u) {
    if (!u.isMessage()) return;

    MessageRead msg = u.message();
    String chatID = msg.chat().id();
    String text   = msg.text();

    // Защита от дубликатов
    static uint32_t lastUpdateID = 0;
    uint32_t thisID = u.id();
    if (thisID <= lastUpdateID) return;
    lastUpdateID = thisID;

    text.trim();
    Serial.println("Получен текст: [" + text + "] от chatID: " + chatID);

    Serial.println("Получен текст: [" + text + "]");
    Serial.print("Длина: "); Serial.println(text.length());
    Serial.print("Hex: ");
    for (unsigned int i = 0; i < text.length(); i++) {
        Serial.print("0x");
        Serial.print((uint8_t)text[i], HEX);
        Serial.print(" ");
    }
    Serial.println();

    if (text == "/start") {
        fb::Message m("Добро пожаловать!\nВыберите действие:", chatID);
        fb::Keyboard kb = getMainKeyboard();
        m.setKeyboard(&kb);
        bot.sendMessage(m);
        return;
    }

    if (text == "Статус системы" || text == "Обновить" || text == "/status") {
        sendStatus(chatID);
        return;
    }

    if (text == "Настройки оповещений" || text == "Оповещения" || text == "/interval") {
        sendInterval(chatID);
        return;
    }

    if (text == "Сброс настроек оповещения" || text == "Сброс" || text == "/Reset settings") {
        saveDefaults();
        fb::Message m("Настройки оповещений сброшены", chatID);
        fb::Keyboard kb = getIntervalKeyboard();
        m.setKeyboard(&kb);
        bot.sendMessage(m);
        return;
    }

    if (text == "Датчики" || text == "/sensors") {
        fb::Message m("Выберите датчик:", chatID);
        fb::Keyboard kb = getSensorKeyboard();
        m.setKeyboard(&kb);
        bot.sendMessage(m);
        return;
    }

    if (text == "Управление" || text == "/control") {
        fb::Message m("Управление устройствами:", chatID);
        fb::Keyboard kb = getControlKeyboard();
        m.setKeyboard(&kb);
        bot.sendMessage(m);
        return;
    }

    if (text == "Назад" || text == "/back") {
        fb::Message m("Главное меню:", chatID);
        fb::Keyboard kb = getMainKeyboard();
        m.setKeyboard(&kb);
        bot.sendMessage(m);
        return;
    }

    if (text == sensor_0_name) {
        String m_text = "🐜 Климат у " + sensor_0_name + "\n";
        m_text += "🌡️ Температура: " + String(last_temp_0, 2) + " °C\n";
        m_text += "💧 Влажность: " + String(last_hum_0, 2) + " %";

        fb::Message m(m_text, chatID);
        fb::Keyboard kb = getSensorKeyboard();
        m.setKeyboard(&kb);
        bot.sendMessage(m);
        return;
    }
    if (text == sensor_1_name) {
        String m_text = "🐜 Климат у " + sensor_1_name + "\n";
        m_text += "🌡️ Температура: " + String(last_temp_1, 2) + " °C\n";
        m_text += "💧 Влажность: " + String(last_hum_1, 2) + " %";

        fb::Message m(m_text, chatID);
        fb::Keyboard kb = getSensorKeyboard();
        m.setKeyboard(&kb);
        bot.sendMessage(m);
        return;
    }

    // Управление — реле
    if (text.startsWith("Подогрев у ")) {
        String prefix = "Подогрев у ";
        String rest = text.substring(prefix.length());
        int colonPos = rest.lastIndexOf(':');
        if (colonPos > 0) {
            String namePart = rest.substring(0, colonPos);
            namePart.trim();
            if (namePart == sensor_0_name) {
                relay_0_status = !relay_0_status;
                digitalWrite(RELAY0_PIN, relay_0_status);
                String reply = "Подогрев у " + sensor_0_name + (relay_0_status == LOW ? " включён" : " выключен");
                Serial.println("Переключено реле 0 → " + reply);

                fb::Message m(reply, chatID);
                fb::Keyboard kb = getControlKeyboard();
                m.setKeyboard(&kb);
                bot.sendMessage(m);
                return;
            }
            if (namePart == sensor_1_name) {
                relay_1_status = !relay_1_status;
                digitalWrite(RELAY1_PIN, relay_1_status);
                String reply = "Подогрев у " + sensor_1_name + (relay_1_status == LOW ? " включён" : " выключен");
                Serial.println("Переключено реле 1 → " + reply);

                fb::Message m(reply, chatID);
                fb::Keyboard kb = getControlKeyboard();
                m.setKeyboard(&kb);
                bot.sendMessage(m);
                return;
            }
        }
    }

    if (text.startsWith("Свет:")) {
        light_status = !light_status;
        digitalWrite(LIGHT0_PIN, light_status);
        digitalWrite(LIGHT1_PIN, light_status);
        String reply = "Освещение " + String(light_status == LOW ? "включено" : "выключено");
        Serial.println("Переключён свет → " + reply);

        fb::Message m(reply, chatID);
        fb::Keyboard kb = getControlKeyboard();
        m.setKeyboard(&kb);
        bot.sendMessage(m);
        return;
    }

    if (text.startsWith("{")) {
        String action; int rid = -1; String state; int sid = -1;
        parseJsonCommand(text, action, rid, state, sid);
        executeApiCommand(action, rid, state, sid, chatID);
        return;
    }

    // Fallback
    Serial.println("Неизвестный текст: [" + text + "]");
    fb::Message m("Не понял команду… Используй кнопки меню.", chatID);
    fb::Keyboard kb = getMainKeyboard();
    m.setKeyboard(&kb);
    bot.sendMessage(m);
}

void sendAdminMessage(String message) {
    fb::Message m(message, CHAT_ADMIN);
    bot.sendMessage(m);
}