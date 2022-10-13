void SX1276Init()
{
  uint32_t freq, bw;
  uint8_t sf, pwr;
  freq = EEPROM.read(7);
  freq |= freq << 8;
  freq |= EEPROM.read(8);
  freq = freq - 1;
  freq = freq * 100000;
  bw = EEPROM.read(9);
  bw = bw * 1000;
  sf = EEPROM.read(10);
  pwr = EEPROM.read(11);
  pinMode(TXEN, OUTPUT);
  pinMode(RXEN, OUTPUT);
  LoRa.setPins(NSS, RST, DIO0);
  if (LoRa.begin(868E6)) mySerial.println("SX1276...Ok");
  else mySerial.println("SX1276...failed.");
  LoRa.setSignalBandwidth(125E3);
  LoRa.setSpreadingFactor(12);
  LoRa.setCodingRate4(4);
  LoRa.setTxPower(20);
  //txEnable();
  mySerial.print("Lora parameters: FREQ="); mySerial.print(freq/1E6); mySerial.print("MHz; ");
  mySerial.print("BW="); mySerial.print((uint8_t)(bw/1E3)); mySerial.print("kHz; ");
  mySerial.print("SF="); mySerial.print(sf);
  mySerial.print(" PWR="); mySerial.print(pwr); mySerial.println("dBm.");
  mySerial.println("----------------------------------------------------------");
}
//----------------------------------------------------------------------------------
//void SDInit(){
//  mySerial.print("Initializing SD card...");
//  if (!card.init(SPI_HALF_SPEED, SD_CS))
//    mySerial.println("SD initialization failed");
//  else 
//    mySerial.println("SD initialization success");
//  digitalWrite(SD_CS, HIGH);
//}
//----------------------------------------------------------------------------------
void SensInit(){
  myRTC.begin();
  mySerial.println("DS3231...Ok");
  RTC.set(compileTime());
  if (htu.begin()) mySerial.println("HTU21...Ok");
  else mySerial.println("HTU21...failed");
  sensors.begin();
  mySerial.println("DS18B20...Ok");
  if (bmp.begin(0x76)) mySerial.println("BMP280...Ok");
  else mySerial.print("BMP280...failed");
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                    Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                    Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                    Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                    Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
}
//-----------------------------------------------------------------
uint8_t arraySize(uint8_t *myArray) { //подсчет кол-ва ненулевых элементов массива
  uint8_t N = 0;
  for (uint8_t i = 29; i > 0; i--) {
    if (myArray[i] == 0) {
      N++; 
    }
    if (myArray[i] != 0) break;
  }
  N = 40 - N;
  return N;  
}
//----------------------------------------------------------------------------------
void txEnable(){
  digitalWrite(TXEN, HIGH);
  digitalWrite(RXEN, LOW);
}
//----------------------------------------------------------------------------------
void rxEnable(){
  digitalWrite(TXEN, LOW);
  digitalWrite(RXEN, HIGH);
}
//----------------------------------------------------------------------------------
time_t compileTime(){ //функция возвращает время компиляции
    const time_t FUDGE(10);    //fudge factor to allow for upload time, etc. (seconds, YMMV)
    const char *compDate = __DATE__, *compTime = __TIME__, *months = "JanFebMarAprMayJunJulAugSepOctNovDec";
    char compMon[3], *m;

    strncpy(compMon, compDate, 3);
    compMon[3] = '\0';
    m = strstr(months, compMon);

    tmElements_t tm;
    tm.Month = ((m - months) / 3 + 1);
    tm.Day = atoi(compDate + 4);
    tm.Year = atoi(compDate + 7) - 1970;
    tm.Hour = atoi(compTime);
    tm.Minute = atoi(compTime + 3);
    tm.Second = atoi(compTime + 6);

    time_t t = makeTime(tm);
    return t + FUDGE;        //add fudge factor to allow for compile time
}
//----------------------------------------------------------------------------------
void readSensorsData(uint8_t sensNumber) {
  uint16_t value;
  dataOut[0] = 0x7F;
  if (sensNumber == 0) { //DS3231 дата, время
      time_t t = myRTC.get(); //получаем время
      dataOut[ptr] = day(t); ptr++; //5
      dataOut[ptr] = month(t); ptr++; //6
      dataOut[ptr] = year(t)-2000; ptr++; //7
      dataOut[ptr] = hour(t); ptr++; //8
      dataOut[ptr] = minute(t); ptr++; //9
      mySerial.print("Time:");
      printDateTime(t);
      value = (uint16_t)myRTC.temperature() / 4.0 * 100;
      dataOut[ptr] = value >> 8; ptr++; //10
      dataOut[ptr] = value; ptr++; //11
      mySerial.print(";RTC_t:");
      printData(value, 100);  
  }
  if (sensNumber == 1) { //bmp280 температура и атмосферное давление
      value = (uint16_t)bmp.readTemperature() * 100;
      dataOut[ptr] = value >> 8; ptr++;
      dataOut[ptr] = value; ptr++;
      mySerial.print(";bmp_t:");
      printData(value, 100);
      value = (uint16_t)(bmp.readPressure() * 10 * 0.00750062);
      dataOut[ptr] = value >> 8; ptr++;
      dataOut[ptr] = value; ptr++;
      mySerial.print(";bmp_p:");
      printData(value, 10);
  }
  if (sensNumber == 2) { //htu21 только влажность
      value = (uint16_t)htu.readHumidity() * 100;
      dataOut[ptr] = value >> 8; ptr++;
      dataOut[ptr] = value; ptr++;
      mySerial.print(";htu_h:");
      printData(value, 100);
      value = (uint16_t)htu.readTemperature() * 100;
      dataOut[ptr] = value >> 8; ptr++;
      dataOut[ptr] = value; ptr++;
      mySerial.print(";htu_t:");
      printData(value, 100);
  }
  if (sensNumber == 3) { //ds18b20 температура почвы
        sensors.requestTemperatures();
        value = (uint16_t)sensors.getTempCByIndex(0) * 100;
        dataOut[ptr] = value >> 8; ptr++;
        dataOut[ptr] = value; ptr++;
        mySerial.print(";soil_t:");
        printData(value, 100);
  }
  if (sensNumber == 4) { //напряжение на клеммах АКБ и расчет заряда в %
        value = 0;
        for (uint16_t i = 0; i<=1000; i++) value = value + analogRead(A7);
        //value = value / (i+1) * 0.00085217391 * 5.058 * 100; //делим на 1002, умножаем на коэффициенты
        value = 523 * 0.00085217391 * 5.058 * 100; //делим на 1002, умножаем на коэффициенты
        dataOut[ptr] = value >> 8; ptr++;
        dataOut[ptr] = value; ptr++;
        mySerial.print(";vcc:");
        printData(value, 100);
        if (value < 180) value = 0; else value = map(value, 180, 420, 0, 100);
        dataOut[ptr] = value; ptr++;
        mySerial.print(";crg:");
        printData(value, 1);
  }
}
//----------------------------------------------------------------------------------
void printDateTime(time_t tm) {
  mySerial.print(day(tm)); 
  mySerial.print(".");  
  mySerial.print(month(tm)); 
  mySerial.print(".");
  mySerial.print(year(tm)); 
  mySerial.print(" "); 
  mySerial.print(hour(tm)); 
  mySerial.print(":");
  mySerial.print(minute(tm));
}
//----------------------------------------------------------------------------------
void printData(uint16_t val, uint8_t divider) {
  mySerial.print("");
  mySerial.print((float)val / divider);
}
//----------------------------------------------------------------------------------
void perInit(){
  mySerial.begin(9600);
  while (!mySerial) {
  ; // wait for serial port to connect. Needed for native USB port only
  }
  pinMode(2, INPUT_PULLUP); //подтягиваем 2 пин к питанию
  analogReference(INTERNAL); //опорное напряжение для АЦП 1.1В
  pinMode(A7, INPUT);
  RTC.squareWave(SQWAVE_NONE); //конфигурируем пин SQW (DS3231) на прерывания
  RTC.alarmInterrupt(ALARM_1, true); //разрешаем прерывания по 1 будильнику
}
//----------------------------------------------------------------------------------
uint32_t idread(){
  uint32_t id;
  for (uint8_t i = 0; i <= 3; i++) dataOut[i+1] = EEPROM.read(i);
  id = dataOut[3];
  id |= id >> 8;
  id |= dataOut[2];
  id |= id >> 8;
  id |= dataOut[1];
  id |= id >> 8;
  id |= dataOut[0];
  return id;
}
//----------------------------------------------------------------------------------
void set_alarm(uint8_t Hour, uint8_t Min){
  if (dataOut[9] + Min > 59) {
    Min = dataOut[9] + Min - 60;
    Hour++;
  } else Min = dataOut[9] + Min;
  if (dataOut[8] + Hour > 23) Hour = 0;
  else Hour = dataOut[8] + Hour;
  RTC.setAlarm(ALM1_EVERY_SECOND, 0, Min, Hour, 0);
  //RTC.setAlarm(ALM1_MATCH_HOURS, 0, Min, Hour, 0);
  RTC.alarm(ALARM_1); //очищаем флаг будильника ALARM_1
  printAlarmDate();
  sleep.sleepInterrupt(0,FALLING); //отправляем в сон и будим по прерыванию на 2 пине 
}
//----------------------------------------------------------------------------------
void printAlarmDate() {
  uint8_t value[2];
  RTC.readRTC(0x08, value, 2);
  mySerial.print("Next alarm time: ");
  mySerial.print(bcd2dec(value[1]));
  mySerial.print(":");
  mySerial.println(bcd2dec(value[0]));
}
//----------------------------------------------------------------------------------
byte getHash(byte* data, int length) {
  byte hash = 0;
  int i = 0;
  while (length--) {
    hash += *(data + i);
    i++;
  }
  return hash;
}
//----------------------------------------------------------------------------------
void sendLoraPacket()
{
  uint8_t i;
  uint8_t packetSize;
  uint32_t time_out;
  char output[100];
  
  dataOut[ptr] = getHash(dataOut, ptr); ptr++; //считаем контрольную сумму
  //отправляем пакет с климатическими данными
  LoRa.beginPacket(); 
  LoRa.write(dataOut, ptr);
  LoRa.endPacket();

  //выводим в последовательный порт то, что отправили
  mySerial.print("Пакет с климатическими данными ");
  for (i = 0; i <= ptr; i++) mySerial.print(dataOut[i], HEX);
  mySerial.print(" CRC = ");
  mySerial.println(dataOut[ptr - 1], HEX);

  //чистим массив
  for (i = 0; i < sizeof(dataOut); i++) {
    dataOut[i] = 0;
  }
  
  //отправляем байт, подтверждающий готовность к приему обратного сообщения
  uint8_t readyByte = 0x7F;
  mySerial.print("Отправляем readyByte ");
  mySerial.println(readyByte, HEX);
  LoRa.beginPacket(); 
  LoRa.write(readyByte, 1);
  LoRa.endPacket();

  //ждем обратный пакет
  packetSize = LoRa.parsePacket();
  time_out = millis();
  while (!packetSize && !(millis() - time_out > 5000)) packetSize = LoRa.parsePacket();

  //если пакет принят и его размер отличен от 0
  if (packetSize) {
    i = 0;
    while (LoRa.available()) {
      dataOut[i] = LoRa.read();
      i++;
    }
  }

  for (uint8_t i = 0; i < sizeof(dataOut); i++) mySerial.print(dataOut[i], HEX);
  mySerial.println();
  sprintf(output, "ID:%d;%d.%d.%d %d:%d", id(dataOut[1], dataOut[2], dataOut[3], dataOut[4]), dataOut[5], dataOut[6], dataOut[7], dataOut[8], dataOut[9]);
  mySerial.println(output);
  
    
//    uint32_t time_out;
//    uint8_t packetSize = LoRa.parsePacket();
//    time_out = millis();
//    //пока не принята посылка и не прошел таймаут ожидания
//    while (!packetSize && !(millis() - time_out > 5000)) {
//      packetSize = LoRa.parsePacket();
//      uint8_t i = 0;
//      time_out = millis();
//      //пока в буффере приемника есть данные, пишем их в переменную
//      while (LoRa.available()) {
//        dataOut[i] = LoRa.read();
//        i++;
//      }
//    }
//    mySerial.println("Recieved syncpacket: ");
//    for (uint8_t i = 0; i <= sizeof(dataOut); i++) mySerial.print(dataOut[i], HEX);
//    
//    if (!packetSize && n < 4) {
//      sendLoraPacket();
//      n++;
//    }
//    
//    if (packetSize && dataOut[0] == 0x7F && id(dataOut[4], dataOut[3], dataOut[2], dataOut[1]) == idread() && dataOut[10] == crc && dataOut[11] == getHash(dataOut, 11)) {
//      n = 0;
//      tmElements_t tm1;
//      tm1.Month = dataOut[6];
//      tm1.Day = dataOut[5];
//      tm1.Year = dataOut[7];
//      tm1.Hour = dataOut[8];
//      tm1.Minute = dataOut[9];
//      tm1.Second = 0;
//      RTC.set(makeTime(tm1)); 
//      
//      for (uint8_t i = 0; i < 40; i++) {
//        dataOut[i] = 0;
//      }
//      
//      dataOut[0] = 0x7F;
//      idread();
//      dataOut[5] = getHash(dataOut, 5);
//
//      LoRa.beginPacket(); 
//      LoRa.write(dataOut, sizeof(dataOut));
//      LoRa.endPacket();
//      LoRa.end();
//
//      mySerial.print("Confirm dataOut: ");
//      for (uint8_t i = 0; i <= ptr; i++) mySerial.print(dataOut[i], HEX);
//      mySerial.print(" CRC = ");
//      mySerial.println(dataOut[ptr - 1], HEX);
//    }
}
// BCD-to-Decimal conversion
uint8_t bcd2dec(uint8_t n)
{
    return n - 6 * (n >> 4);
}

