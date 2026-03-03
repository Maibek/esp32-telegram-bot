#pragma once
#include <UniversalTelegramBot.h>
#include <Arduino.h>

#include "secret.h"
#include "config.h"
#include "read_sensor.h"
#include "api_handler.h"


WiFiClientSecure client;
UniversalTelegramBot bot(TOKEN_BOT, client);

enum KeyboardType {
    MAIN_KEYBOARD,
    RELAY_KEYBOARD,
    SENSOR_KEYBOARD,
    STATUS_KEYBOARD
};

String getMainKeyboard() {
    String keyboardJson = "[[{";
    keyboardJson += "\"text\":\"🖥️ Статус системы\", \"callback_data\": \"/status\"";
    keyboardJson += "}],[{";
    keyboardJson += "\"text\":\"🌡️ Датчики\", \"callback_data\": \"/sensors\"";
    keyboardJson += "}],[{";
    keyboardJson += "\"text\":\"⚙️ Управление\", \"callback_data\": \"/control\"";
    keyboardJson += "}]]";
    return keyboardJson;
}

String getControlKeyboard() {
    String keyboardJson = "[[{";
    keyboardJson += "\"text\":\"🔥 Подогрев у " + sensor_0 + ": " + String(!relay0_status ? "🟢" : "🔴") + "\",";
    keyboardJson += "\"callback_data\":\"/relay0\"";
    keyboardJson += "}],[{";
    keyboardJson += "\"text\":\"🔥 Подогрев у " + sensor_1 + ": " + String(!relay1_status ? "🟢" : "🔴") + "\",";
    keyboardJson += "\"callback_data\":\"/relay1\"";
    keyboardJson += "}],[{";
    keyboardJson += "\"text\":\"💡 Свет: " + String(light_status ? "🟢" : "🔴") + "\",";
    keyboardJson += "\"callback_data\":\"/light\"";
    keyboardJson += "}],[{";
    keyboardJson += "\"text\":\"🔙 Назад\",";
    keyboardJson += "\"callback_data\":\"/back\"";
    keyboardJson += "}]]";
    return keyboardJson;
}


String getSensorKeyboard() {
    String keyboardJson = "[[{";
    keyboardJson += "\"text\":\"🌡️ " + sensor_0 + "\", \"callback_data\": \"/sensor0\"";
    keyboardJson += "}],[{";
    keyboardJson += "\"text\":\"🌡️ " + sensor_1 + "\", \"callback_data\": \"/sensor1\"";
    keyboardJson += "}],[{";
    keyboardJson += "\"text\":\"🔙 Назад\", \"callback_data\": \"/back\"";
    keyboardJson += "}]]";
    return keyboardJson;
}

String getStatusKeyboard() {
    String keyboardJson = "[[{";
    keyboardJson += "\"text\":\"🔄 Обновить\",";
    keyboardJson += "\"callback_data\":\"/status\"";
    keyboardJson += "}],[{";
    keyboardJson += "\"text\":\"🔙 Назад\",";
    keyboardJson += "\"callback_data\":\"/back\"";
    keyboardJson += "}]]";
    return keyboardJson;
}

String getKeyboard(KeyboardType type = MAIN_KEYBOARD) {
    switch(type) {
        case RELAY_KEYBOARD:
        return getControlKeyboard();
        case SENSOR_KEYBOARD:
        return getSensorKeyboard();
        case STATUS_KEYBOARD:
        return getStatusKeyboard();
        case MAIN_KEYBOARD:
        default:
        return getMainKeyboard();
    }
}

void sendStatus(String chat_id) {
    String status = "🖥️ *Статус системы*\n";
    status += "📶 Сигнал: " + String(WiFi.RSSI()) + " dBm\n";
    status += "💽 Память: " + String(ESP.getFreeHeap() / 1024.0, 1) + " KB свободно\n";
    status += "🕒 Аптайм: " + String(millis() / 1000 / 60) + " минут\n";
    status += "🌡️ Температура ядра: " + String((temprature_sens_read() - 32) / 1.8f) + " °C\n";
    status += "🔥 *Статус подогрева: *\n";
    status += String("☀️ Подогрев у ") + sensor_0 + (!relay0_status ? " включен" : " выключен") + "!\n";
    status += String("☀️ Подогрев у ") + sensor_1 + (!relay1_status ? " включен" : " выключен") + "!\n";
    status += "💡 *Статус освещения: *\n";
    status += String("🌟 Освещение у муравьев ") + (light_status ? "включено" : "выключено") + "!\n";
    bot.sendMessage(chat_id, status, "Markdown");
}

