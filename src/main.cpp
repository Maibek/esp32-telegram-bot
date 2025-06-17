#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

// Данные для подключения к Wi-Fi и токен бота
#include "wifi_data.h"
#include "token_bot.h"

const unsigned long BOT_MTBS = 1000;
const int LED_PIN = 2;

WiFiClientSecure client;
UniversalTelegramBot bot(TOKEN_BOT, client);
unsigned long bot_lasttime = 0;

void connectToWiFi() {
  Serial.println("Подключение к Wi-Fi...");
  
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  
  Serial.print("SSID: ");
  Serial.println(WIFI_SSID);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    attempts++;
    
    if (attempts > 30) { // 15 секунд
      Serial.println("\nПревышено ожидание. Перезапуск соединения...");
      delay(2000);
      ESP.restart();
    }
  }
  
  Serial.println("\nУспешное подключение!");
  Serial.print("IP адрес: ");
  Serial.println(WiFi.localIP());
}

void handleNewMessages(int numNewMessages)
{
  for (int i = 0; i < numNewMessages; i++)
  {
    bot.sendMessage(bot.messages[i].chat_id, bot.messages[i].text, "New");

    if (bot.messages[i].text == "/control") {
      String keyboardJson = "[[\"/led_on\", \"/led_off\"]]";
      bot.sendMessageWithReplyKeyboard(bot.messages[i].chat_id, "Выберите действие:", "", keyboardJson, true);
}

    if (bot.messages[i].text.equalsIgnoreCase("/led_on")) {
      digitalWrite(LED_PIN, HIGH);
      bot.sendMessage(bot.messages[i].chat_id, "Светодиод включен!");
    }
    else if (bot.messages[i].text.equalsIgnoreCase("/led_off")) {
      digitalWrite(LED_PIN, LOW);
      bot.sendMessage(bot.messages[i].chat_id, "Светодиод выключен!");
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(2000); // Время для инициализации
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT);
  pinMode(LED_PIN, OUTPUT);

  connectToWiFi();

  digitalWrite(LED_PIN, HIGH);
  delay(1000);
  digitalWrite(LED_PIN, LOW);
}

void loop() {
  // Проверка соединения каждые 10 секунд. Необходимо в будущем поменять согласно будущему режиму сна и минимализации энергопотребления.
  static unsigned long lastCheck = 0;
  
  if (millis() - lastCheck > 10000) {
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("Wi-Fi подключение активно");
    } else {
      Serial.println("Потеряно соединение Wi-Fi! Переподключение...");
      WiFi.reconnect();
    }
    lastCheck = millis();
  }

  if (millis() - bot_lasttime > BOT_MTBS)
  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages){
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    bot_lasttime = millis();
    Serial.println("Free heap: " + String(ESP.getFreeHeap()));
  }
}