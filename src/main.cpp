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
const int SDA_PIN = 18;
const int SCL_PIN = 19;

Adafruit_HTU21DF htu = Adafruit_HTU21DF();
WiFiClientSecure client;
UniversalTelegramBot bot(TOKEN_BOT, client);
unsigned long bot_lasttime = 0;

// Глобальные переменные для показаний датчика
float last_temp = NAN;
float last_hum = NAN;
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
    Serial.print("Connect");
    Serial.print(".");
    attempts++;
    
    if (attempts > 30) {
      Serial.println("Превышено ожидание. Подключение к мобильной точке доступа Wi-Fi...");
      WiFi.disconnect(true);
      delay(2000);
      // Подключение к точке резервной точке доступа, сейчас сделано на мой телефон.
      WiFi.begin(WIFI_SSID_2, WIFI_PASS_2);
      WiFi.setSleep(false);
      if (attempts > 30) {
        Serial.println("\nПревышено ожидание. Перезапуск...");
        delay(2000);
        ESP.restart();
      }
    }
  }
  
  Serial.println("\nУспешное подключение к точке доступа ");
  Serial.print(WIFI_SSID);
  Serial.print("IP адрес: ");
  Serial.println(WiFi.localIP());

  // Синхронизация времени для SSL. Временное решение, без него не всегда бот получает сообщения.
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
    float t = htu.readTemperature();
    float h = htu.readHumidity();
    
    Serial.print("Температура = ");
    Serial.print(t);
    Serial.println("Влажность = ");
    Serial.print(h);
    
    // Проверка на корректность показаний датчика.
    if (!isnan(t)) last_temp = t;
    if (!isnan(h)) last_hum = h;
    
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
      String keyboardJson = "[[\"Meteo Structor\", \"Meteo Nicobarensis\"]]";
      bot.sendMessageWithReplyKeyboard(bot.messages[i].chat_id, "Выберите действие:", "", keyboardJson, true);
    }
    if (bot.messages[i].text.equalsIgnoreCase("Meteo Structor")) {
      
      String message = "Климат у Structor:";
      message += "\nТемпература: " + String(last_temp, 2) + " C";
      message += "\nВлажность: " + String(last_hum, 2) + " %";      
      
      bot.sendMessage(bot.messages[i].chat_id, message);
    }
    else if (bot.messages[i].text.equalsIgnoreCase("Meteo Nicobarensis")) {
      String message = "Заглушка";
      // Пока заглушка. После реализации подключения нескольких датчиков, необходимо будет подправить закоментирвоанный код
      // String message = "Климат у Nicobarensis";      
      // if (!isnan(last_temp)) {
      //   message += "\nТемпература: " + String(last_temp, 2) + " C";
      // }      
      // if (!isnan(last_hum)) {
      //   message += "\nВлажность: " + String(last_hum, 2) + " %";
      // }      
      bot.sendMessage(bot.messages[i].chat_id, message);
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setTimeOut(250); // Таймаут I2C. Без него датчик не всегда успевает инициализироваться.
  
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT);
  pinMode(LED_PIN, OUTPUT); // Временная мера для отладки. В будущем это не нужно.

  Serial.println("Инициализация датчика...");
  if (!htu.begin(&Wire)) {
    Serial.println("Ошибка инициализации HTU21DF!");
  } else {
    Serial.println("HTU21DF инициализирован");
  }

  connectToWiFi();
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