void handleNewMessages(int numNewMessages){
    for (int i = 0; i < numNewMessages; i++) {
        int message_id = bot.messages[i].message_id;
        String chat_id = String(bot.messages[i].chat_id);
        String text = bot.messages[i].text;
        String from_name = bot.messages[i].from_name;
        String message = "";
        Serial.println("Тип сообщения: " + bot.messages[i].type);
        Serial.println("ID сообщения: " + String(bot.messages[i].message_id));
        Serial.println("ID чата: " + String(bot.messages[i].chat_id));
        Serial.println("Текст: " + bot.messages[i].text);

        if (text.startsWith("{")) {

            String action = "";
            int relay_id = -1;
            String state = "";
            int sensor_id = -1;

            parseJsonCommand(text, action, relay_id, state, sensor_id);
            executeApiCommand(action, relay_id, state, sensor_id, chat_id);

            continue;
        }
        
        if (bot.messages[i].type == "callback_query"){
            Serial.println("Обработка callback: " + text);
      
        if (text == "/relay0") {
            relay0_status = !relay0_status;
            digitalWrite(RELAY0_PIN, relay0_status);
            bot.answerCallbackQuery(bot.messages[i].query_id, String("Подогрев у ") + sensor_0 + (!relay0_status ? " включен" : " выключен"));
            message = String("Подогрев у ") + sensor_0 + (!relay0_status ? " включен" : " выключен");
            bot.sendMessageWithInlineKeyboard(chat_id, message, "Markdown", getKeyboard(RELAY_KEYBOARD), message_id);
        }
        else if (text == "/relay1") {
            relay1_status = !relay1_status;
            digitalWrite(RELAY1_PIN, relay1_status);
            bot.answerCallbackQuery(bot.messages[i].query_id, String("Подогрев у ") + sensor_1 + (!relay1_status ? " включен" : " выключен"));
            message = String("Подогрев у ") + sensor_1 + (!relay1_status ? " включен" : " выключен");
            bot.sendMessageWithInlineKeyboard(chat_id, message, "Markdown", getKeyboard(RELAY_KEYBOARD), message_id);
        }
        else if (text == "/light") {
            light_status = !light_status;
            digitalWrite(LIGHT0_PIN, light_status);
            digitalWrite(LIGHT1_PIN, light_status);
            bot.answerCallbackQuery(bot.messages[i].query_id, String("Подсветка ") + (light_status ? "включена" : "выключена"));
            message = String("Подсветка ") + (light_status ? "включена" : "выключена");
            bot.sendMessageWithInlineKeyboard(chat_id, message, "Markdown", getKeyboard(RELAY_KEYBOARD), message_id);
        }
        else if (text == "/sensor0") {
            String message = "🐜 Климат у " + sensor_0 + "\n";      
            message += "🌡️ Температура: " + String(last_temp0, 2) + " °C\n";
            message += "💧 Влажность: " + String(last_hum0, 2) + " %";
            bot.sendMessageWithInlineKeyboard(chat_id, message, "Markdown", getKeyboard(SENSOR_KEYBOARD), message_id);
        }
        else if (text == "/sensor1") {
            String message = "🐜 Климат у " + sensor_1 + "\n";      
            message += "🌡️ Температура: " + String(last_temp1, 2) + " °C\n";
            message += "💧 Влажность: " + String(last_hum1, 2) + " %";
            bot.sendMessageWithInlineKeyboard(chat_id, message, "Markdown", getKeyboard(SENSOR_KEYBOARD), message_id);
        }
        else if (text == "/status") {
            sendStatus(chat_id);
            bot.sendMessageWithInlineKeyboard(chat_id, "Главное меню:", "Markdown", getKeyboard(), message_id);
        }
        else if (text == "/sensors") {
            message = "📊 Выберите датчик:";
            bot.sendMessageWithInlineKeyboard(chat_id, message, "Markdown", getKeyboard(SENSOR_KEYBOARD), message_id);
        }
        else if (text == "/control") {
            message = "⚙️ Управление устройствами:";
            bot.sendMessageWithInlineKeyboard(chat_id, message, "Markdown", getKeyboard(RELAY_KEYBOARD), message_id);
        }
        else if (text == "/back") {
            message = "Главное меню:";
            bot.sendMessageWithInlineKeyboard(chat_id, message, "Markdown", getKeyboard(), message_id);
        }
        else {
            bot.sendMessage(chat_id, "Неизвестная команда", "Markdown");
        }
        }
        else if (text == "/start") {
            message = String("Добро пожаловать, ") + from_name + "!\nИспользуйте кнопки для управления:";
            bot.sendMessageWithInlineKeyboard(chat_id, message, "Markdown", getKeyboard());
        }
        else if (text == "/status") {
            sendStatus(chat_id);
        }
        else if (text == "/keyboard") {
            bot.sendMessageWithInlineKeyboard(chat_id, "Выберите действие:", "Markdown", getKeyboard());
        }
        else {
            message = "Неверная команда.\n";
            message += "Используйте меню:";
            bot.sendMessageWithInlineKeyboard(chat_id, message, "Markdown", getKeyboard());
        }
        delay(50);
    }
}