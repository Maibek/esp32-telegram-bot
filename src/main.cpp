#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <Adafruit_HTU21DF.h>
#include <Wire.h>

// Данные для подключения к Wi-Fi и токен бота
#include "wifi_data.h"
#include "token_bot.h"

const unsigned long BOT_MTBS = 1000;
const int LED_PIN = 2; // Временная мера для отладки. В будущем это не нужно.
const int SDA0_PIN = 18;
const int SCL0_PIN = 19;
const int SDA1_PIN = 22;
const int SCL1_PIN = 23;

extern TwoWire Wire;
extern TwoWire Wire1;

Adafruit_HTU21DF sensor0;
Adafruit_HTU21DF sensor1;
WiFiClientSecure client;
UniversalTelegramBot bot(TOKEN_BOT, client);
unsigned long bot_lasttime = 0;

// Глобальные переменные для показаний датчика
float last_temp0 = NAN;
float last_hum0 = NAN;
float last_temp1 = NAN;
float last_hum1 = NAN;
unsigned long last_sensor_read = 0;

void connectToWiFi() {
  Serial.println("Подключение к Wi-Fi...");
  WiFi.disconnect(true);
  delay(1000);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  WiFi.setSleep(false); // Отключение энергосбережения WiFi
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    attempts++;
    
    if (attempts > 30) {
      Serial.println("\nПревышено ожидание. Перезапуск...");
      delay(2000);
      ESP.restart();
    }
  }
  
  Serial.println("\nУспешное подключение к точке доступа ");
  Serial.println(WIFI_SSID);
  Serial.print("IP адрес: ");
  Serial.println(WiFi.localIP());

  // Синхронизация времени для SSL. Временное решение, без него не всегда бот получает сообщения. Необходимо подумать. Иногда синхронизация не проходит, возможно сделать асинхронную синхронизацию.
  configTime(0, 0, "pool.ntp.org");
  Serial.println("Ожидание синхронизации времени...");
  time_t now = time(nullptr);
  while (now < 24 * 3600) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println("\nВремя синхронизировано!");
}

void safe_sensor_read() {
  if (millis() - last_sensor_read > 5000) {
    // Чтение первого датчика
    float t0 = sensor0.readTemperature();
    float h0 = sensor0.readHumidity();
    
    // Чтение второго датчика
    float t1 = sensor1.readTemperature();
    float h1 = sensor1.readHumidity();
    
    Serial.print("Датчик 0: Температура = ");
    Serial.print(t0);
    Serial.print(", Влажность = ");
    Serial.println(h0);
    
    Serial.print("Датчик 1: Температура = ");
    Serial.print(t1);
    Serial.print(", Влажность = ");
    Serial.println(h1);
    
    // Сохранение корректных значений
    if (!isnan(t0)) last_temp0 = t0;
    if (!isnan(h0)) last_hum0 = h0;
    if (!isnan(t1)) last_temp1 = t1;
    if (!isnan(h1)) last_hum1 = h1;
    
    last_sensor_read = millis();
  }
}

void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    Serial.print("Обработка сообщения от: ");
    Serial.println(bot.messages[i].chat_id);
    Serial.print("Текст: ");
    Serial.println(bot.messages[i].text);

// В будущем надо сделать нормальные кнопки, без необходимости отправлять команду в бота. Так же необходимо удалять переписку, чтобы не захламлять бота.
    if (bot.messages[i].text == "/control") {
      String keyboardJson = "[[\"/Meteo Structor\", \"/Meteo Nicobarensis\"]]";
      bot.sendMessageWithReplyKeyboard(bot.messages[i].chat_id, "Выберите действие:", "", keyboardJson, true);
    }
    if (bot.messages[i].text.equalsIgnoreCase("/Meteo Structor")) {
      String message = "Климат у Structor\n";      
      if (!isnan(last_temp0)) {
        message += "Температура: " + String(last_temp0, 2) + " °C\n";
      }
      if (!isnan(last_hum0)) {
        message += "Влажность: " + String(last_hum0, 2) + " %";
      }
      bot.sendMessage(bot.messages[i].chat_id, message);
    }
    // Команда для второго датчика
    else if (bot.messages[i].text.equalsIgnoreCase("/Meteo Nicobarensis")) {
      String message = "Климат у Nicobarensis\n";      
      if (!isnan(last_temp1)) {
        message += "Температура: " + String(last_temp1, 2) + " °C\n";
      }
      if (!isnan(last_hum1)) {
        message += "Влажность: " + String(last_hum1, 2) + " %";
      }
      bot.sendMessage(bot.messages[i].chat_id, message);
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Wire.begin(SDA0_PIN, SCL0_PIN);
  Wire.setTimeOut(250); // Таймаут I2C. Без него датчик не всегда успевает инициализироваться.
  
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT);
  pinMode(LED_PIN, OUTPUT); // Временная мера для отладки. В будущем это не нужно.

  Wire.begin(SDA0_PIN, SCL0_PIN);
  Wire.setTimeOut(250);
  
  // Инициализация второй шины I2C
  Wire1.begin(SDA1_PIN, SCL1_PIN);
  Wire1.setTimeOut(250);

  // Инициализация датчика 0 на первой шине
  if (!sensor0.begin(&Wire)) {
    Serial.println("Ошибка инициализации HTU21DF #0!");
  } else {
    Serial.println("HTU21DF #0 инициализирован");
  }

  // Инициализация датчика 1 на второй шине
  if (!sensor1.begin(&Wire1)) {
    Serial.println("Ошибка инициализации HTU21DF #1!");
  } else {
    Serial.println("HTU21DF #1 инициализирован");
  }

  connectToWiFi();
  // Для диагностики, в будущем убрать.
  digitalWrite(LED_PIN, HIGH);
  delay(1000);
  digitalWrite(LED_PIN, LOW);
}

void loop() {
  safe_sensor_read(); // Безопасное чтение датчика

  // Проверка соединения WiFi
  static unsigned long lastCheck = 0;
  if (millis() - lastCheck > 10000) {
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("Переподключение к WiFi...");
      WiFi.disconnect();
      WiFi.reconnect();
    }
    lastCheck = millis();
  }

  // Обработка сообщений Telegram. В будущем лучше реализовать webhook. Так как иногда бот не сбрасывает значения сообщений и начинает спамить в ответ на запрос.
  if (millis() - bot_lasttime > BOT_MTBS) {
    Serial.println("Проверка сообщений Telegram...");
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    if (numNewMessages > 0) {
      Serial.println("Найдено сообщений: " + String(numNewMessages));
      handleNewMessages(numNewMessages);
    }
    
    // Нужно для отладки. После окончания проекта можно будет удалить.
    bot_lasttime = millis();
    Serial.println("Свободная память: " + String(ESP.getFreeHeap()));
  }
}