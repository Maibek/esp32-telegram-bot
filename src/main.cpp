#include <Wire.h>
#include <Adafruit_BME280.h>

Adafruit_BME280 bme;
const int SDA_PIN = 18;
const int SCL_PIN = 19;

void setup() {
  Serial.begin(115200);
  delay(2000); // Даем время для инициализации
  
  // Инициализация I2C с указанием пинов
  Wire.begin(SDA_PIN, SCL_PIN);
  
  Serial.println("BME280 Test");
  
  // Попробуем оба возможных адреса
  if (!bme.begin(0x76, &Wire)) { // Первый вариант адреса
    if (!bme.begin(0x77, &Wire)) { // Второй вариант адреса
      Serial.println("Could not find BME280 sensor!");
      while (1); // Бесконечный цикл при ошибке
    } else {
      Serial.println("BME280 found at 0x77");
    }
  } else {
    Serial.println("BME280 found at 0x76");
  }
  
  // Настройка параметров измерения
  bme.setSampling(Adafruit_BME280::MODE_NORMAL,
                  Adafruit_BME280::SAMPLING_X2,  // Температура
                  Adafruit_BME280::SAMPLING_X16, // Давление
                  Adafruit_BME280::SAMPLING_X1,  // Влажность
                  Adafruit_BME280::FILTER_OFF,
                  Adafruit_BME280::STANDBY_MS_0_5);
}

void loop() {
  Serial.print("Temperature = ");
  Serial.print(bme.readTemperature());
  Serial.println(" °C");

  Serial.print("Humidity = ");
  Serial.print(bme.readHumidity());
  Serial.println(" %");

  Serial.print("Pressure = ");
  Serial.print(bme.readPressure() / 100.0F); // Переводим в гПа
  Serial.println(" hPa");

  Serial.println("------------------");
  delay(3000);
}