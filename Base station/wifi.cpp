//----------------------------------------------------------------------------------------
void WIFIinit() {
  WiFi.mode(WIFI_AP_STA); //переводим модуль в режим станции
  byte tries = 11; //количество попыток подключения
  //парсим файл настроек и вытаскиваем оттуда логин и пароль от Wi-Fi 
  DynamicJsonDocument  jsonBuffer(1024);
  DeserializationError error = deserializeJson(jsonBuffer, conf);
  String ssid = jsonBuffer["ssid"].as<String>();
  String pass = jsonBuffer["pass"].as<String>();
  WiFi.begin(ssid, pass); //попытки подключиться к сети Wi-Fi 
  while (--tries && WiFi.status() != WL_CONNECTED) //пытаемся подключиться к Wi-Fi
  {
    Serial.print(".");
    delay(1000);
  }
  if (WiFi.status() != WL_CONNECTED) //если не подключились, то запускам точку доступа
  {
    Serial.println("");
    Serial.println("WiFi up AP");
    StartAPMode(); //функция для запуска точки доступа
  }
  else { //если подключились, то выводим IP на экран
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP()); //выводим IP в последовательный порт
    myOLED.drawImage(Wi_Fi, 98, 10); //выводим иконку Wi-Fi
    myOLED.print(IpAddress2String(WiFi.localIP()), 0, 25); //IP на дисплей
  }
}
//----------------------------------------------------------------------------------------
bool StartAPMode() {
  //парсим конфиг файл и выделаем имя и пароль точки доступа
  DynamicJsonDocument  jsonBuffer(1024);
  DeserializationError error = deserializeJson(jsonBuffer, conf);
  String apssid = jsonBuffer["apssid"].as<String>();
  String appass = jsonBuffer["appass"].as<String>();
  WiFi.disconnect(); // Отключаем WIFI
  WiFi.mode(WIFI_AP_STA); // Меняем режим на режим точки доступа
  IPAddress apIP(192, 168, 4, 1); //задаем IP точки доступа
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0)); // Задаем настройки сети
  WiFi.softAP(apssid, appass); //запускаем точку доступа
  myOLED.print("AP", 96, 10); //рисуем соответвующие символы на дисплее
  myOLED.print("IP  192.168.4.1", 0, 25);
  return true;
}
//----------------------------------------------------------------------
void wifi_status(){ //функция проверки подключения к сети Wi-Fi
  if (WiFi.getMode() == 1 && WiFi.status() == WL_CONNECTED) {
    myOLED.drawImage(Wi_Fi, 98, 10); 
  } else if (WiFi.getMode() == 1 && WiFi.status() != WL_CONNECTED) {
      myOLED.drawImage(black, 98, 10);
      WiFi.begin("Room 52", "10031998");
  }
}
