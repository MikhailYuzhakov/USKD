void HTTPInit() {  
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.htm", String());
  });

  server.on("/css/main.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/css/main.css", "text/css");
  });

  server.on("/css/index.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/css/index.css", "text/css");
  });

  server.on("/css/loader.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/css/loader.css", "text/css");
  });

  server.on("/css/config.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/css/config.css", "text/css");
  });

  server.on("/img/logo.png", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/img/logo.png");
  });

  server.on("/js/main.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/js/main.js", "text/js");
  });

  server.on("/js/config.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/js/config.js", "text/js");
  });

  server.on("/config.live.json", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/config.live.json", "application/json");
  });

  server.on("/config.html", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/config.html", String());
  });

  server.on("/probes-configs", HTTP_GET, [](AsyncWebServerRequest *request){ 
    String conf = "[";
    Dir dir = SPIFFS.openDir("/");
    while (dir.next()) {
      if (dir.fileName().substring(1, 3) == "id") {
        File confprobes = SPIFFS.open(dir.fileName(), "r");
        conf += confprobes.readString() + ", ";
        confprobes.close();
        Serial.println(dir.fileName());
      }
    }
    conf.remove(conf.length() - 2, 3);
    conf += "]";
    Serial.println(conf);
    request->send(200, "text/plain", conf);
  });

  server.on("/hotspot-config", HTTP_GET, [](AsyncWebServerRequest *request){ 
    File confhotspot = SPIFFS.open("/hotspot-config.json", "r"); 
    String conf = confhotspot.readString(); 
    confhotspot.close();
    request->send(200, "text/plain", conf);
  });

  server.on("/hotspot", HTTP_GET, [](AsyncWebServerRequest *request){ 
    configRead(request);
    request->send(200, "text/plain", "Ok");
  });

  server.on("/probe", HTTP_GET, [](AsyncWebServerRequest *request){ 
    configProbeRead(request);
    request->send(200, "text/plain", "Ok");
  });
  
  server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/favicon.ico");
  });

  // Handle Web Server Events
  events.onConnect([](AsyncEventSourceClient *client){
    if(client->lastId()){
      Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
    } else Serial.printf("Clietn sse!");
  });
  server.addHandler(&events);

  server.begin();
}

void mDNSInit(){
  if (!MDNS.begin("USKD")) {
  Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");  
}

void configRead(AsyncWebServerRequest *request) {
  int args = request->args();
  uint8_t i = 0;
  File confhotspot = SPIFFS.open("/hotspot-config.json", "w"); 
  StaticJsonDocument<200> configHotspot;
  configHotspot["id"] = "1";
  configHotspot["ssid"] = request->arg(i).c_str(); i++;
  configHotspot["pass"] = request->arg(i).c_str(); i++;
  configHotspot["apssid"] = request->arg(i).c_str(); i++;
  configHotspot["appass"] = request->arg(i).c_str(); i++;
  configHotspot["bw"] = request->arg(i).c_str(); i++;
  configHotspot["sf"] = request->arg(i).c_str(); i++;
  configHotspot["freq"] = request->arg(i).c_str(); i++;
  configHotspot["pwr"] = request->arg(i).c_str(); i++;
  configHotspot["timezone"] = "3";
  serializeJson(configHotspot, confhotspot);
  loadConfig();
  confhotspot.close();
}

void configProbeRead(AsyncWebServerRequest *request) {
  int args = request->args();
  uint8_t i = 0;
  String id = request->arg(i).c_str();
  String filename = "/id" + id + ".json";
  File confprobe = SPIFFS.open(filename, "w"); 
  StaticJsonDocument<200> configProbe;
  configProbe["id"] = request->arg(i).c_str(); i++;
  configProbe["time_read"] = request->arg(i).c_str(); i++;
  configProbe["bw"] = request->arg(i).c_str(); i++;
  configProbe["sf"] = request->arg(i).c_str(); i++;
  configProbe["freq"] = request->arg(i).c_str(); i++;
  configProbe["time_trans"] = request->arg(i).c_str(); i++;
  serializeJson(configProbe, confprobe);
  serializeJson(configProbe, Serial);
  confprobe.close();
}
