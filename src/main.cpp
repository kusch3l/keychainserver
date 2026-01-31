#include <Arduino.h>

#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include <ESPmDNS.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

AsyncWebServer server(80);

void configureRoutes() {

  server.serveStatic("/", SD, "/www")
  .setDefaultFile("index.html");

  server.serveStatic("/game/", SD, "/game/")
        .setCacheControl("max-age=3600");

  server.onNotFound([](AsyncWebServerRequest *request){
    request->send(404, "text/plain", "File not found");
  });
}

void setup(){
  Serial.begin(115200);

  if(!SD.begin(5, SPI, 40000000, "/sd", 10)){
    Serial.println("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD.cardType();

  if(cardType == CARD_NONE){
    Serial.println("No SD card attached");
    return;
  }

  String ssid = "ESP32 AP Mode"; 
  String password = "123456789";

  File configFile = SD.open("/wifi.txt");
  if (configFile) {
    while (configFile.available()) {
      String line = configFile.readStringUntil('\n');
      Serial.println("ssid in while: " + ssid);
      if (line.startsWith("ssid=")){
        ssid = line.substring(5);
        Serial.println("ssid updated: " + ssid);
      }
      if (line.startsWith("password=")) {
        password = line.substring(9); //needs to be minimum 8 chars
        Serial.println("pw updated: " + password);
      }
      Serial.println("line: " + line);
    }
    configFile.close();
    Serial.println("ssid after while: " + ssid);
    Serial.println("WiFi config loaded from SD: " + ssid);
  } else {
    Serial.println("No /wifi.txt - using hardcoded defaults!");
  }
  
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  delay(5000);
  Serial.println("WiFi: " + WiFi.softAPSSID()); 
  Serial.println("IP-Address: " + WiFi.localIP().toString());

  if (MDNS.begin("keychainserver")) {
    Serial.println("mDNS: http://keychainserver.local");
  }

  configureRoutes();

  server.begin();
  MDNS.addService("http", "tcp", 80);
  Serial.println("Async Webserver läuft!");
}

void loop(){

}
