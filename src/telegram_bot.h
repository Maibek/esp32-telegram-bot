#pragma once
#include <UniversalTelegramBot.h>

#include "wifi_connect.h"
#include "token_bot.h"
#include "read_sensor.h"
#include "config.h"

WiFiClientSecure client;
UniversalTelegramBot bot(TOKEN_BOT, client);

void sendMainMenu(String chat_id) {
    // Главное меню с inline-кнопками
    String keyboardJson = R"([
        [{"text": "🐜 Данные Structor", "callback_data": "/meteo_structor"}],
        [{"text": "🐜 Данные Nicobarensis", "callback_data": "/meteo_nicobarensis"}],
        [{"text": "☀️ Включить подогрев", "callback_data": "/relay_on"}, {"text": "❄️ Выключить подогрев", "callback_data": "/relay_off"}],
        [{"text": "💡 Включить подсветку", "callback_data": "/light_on"}, {"text": "🏮 Выключить подсветку", "callback_data": "/light_off"}],
        [{"text": "ℹ️ Статус системы", "callback_data": "/status"}, {"text": "🔄 Обновить", "callback_data": "/update"}]
        ])";
  
    bot.sendMessageWithInlineKeyboard(chat_id, "Выберите действие:", "", keyboardJson);
}

void sendStatus(String chat_id) {
    static String status_relay0 = "";
    static String status_relay1 = "";
    static String status_light0 = "";
    static String status_light1 = "";
    if (digitalRead(RELAY0_PIN) == LOW){
        status_relay0 = "включен";
    }
    if (digitalRead(RELAY0_PIN) == HIGH){
        status_relay0 = "выключен";
    }
    if (digitalRead(RELAY1_PIN) == LOW){
        status_relay1 = "включен";
    }
    if (digitalRead(RELAY1_PIN) == HIGH){
        status_relay1 = "выключен";
    }
    if (digitalRead(LIGHT0_PIN) == LOW){
        status_light0 = "включено";
    }
    if (digitalRead(LIGHT0_PIN) == HIGH){
        status_light0 = "выключено";
    }
    if (digitalRead(LIGHT1_PIN) == LOW){
        status_light1 = "включено";
    }
    if (digitalRead(LIGHT1_PIN) == HIGH){
        status_light1 = "выключено";
    }
    String status = "🖥️ *Статус системы*\n";
    status += "📶 Сигнал: " + String(WiFi.RSSI()) + " dBm\n";
    status += "💽 Память: " + String(ESP.getFreeHeap() / 1024.0, 1) + " KB свободно\n";
    status += "🕒 Аптайм: " + String(millis() / 1000 / 60) + " минут\n";
    status += "🌡️ Температура ядра: " + String((temprature_sens_read() - 32) / 1.8f) + " °C\n";
    status += "🔥 *Статус подогрева: *\n";
    status += "☀️ Подогрев у " + sensor_0 + " " + status_relay0 + "!\n";
    status += "☀️ Подогрев у " + sensor_1 + " " +  status_relay1 + "!\n";
    status += "💡 *Статус освещения: *\n";
    status += "🌟 Освещение у " + sensor_0 + " " +  status_light0 + "!\n";
    status += "🌟 Освещение у " + sensor_1 + " " +  status_light1 + "!\n";    
    bot.sendMessage(chat_id, status, "Markdown");
}


