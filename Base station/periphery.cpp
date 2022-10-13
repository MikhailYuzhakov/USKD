//-----------------------------------------------------------------
void per_begin(){ //запуск всех переферийных интерфейсов и устройств
  DynamicJsonDocument  jsonBuffer(1024);
  DeserializationError error = deserializeJson(jsonBuffer, conf);
  unsigned long freq = jsonBuffer["freq"].as<unsigned long>();
  unsigned long bw = jsonBuffer["bw"].as<unsigned int>() * 1000;
  unsigned long sf = jsonBuffer["sf"].as<unsigned int>();
  unsigned long pwr = jsonBuffer["pwr"].as<unsigned int>();
  Wire.begin(5, 4); //запускаем twi SCL -> GPIO5, SDA -> GPIO4
  RTC.begin();
  SX1276Init(freq, bw, sf, 4, pwr);
  myOLED.begin(); // Инициируем работу с дисплеем.
  myOLED.setFont(SmallFontRus);
  myOLED.setCoding(TXT_UTF8);
  myOLED.drawLine(0, 14, 127, 14);
}
//-----------------------------------------------------------------
uint8_t arraySize(uint8_t *myArray) { //подсчет кол-ва ненулевых элементов массива
  uint8_t N = 0;
  for (uint8_t i = 49; i > 0; i--) {
    if (myArray[i] == 0) {
      N++; 
    }
    if (myArray[i] != 0) break;
  }
  N = 50 - N;
  return N;  
}
//-----------------------------------------------------------------
uint8_t flushArray(uint8_t *myArray) { //обнуление всех элементов массива
  uint8_t N = 0;
  for (uint8_t i = 0; i < 49; i++) {
    myArray[i] = 0;
  }
  return N;  
}
//-----------------------------------------------------------------
void charge(){ //функция проверки уровня заряда и отрисовки на экране
  uint16_t vcc_mv = 0;
  uint8_t bat_status = 0;
  vcc_mv = ESP.getVcc();
  //if (vcc_mv > 4200) myOLED.drawImage(Img_Battery_charging, 112, 10);
  if (vcc_mv > 3700) myOLED.drawImage(Img_Battery_3, 112, 10);
  if (vcc_mv < 3700 && vcc_mv >= 3200) myOLED.drawImage(Img_Battery_2, 112, 10);
  if (vcc_mv < 3200 && vcc_mv >= 2700) myOLED.drawImage(Img_Battery_1, 112, 10);
  if (vcc_mv < 2700) myOLED.drawImage(Img_Battery_0, 112, 10);
}
//-----------------------------------------------------------------
String IpAddress2String(const IPAddress& ipAddress)
{
    return "IP " + String(ipAddress[0]) + String(".") +
           String(ipAddress[1]) + String(".") +
           String(ipAddress[2]) + String(".") +
           String(ipAddress[3]);
}
//-----------------------------------------------------------------
void TCPInit(){
  if (!client.connect(host, port_tcp)) //подключение к серверу по TCP
  {
    Serial.println("TCP server connection failed");
    myOLED.print("   ", 76, 10);
  } else
  {
    Serial.println("TCP server connect");
    myOLED.print("TCP", 76, 10);
  }
}
//-----------------------------------------------------------------
void SyncTime(){ //вывод данных с микросхемы часов
    DynamicJsonDocument  jsonBuffer(1024);
    DeserializationError error = deserializeJson(jsonBuffer, conf);
    uint8_t zone = jsonBuffer["timezone"].as<unsigned int>();
    if (WiFi.status() == WL_CONNECTED) {
      configTime(zone * 3600, 0, "ru.pool.ntp.org", "pool.ntp.org"); // Настройка соединения с NTP сервером
      int i = 0;
      while (!time(nullptr) && i < 10) {
        Serial.print(".");
        i++;
        delay(1000);
      }
    }
    char buf[40];
    time_t now = time(nullptr); // получаем время с помощью библиотеки time.h
    timeinfo = localtime(&now);
    tmElements_t tm1;
    tm1.Month = timeinfo->tm_mon + 1;
    tm1.Day = timeinfo->tm_mday;
    tm1.Year = timeinfo->tm_year - 70;
    tm1.Hour = timeinfo->tm_hour;
    tm1.Minute = timeinfo->tm_min;
    tm1.Second = timeinfo->tm_sec;
    RTC.set(makeTime(tm1));
    
    time_t t = RTC.get();
    sprintf(buf, "%.2d:%.2d", hour(t), minute(t));
    myOLED.print(buf, 0, 10);
    sprintf(buf, "SyncTime: %d.%d.%d %d:%d:%d\n", day(t), month(t), year(t), hour(t), minute(t), second(t));
    Serial.print(buf);
}
//-----------------------------------------------------------------
void SX1276Init(uint32_t freq, uint32_t bw, uint8_t sf, uint8_t cr, uint8_t pwr)
{
  LoRa.setPins(NSS, RST, DIO0);
  if (LoRa.begin(freq)) Serial.println("SX1276 initialization success");
  else Serial.println("SX1276 initialization failed.");
  LoRa.setSignalBandwidth(bw);
  LoRa.setSpreadingFactor(sf);
  LoRa.setTxPower(pwr);
  Serial.println("LoRa parameters: FREQ=" + String(freq) + "Hz, BW=" + String(bw) +
                  "Hz, SF=" + String(sf) + ", PWR=" + String(pwr) + "dBm");
}
//-------------------------------------------------------------------
byte getHash(byte* data, int length) { //расчет и вывод контрольной суммы
  byte hash = 0;
  int i = 0;
  while (length--) {
    hash += *(data + i);
    i++;
  }
  return hash;
}
//-------------------------------------------------------------------
void LoRaListening() { //обработка данных от зондов
  packetSize = LoRa.parsePacket();  
  if (packetSize) {
    flushArray(recvData);
    flushArray(sendData);
    Serial.print("Recieving packet = ");
    uint8_t i = 0;
    probes_data = "";
    while (LoRa.available()) {
      recvData[i] = LoRa.read();
      Serial.print(recvData[i], HEX);
      i++;
    }
    uint8_t crc = getHash(recvData, arraySize(recvData) - 1);
    Serial.print(" CRC = ");
    Serial.println(crc, HEX);  //печать контрольной суммы, рассчитанной на БС
    Serial.print("recvData[0] = ");
    Serial.print(recvData[0], HEX); //печать кода протокола (0х7F или 0x8F)
    Serial.print(" recvData[last] = ");
    Serial.println(recvData[arraySize(recvData) - 1], HEX); //печать контрольной суммы, рассчитанной на зонде

    //1) Если БС приняла от зонда запрос на конфигурацию
    if (recvData[0] == 0x8F && recvData[arraySize(recvData) - 1] == crc)
      recvConfigPacket();
      
    //2) Если БС приняла от зонда климатические данные
    if (recvData[0] == 0x7F && recvData[arraySize(recvData) - 1] == crc)
      recvDataPacket();
 }
}
//-------------------------------------------------------------------
void recvConfigPacket() {
  uint32_t ID = id(recvData[4], recvData[3], recvData[2], recvData[1]);
  String filename = "/id" + String(ID) + ".json";
  Serial.println(filename); //печаьаем имя созданного файла для зонда
  File confprobe = SPIFFS.open(filename, "w"); //создаем файл с конфигурацией
  StaticJsonDocument<200> configProbe;
  //записываем в конфигурационный файл все настройки (по умолчанию)
  configProbe["id"] = String(ID);
  configProbe["time_trans"] = "0:1";
  configProbe["freq"] = "868000000";
  configProbe["bw"] = "125";
  configProbe["sf"] = "12";
  configProbe["time_read"] = "0:1";
  serializeJson(configProbe, confprobe); //преобразуем в json формат
  confprobe.close();
  
  Serial.println("Get request for configuration");
  File confprobes = SPIFFS.open(filename, "r"); //открываем конфигурационный файл
  String configProbes = confprobes.readString(); //считываем в строку настройки
  Serial.println("Configuration: " + configProbes); //выводим конфигурацию зонда в монитор порта
  DynamicJsonDocument  jsonBuffer(1024);
  DeserializationError error = deserializeJson(jsonBuffer, configProbes); //передеаем все настройки в объект json
  time_t now = time(nullptr); // получаем время с помощью библиотеки time.h
  timeinfo = localtime(&now);
  tmElements_t tm;
  //заполняем конфигурационный массив для зонда
  sendData[0] = 0x8F; 
  sendData[1] = recvData[1]; //ID 1 байт
  sendData[2] = recvData[2]; //ID 2 байт
  sendData[3] = recvData[3]; //ID 3 байт
  sendData[4] = recvData[4]; //ID 4 байт
  //вытягиваем из объекта json настройки и переписываем в переменные
  sendData[5] = jsonBuffer["time_trans"].as<String>().substring(0).toInt(); //час
  sendData[6] = jsonBuffer["time_trans"].as<String>().substring(2).toInt(); //минута
  uint32_t value = (float)jsonBuffer["freq"].as<String>().toInt() / 100000; //частота
  sendData[7] = value >> 8; //частоту передаем двумя байтами (1 байт)
  sendData[8] = value; //2 байт
  sendData[9] = jsonBuffer["bw"].as<String>().toInt(); //bandwidth
  sendData[10] = jsonBuffer["sf"].as<String>().toInt(); //spreading factor
  sendData[11] = jsonBuffer["time_read"].as<String>().substring(0).toInt(); //час
  sendData[12] = jsonBuffer["time_read"].as<String>().substring(2).toInt(); //минута
  //пишем текущее время
  sendData[13] = timeinfo->tm_mday;
  sendData[14] = timeinfo->tm_mon + 1;
  sendData[15] = timeinfo->tm_year - 70;
  sendData[16] = timeinfo->tm_hour;
  sendData[17] = timeinfo->tm_min;
  //считаем контрольную сумму и записываем в последний байт
  sendData[18] = getHash(sendData, arraySize(sendData));
  //выводим в монитор порта конфигурационный пакет и контрольную сумму отдельно
  Serial.print("Configuration packet = ");
  for (uint8_t i = 0; i <= 18; i++) Serial.print(sendData[i], HEX);
  Serial.print(" CRC = ");
  Serial.println(sendData[18], HEX);
  //отправляем конфигурационный пакет
  LoRa.beginPacket();
  LoRa.write(sendData, arraySize(sendData));
  LoRa.endPacket();
}
//-------------------------------------------------------------------
void recvDataPacket() {
      uint8_t readyByte[10];
      Serial.println("Зонд передал климатические данные. Обработка...");
      uint32_t ID = id(recvData[4], recvData[3], recvData[2], recvData[1]);
      uint8_t Day = recvData[5];
      uint8_t Month = recvData[6];
      uint8_t Year = recvData[7];
      uint8_t Hour = recvData[8];
      uint8_t Min = recvData[9];
      float temp_rtc = convert(recvData[10], recvData[11], 100);
      float temp_bmp = convert(recvData[12], recvData[13], 100);
      float press_bmp = convert(recvData[14], recvData[15], 10);
      float hum_htu = convert(recvData[16], recvData[17], 100);
      float temp_htu = convert(recvData[18], recvData[19], 100);
      float temp_ds18b20 = convert(recvData[20], recvData[21], 100);
      float vacc = convert(recvData[22], recvData[23], 100);
      uint8_t chrg = recvData[24];
      sprintf(buffer1, "ID:%d;Time:%d.%d.%d %d:%d;RTC_t:%.2f;bmp_t:%.2f;bmp_p:%.2f;hum_h:%.2f;htu_t:%.2f;soil_t:%.2f;vacc:%.2f;chrg:%d\n",
             ID, Day, Month, Year, Hour, Min, temp_rtc, temp_bmp, press_bmp, hum_htu, temp_htu, temp_ds18b20, vacc, chrg);
      Serial.print(buffer1);
      
      time_t now = time(nullptr); // получаем время с помощью библиотеки time.h
      timeinfo = localtime(&now);
      tmElements_t tm;
      sendData[0] = 0x7F;
      sendData[1] = recvData[1];
      sendData[2] = recvData[2];
      sendData[3] = recvData[3];  
      sendData[4] = recvData[4];
      sendData[5] = timeinfo->tm_mday;
      sendData[6] = timeinfo->tm_mon + 1;
      sendData[7] = timeinfo->tm_year - 70;
      sendData[8] = timeinfo->tm_hour;
      sendData[9] = timeinfo->tm_min;

      //ждем байта о готовности к приему обратного пакета
      uint32_t time_out = millis();
      packetSize = LoRa.parsePacket();
      while (!packetSize && !(millis() - time_out > 10000)) packetSize = LoRa.parsePacket();

      uint8_t i = 0;
      if (packetSize) {
        while (LoRa.available()) {
          readyByte[i] = LoRa.read();
          i++;
        }
       
      Serial.print("packetSize = ");
      Serial.println(packetSize);
      Serial.print("Bytes number of last packet = ");
      Serial.println(LoRa.available());
       
      }
      Serial.print("readyByte = ");
      for (i = 0; i <= 9; i++) {
        Serial.print(" 0x");
        Serial.print(readyByte[i], HEX); 
      }
      Serial.println();
      
      //выводим в последовательный порт то, что собираемся отправить
      Serial.print("Обратный пакет = ");
      for (uint8_t i = 0; i <= arraySize(sendData); i++) Serial.print(sendData[i], HEX);
      Serial.println(arraySize(sendData));
      
      //отправляем обратный пакет с ID и текущем временем  
      if (readyByte[9] == 0x7F) {
        LoRa.beginPacket();
        LoRa.write(sendData, arraySize(sendData));
        LoRa.endPacket(); 
      }
//  
//      uint32_t time_out;
//      uint8_t packetSize = LoRa.parsePacket();
//      time_out = millis();
//      while (!packetSize && !(millis() - time_out > 5000)) {
//        packetSize = LoRa.parsePacket();
//        uint8_t i = 0;
//        
//        while (LoRa.available()) {
//          checkBuffer[i] = LoRa.read();
//          i++;
//        }
//      }
//      
//       if (checkBuffer[0] == 0x7F && checkBuffer[1] == recvData[1] && checkBuffer[2] == recvData[2] && checkBuffer[3] == recvData[3] && checkBuffer[4] == recvData[4] && checkBuffer[5] == sendData[11]) {
//        DynamicJsonDocument  jsonBuffer(1024);
//        sprintf(buffer1, "20%d-%d-%d %d:%d:%d", Year, Month, Day, Hour, Min, 0);
//        jsonBuffer["id"] = ID;
//        jsonBuffer["datetime"] = buffer1;
//        sprintf(buffer1, "%.2f", temp_bmp);
//        jsonBuffer["atemp"] = buffer1;
//        sprintf(buffer1, "%.2f", temp_rtc);
//        jsonBuffer["temp_rtc"] = buffer1;
//        sprintf(buffer1, "%.1f", press_bmp);
//        jsonBuffer["press_bmp"] = buffer1;
//        sprintf(buffer1, "%.2f", temp_htu);
//        jsonBuffer["temp_htu"] = buffer1;
//        sprintf(buffer1, "%.2f", hum_htu);
//        jsonBuffer["hum_htu"] = buffer1;
//        sprintf(buffer1, "%.2f", temp_ds18b20);
//        jsonBuffer["stemp"] = buffer1;
//        jsonBuffer["vacc"] = vacc;
//        jsonBuffer["charge"] = crg;
//        jsonBuffer["rssi"] = LoRa.packetRssi();
//        serializeJson(jsonBuffer, probes_data);
//        Serial.println(probes_data);
//        events.send(probes_data.c_str(), NULL, millis());
//        if (client.connected()) {
//          client.write(buffer1, strlen(buffer1));
//        } else {
//          //инчае писать в файл
//        }
//      }  
}
//-------------------------------------------------------------------
float convert(uint8_t byte_1, uint8_t byte_2, uint8_t divider){
    uint16_t value = byte_1;
    value |= value << 8;
    value |= byte_2;
    return (float)value / divider;
}
//-------------------------------------------------------------------
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
