void timeSynch() {
  DynamicJsonDocument  jsonBuffer(1024);
  DeserializationError error = deserializeJson(jsonBuffer, conf);
  uint8_t zone = jsonBuffer["timezone"].as<unsigned int>();
  if (WiFi.status() == WL_CONNECTED) {
    configTime(zone * 3600, 0, "pool.ntp.org", "ru.pool.ntp.org"); // Настройка соединения с NTP сервером
    int i = 0;
    Serial.println("Waiting for time");
    while (!time(nullptr) && i < 10) {
      Serial.print(".");
      i++;
      delay(1000);
    }
    Serial.println(GetDate());
    Serial.println(GetTime());
  }
}
//----------------------------------------------------------------------------------------
String GetTime() { // Получение текущего времени
 time_t now = time(nullptr); // получаем время с помощью библиотеки time.h
 String Time = ""; // Строка для результатов времени
 Time += ctime(&now); // Преобразуем время в строку формата Thu Jan 19 00:55:35 2017
 int i = Time.indexOf(":"); //Ишем позицию первого символа :
 Time = Time.substring(i - 2, i + 6); // Выделяем из строки 2 символа перед символом : и 6 символов после
 return Time; // Возврашаем полученное время
}
//----------------------------------------------------------------------------------------
String GetDate() { // Получение даты
 time_t now = time(nullptr); // получаем время с помощью библиотеки time.h
 String Data = ""; // Строка для результатов времени
 Data += ctime(&now); // Преобразуем время в строку формата Thu Jan 19 00:55:35 2017
 int i = Data.lastIndexOf(" "); //Ишем позицию последнего символа пробел
 String Time = Data.substring(i - 8, i+1); // Выделяем время и пробел
 Data.replace(Time, ""); // Удаляем из строки 8 символов времени и пробел
 Data.replace("\n", ""); // Удаляем символ переноса строки
 return Data; // Возврашаем полученную дату
}
