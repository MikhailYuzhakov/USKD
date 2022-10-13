#include <ESP8266WiFi.h> //библиотека для работы все функций Wi-Fi
#include <ESPAsyncTCP.h> //асинхронный tcp сервер для sse
#include <ESPAsyncWebServer.h> //асинхронный http сервер для sse
#include <FS.h> //для работы с файловой системой внутри ESP8266
#include <ArduinoJson.h> //библиотека для работы с json
#include <DS3232RTC.h> //для работы с часами реального времени DS3231
#include <ESP8266mDNS.h>
#include <Wire.h> //для работы с шиной I2C (TWI)
#include <time.h> //для работы с временем
#include <iarduino_OLED.h> //для работы с дисплеем
#include <SPI.h> // для работы с шиной SPI (SX1276)
#include <LoRa.h> //для работы с модулем SX1276

DS3232RTC RTC; //объявляем класс для часов
File fsUploadFile; // Для файловой системы
iarduino_OLED myOLED(0x3C); //адрес дисплея
WiFiClient client; //класс для работы с TCP сервером
AsyncEventSource events("/probes"); //создаем объект для sse
AsyncWebServer server(80); //создаем объект http для сервера
WiFiEventHandler stationConnectedHandler;

#define NSS 15 //пин Chip select (slave select) lora
#define RST 2 //пин сброса lora
#define DIO0 16 // цифровой пин для lora

ADC_MODE(ADC_VCC); //инициализируем АЦП ESP для измерения напряжения питания

String jsonConfig = "{}";
String probes_data;
String outPrint;

uint32_t myTimer1, myTimer2; //для разделения задач
int rssi; //уровень сигнала
char buffer1[200]; //буффер для данных
int packetSize; //возвращает размер принятого пакета LoRa
uint8_t recvData[50]; //для храннения принятого пакела LoRa
uint8_t sendData[50]; //для хранения пакета для отправки
uint8_t checkBuffer[20];
const char* host = "xn--d1aluh.xn--p1ai"; //host TCP сервера
const uint16_t port_tcp = 7070; //порт TCP сервера
struct tm * timeinfo; //структура для хранения времени и даты
String conf;

//иконки и шрифты для дисплея
extern uint8_t Wi_Fi[], update_ota[], black[], point[], point_2[], grad[], 
               Img_Battery_0[], Img_Battery_1[], Img_Battery_2[], Img_Battery_3[],
               Img_Battery_charging[], black_50[], light[], clock1[], SmallFontRus[], 
               MediumFontRus[], MediumNumbers[], BigNumbers[];

void setup() {
  Serial.begin(115200);
  FileSystemInit(); //запускаем файловую систему
  loadConfig();
  per_begin(); //запускам переферийные модули
  charge(); //рисуем индикатор уровня заряда АКБ
  WIFIinit(); //инициализируем Wi-Fi
  SyncTime();//синхронизауия времени с серверами
  mDNSInit(); //инициализация mDNS
  HTTPInit(); //запуск асинхронного сервера
  MDNS.addService("http", "tcp", 80);
  TCPInit(); //Подключаемся к TCP серверу
}

void loop() {
  MDNS.update();
  LoRaListening();
  if (millis() - myTimer1 >= 5000){
    myTimer1 = millis();
    SyncTime();
    wifi_status(); 
  }
  
  if (millis() - myTimer2 >= 60000){
    charge();
    myTimer2 = millis();
  }
}
