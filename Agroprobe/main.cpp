#include <SoftwareSerial.h>
#include <DS3232RTC.h>
#include <SPI.h>
//#include <SD.h>
#include <LoRa.h>
#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "Adafruit_HTU21DF.h"
#include <Adafruit_BMP280.h>
#include <Sleep_n0m1.h> //библиотека для сна
#include <EEPROM.h> //библиотека для работы с энергонезависимой памятью

#define SD_CS 9 //Chip select для карты памяти
#define NSS 10 //пин Chip select (slave select) lora
#define RST 3 //пин сброса lora
#define DIO0 4 // цифровой пин для lora
#define TXEN 8 //режим на передачу
#define RXEN 7 //режим на прием
#define DS18B20_PIN A0 //пин data датчика ds18b20
#define INT0_PIN 2 //пин для пробуждения

SoftwareSerial mySerial(5, 6); // RX, TX
OneWire oneWire(DS18B20_PIN);
Sleep sleep; //объявляем класс для сна
DS3232RTC myRTC; //объявляем класс для часов
DallasTemperature sensors(&oneWire);
Adafruit_HTU21DF htu = Adafruit_HTU21DF();
Adafruit_BMP280 bmp; // I2C
//Sd2Card card;

uint8_t dataOut[40];
uint8_t ptr = 0;
uint8_t n = 0;
uint8_t crc;

void setup() {
  perInit();
  mySerial.print("Agroprobe ID ");
  mySerial.println(idread());
  mySerial.print("Alarm period: ");
  mySerial.print(EEPROM.read(5));
  mySerial.print(":");
  mySerial.println(EEPROM.read(6));
  //SDInit(); //инициализация SD карты памяти
  SensInit(); //инициализация датчиков
  SX1276Init(); //инициализация SX1276
  handshake();
}

void loop() {
  idread();
  ptr = 5;
  mySerial.print("Данные с зонда: ");
  for (uint8_t sensNumb = 0; sensNumb <= 4; sensNumb++) readSensorsData(sensNumb);
  mySerial.println();
  sendLoraPacket();
  delay(60000);
//  set_alarm(EEPROM.read(5), EEPROM.read(6)); //период пробуждения 0 часов 1 минута
}