uint32_t id (uint8_t byte_1, uint8_t byte_2, uint8_t byte_3, uint8_t byte_4){
    uint32_t value = byte_4;
    value |= value >> 8;
    value |= byte_3;
    value |= value >> 8;
    value |= byte_2;
    value |= value >> 8;
    value |= byte_1;
    return value;
}

bool handshake() {
  uint8_t i;
  uint8_t packetSize;
  uint32_t time_out;
  
  dataOut[0] = 0x8F;
  idread();
  crc = getHash(dataOut, arraySize(dataOut));
  tmElements_t tm;
  dataOut[5] = crc;
  LoRa.beginPacket();
  LoRa.write(dataOut, arraySize(dataOut));
  LoRa.endPacket();
  
  mySerial.print("Handshake packet is sent = ");
  for (i = 0; i < arraySize(dataOut); i++) mySerial.print(dataOut[i], HEX);
  mySerial.print(", CRC = ");
  mySerial.println(dataOut[5], HEX);
  
  packetSize = LoRa.parsePacket();
  time_out = millis();
  // пока размер принятого пакета 0 (т.е. ничего не принято) и не вышел таймаут, висим на приеме
  while (!packetSize && !(millis() - time_out > 3000)) packetSize = LoRa.parsePacket();
  
  if (packetSize) {
    i = 0;
    while (LoRa.available()) {
      dataOut[i] = LoRa.read();
      i++;
    }
    mySerial.print("Configuration is recieved = ");
    for (i = 0; i <= arraySize(dataOut); i++) mySerial.print(dataOut[i], HEX);
    crc = getHash(dataOut, arraySize(dataOut) - 1);
    mySerial.print(" CRC = ");
    mySerial.println(crc, HEX);
    
    if (crc == dataOut[18] && dataOut[0] == 0x8F) {
      EEPROM.update(5, dataOut[5]); //период передачи данных час
      EEPROM.update(6, dataOut[6]); //период передачи данных минута
      EEPROM.update(7, dataOut[7]); //частота 1 байт
      EEPROM.update(8, dataOut[8]); //частота 2 байт
      EEPROM.update(9, dataOut[9]); //bw
      EEPROM.update(10, dataOut[10]); //sf
      EEPROM.update(12, dataOut[11]); //период считывания данных час
      EEPROM.update(13, dataOut[12]); //период считывания данных минута
      mySerial.println("Configuration is wrote to EEPROM");
      tm.Day = dataOut[13]; //текущий день
      tm.Month = dataOut[14]; //текущий месяц
      tm.Year = dataOut[15]; //текущий год
      tm.Hour = dataOut[16]; //период время час
      tm.Minute = dataOut[17]; //текущий время минуты
      tm.Second = 0;
      RTC.set(makeTime(tm));
    }

    mySerial.println("Handshake is success");
    SX1276Init(); //инициализация SX1276
    for (i = 0; i < sizeof(dataOut); i++) dataOut[i] = 0;
    return true;
  } else {
      mySerial.println("Handshake is failed");
      for (i = 0; i < sizeof(dataOut); i++) dataOut[i] = 0;
      handshake();
  }
}

void reset_conf() {
  
}