void handleNewMessages(int numNewMessages) {
    for (int i = 0; i < numNewMessages; i++) {
        String chat_id = String(bot.messages[i].chat_id);
        String text = bot.messages[i].text;
        String from_name = bot.messages[i].from_name;
        
        // Отладочная информация
        Serial.print("Получено сообщение от ");
        Serial.print(from_name);
        Serial.print(" (");
        Serial.print(chat_id);
        Serial.print("): ");
        Serial.println(text);

        // Обработка команды /start
        if (text == "/start") {
            String welcome = "Привет, " + from_name + "!\n";
            welcome += "Я бот для мониторинга микроклимата.\n";
            welcome += "Используй меню ниже для получения данных.";
            bot.sendMessage(chat_id, welcome);
            sendMainMenu(chat_id);
            continue;
        }

        // Обработка команд (без callback префикса)
        if (text.equalsIgnoreCase("/meteo_structor")) {
            String message = "🐜 Климат у *" + sensor_0 + "!\n";      
            message += "🌡️ Температура: " + String(last_temp0, 2) + " °C\n";
            message += "💧 Влажность: " + String(last_hum0, 2) + " %";
            bot.sendMessage(chat_id, message, "Markdown");
        }
        else if (text.equalsIgnoreCase("/meteo_nicobarensis")) {
            String message = "🐜 Климат у " + sensor_1 + "\n";      
            message += "🌡️ Температура: " + String(last_temp1, 2) + " °C\n";
            message += "💧 Влажность: " + String(last_hum1, 2) + " %";
            bot.sendMessage(chat_id, message, "Markdown");
        }
        else if (text.equalsIgnoreCase("/status")) {
            sendStatus(chat_id);
        }
        else if (text.equalsIgnoreCase("/update")) {
            bot.sendMessage(chat_id, "Данные обновлены!");
            sendMainMenu(chat_id);
        }
        else if (text.equalsIgnoreCase("/help")) {
            sendMainMenu(chat_id);
        }
        else if (text.equalsIgnoreCase("/local_ip")){
            String message = "Локальный IP адресс: " + String(WiFi.localIP()) + "\n";
            bot.sendMessage(chat_id, message, "Markdown");
        }
        else if (text.equalsIgnoreCase("/relay_on")) {
            if (temp_high == false && last_temp0 < max_temp && last_temp1 < max_temp){ 
                digitalWrite(RELAY0_PIN, LOW);
                digitalWrite(RELAY1_PIN, LOW);
                String message = "🐜 *Подогрев у всех музавьев включен!*\n";
                message += "🌡️ Температура у " + sensor_0 + ": " + String(last_temp0, 2) + " °C\n";
                message += "🌡️ Температура у " + sensor_1 + ": " + String(last_temp1, 2) + " °C\n";
                bot.sendMessage(chat_id, message, "Markdown");
                temp_high = true;
            }
            else if (last_temp0 >= max_temp){
                String message = "☀️ *У structor и так тепло!*\n";
                message += "🌡️ Температура у " + sensor_0 + ": " + String(last_temp0, 2) + " °C\n";
                message += "🌡️ Температура у " + sensor_1 + ": " + String(last_temp0, 2) + " °C\n";
                bot.sendMessage(chat_id, message, "Markdown");
            }
            else if (last_temp1 >= max_temp){
                String message = "☀️ *У nicobarensis и так тепло!*\n";
                message += "🌡️ Температура у " + sensor_0 + ": " + String(last_temp0, 2) + " °C\n";
                message += "🌡️ Температура у " + sensor_1 + ": " + String(last_temp0, 2) + " °C\n";
                bot.sendMessage(chat_id, message, "Markdown");
            }
            else if (temp_high = true){
                String message = "🔌 *Подогрев уже включен!*\n";
                message += "🌡️ Температура у " + sensor_0 + ": " + String(last_temp0, 2) + " °C\n";
                message += "🌡️ Температура у " + sensor_1 + ": " + String(last_temp1, 2) + " °C\n";
                bot.sendMessage(chat_id, message, "Markdown");
            }
            else {
                String message = "⚠️ *Подогрев не получилось включить!*\n";
                bot.sendMessage(chat_id, message, "Markdown");

            }
        }
        else if (text.equalsIgnoreCase("/relay_off")) {
            digitalWrite(RELAY0_PIN, HIGH);
            digitalWrite(RELAY1_PIN, HIGH);
            String message = "✅ *Подогрев у всех музавьев выключен!*\n";
            message += "🌡️ Температура у " + sensor_0 + ": " + String(last_temp0, 2) + " °C\n";
            message += "🌡️ Температура у " + sensor_1 + ": " + String(last_temp1, 2) + " °C\n";
            bot.sendMessage(chat_id, message, "Markdown");
            temp_high = false;
        }
        else if (text.equalsIgnoreCase("/light_on")) {
            if (light == false){
                digitalWrite(LIGHT0_PIN, LOW);
                digitalWrite(LIGHT1_PIN, LOW);
                String message = "💡 Подсветка включена\n";
                bot.sendMessage(chat_id, message, "Markdown");
                light = true;
            }
            else {
                String message = "💡 Подсветка уже включена\n";
                bot.sendMessage(chat_id, message, "Markdown");
            }
        }
        else if (text.equalsIgnoreCase("/light_off")) {
            if (light == true){                
                digitalWrite(LIGHT0_PIN, HIGH);
                digitalWrite(LIGHT1_PIN, HIGH);
                String message = "🏮 Подсветка выключена\n";
                bot.sendMessage(chat_id, message, "Markdown");
                light = false;
            }
            else {
                String message = "🏮 Подсветка уже выключена\n";
                bot.sendMessage(chat_id, message, "Markdown");
            }
        }

        // Обработка callback-запросов
        else if (text.startsWith("/cb")) {
            String command = text.substring(3); // Удаляем префикс /cb
            command.trim();
        
            if (command.equalsIgnoreCase("/meteo_structor")) {
                String message = "🐜 Климат у " + sensor_0 + "\n";      
                message += "🌡️ Температура: " + String(last_temp0, 2) + " °C\n";
                message += "💧 Влажность: " + String(last_hum0, 2) + " %";
                bot.sendMessage(chat_id, message, "Markdown");
            }
            else if (command.equalsIgnoreCase("/meteo_nicobarensis")) {
                String message = "🐜 Климат у " + sensor_1 + "\n";      
                message += "🌡️ Температура: " + String(last_temp1, 2) + " °C\n";
                message += "💧 Влажность: " + String(last_hum1, 2) + " %";
                bot.sendMessage(chat_id, message, "Markdown");
            }
            else if (text.equalsIgnoreCase("/relay_on")) { 
                digitalWrite(RELAY0_PIN, LOW);
                digitalWrite(RELAY1_PIN, LOW);
                String message = "🐜 *Подогрев у всех музавьев включен!*\n";
                message += "🌡️ Температура у " + sensor_0 + ": " + String(last_temp0, 2) + " °C\n";
                message += "🌡️ Температура у " + sensor_1 + ": " + String(last_temp1, 2) + " °C\n";
                bot.sendMessage(chat_id, message, "Markdown");
                temp_high = true;
            }
            else if (text.equalsIgnoreCase("/relay_off")) {
                digitalWrite(RELAY0_PIN, HIGH);
                digitalWrite(RELAY1_PIN, HIGH);
                String message = "✅ *Подогрев у всех музавьев выключен!*\n";
                message += "🌡️ Температура у " + sensor_0 + ": " + String(last_temp0, 2) + " °C\n";
                message += "🌡️ Температура у " + sensor_1 + ": " + String(last_temp1, 2) + " °C\n";
                bot.sendMessage(chat_id, message, "Markdown");
                temp_high = false;
            }
            else if (text.equalsIgnoreCase("/light_on")) {
                if (light == false){
                    digitalWrite(LIGHT0_PIN, LOW);
                    digitalWrite(LIGHT1_PIN, LOW);
                    String message = "💡 Подсветка включена\n";
                    bot.sendMessage(chat_id, message, "Markdown");
                    light = true;
                }
                else {
                    String message = "💡 Подсветка уже включена\n";
                    bot.sendMessage(chat_id, message, "Markdown");
                }
            }
            else if (text.equalsIgnoreCase("/light_off")) {
                if (light == true){
                    digitalWrite(LIGHT0_PIN, HIGH);
                    digitalWrite(LIGHT1_PIN, HIGH);
                    String message = "🏮 Подсветка выключена\n";
                    bot.sendMessage(chat_id, message, "Markdown");
                    light = false;
                }
                else {
                    String message = "🏮 Подсветка уже выключена\n";
                    bot.sendMessage(chat_id, message, "Markdown");
                }
            }
            else if (command.equalsIgnoreCase("/status")) {
                sendStatus(chat_id);
            }
            else if (command.equalsIgnoreCase("/update")) {
                bot.sendMessage(chat_id, "Данные обновлены!");
                sendMainMenu(chat_id);
            }
        }
        else {
            // Неизвестная команда
            String response = "Неизвестная команда: " + text + "\n";
            response += "Используй /help для вызова меню";
            bot.sendMessage(chat_id, response);
        }
    }
}
void checkSensors(){
    static bool errorSent0 = false;
    static bool errorSent1 = false;
    if (isnan(last_temp0) || isnan(last_hum0)){
        if (!errorSent0){
            digitalWrite(RELAY0_PIN, HIGH);
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
            digitalWrite(RELAY1_PIN, HIGH);
            String message = "🔧 Датчик у " + sensor_1 + " не исправен\n";
            bot.sendMessage(CHAT_ID, message, "Markdown");
            errorSent1 = true;
        }
    }
    else {
        errorSent1 = false;
    }
    if (digitalRead(RELAY0_PIN) == HIGH && last_temp0 < min_temp){
        digitalWrite(RELAY0_PIN, LOW);
        String message = "✅ Подогрев автоматически включен у " + sensor_0 + "\n";
        message += "🌡️ Температура у " + sensor_0 + ": " + String(last_temp0, 2) + " °C\n";
        message += "🌡️ Температура у " + sensor_1 + ": " + String(last_temp1, 2) + " °C\n";
        bot.sendMessage(CHAT_ID, message, "Markdown");
    }
    if (digitalRead(RELAY1_PIN) == HIGH && last_temp1 < min_temp){
        digitalWrite(RELAY1_PIN, LOW);
        String message = "✅ Подогрев автоматически включен у " + sensor_1 + "\n";
        message += "🌡️ Температура у " + sensor_0 + ": " + String(last_temp0, 2) + " °C\n";
        message += "🌡️ Температура у " + sensor_1 + ": " + String(last_temp1, 2) + " °C\n";
        bot.sendMessage(CHAT_ID, message, "Markdown");
    }
    if (temp_high == true && (last_temp0 >= max_temp && last_temp1 >= max_temp)){
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