void FileSystemInit(){
  if(!SPIFFS.begin()){ // Initialize SPIFFS
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
}

void loadConfig() {
  File confighotspot = SPIFFS.open("/hotspot-config.json", "r");
  conf = confighotspot.readString();
  Serial.println(conf);
}
