#include <Arduino.h>


#include "FS.h"
#include "SD.h"
#include "SPI.h"

#include <ESPmDNS.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#define SPI_CS 2

AsyncWebServer server(80);

void configureWebsite() {

  server.serveStatic("/", SD, "/www")
        .setDefaultFile("index.html");

  server.onNotFound([](AsyncWebServerRequest *request){
    request->send(404, "text/plain", "File not found");
  });

}

void configureGame() {

  server.serveStatic("/game/", SD, "/game")
        .setCacheControl("max-age=3600");
  
  server.on("/sethighscore", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("n")) {
      if (request->hasParam("s")){
        //in JSON speichern
      }
    }
  });

}

void configureGuestbook() {
  
  server.serveStatic("/guestbook/", SD, "/guestbook")
        .setDefaultFile("index.html");

}

void setup(){
  Serial.begin(115200);
  delay(5000);

  if(!SD.begin(SPI_CS, SPI, 40000000, "/sd", 10)){
    Serial.println("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD.cardType();

  if(cardType == CARD_NONE){
    Serial.println("No SD card attached");
    return;
  }

  //define standard ssid/pw
  String ssid = "ESP32 AP Mode"; 
  String password = "123456789";
  //read ssid/pw from sd-card
  File configFile = SD.open("/wifi.txt");
  if (configFile) {
    while (configFile.available()) {
      String line = configFile.readStringUntil('\n');
      if (line.startsWith("ssid=")){
        ssid = line.substring(5);
      }
      if (line.startsWith("password=")) {
        password = line.substring(9); //needs to be empty or minimum 8 chars, otherwise standard AP
      }
    }
    configFile.close();
    Serial.println("WiFi config loaded from SD: " + ssid);
  } else {
    Serial.println("No /wifi.txt - using hardcoded defaults!");
  }
  
  //start wifi in AP mode
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  delay(3000);
  Serial.println("WiFi: " + WiFi.softAPSSID()); 

  //start local mDNS for url keychainserver.local
  if (MDNS.begin("keychainserver")) {
    Serial.println("mDNS: http://keychainserver.local");
  }

  //init different parts of webserver (comment out to disable)
  configureWebsite();
  configureGame();
  configureGuestbook();

  //start webserver
  server.begin();
  MDNS.addService("http", "tcp", 80);
  Serial.println("Async Webserver läuft!");
}

void loop(){

}

