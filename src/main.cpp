#include <Arduino.h>

#include "FS.h"
#include "SD.h"
#include "SPI.h"

#include <ESPmDNS.h>
#include <DNSServer.h>

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncJson.h>
#include <ArduinoJson.h>
#include <algorithm>
#include <vector>
#include <string>

#define SPI_CS 2
#define DNS_INTERVAL 30 // Define the DNS interval in milliseconds between processing DNS requests

const IPAddress localIP(4, 3, 2, 1);		   // the IP address the web server, Samsung requires the IP to be in public space
const IPAddress gatewayIP(4, 3, 2, 1);		   // IP address of the network should be the same as the local IP for captive portals
const IPAddress subnetMask(255, 255, 255, 0);  // no need to change: https://avinetworks.com/glossary/subnet-mask/
const String localIPURL = "http://4.3.2.1";	 // a string version of the local IP with http, used for redirecting clients to your webpage


bool debug = false; // debug can be toggle on via config file

DNSServer dnsServer;
AsyncWebServer server(80);

void configureServer() {

  server.onNotFound([](AsyncWebServerRequest *request){
    request->send(404, "text/plain", "File not found");
  });

  server.serveStatic("/config/", SD, "/config")
        .setDefaultFile("config.html");

  server.on("/api/config", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    JsonDocument doc;
    File file = SD.open("/config.json", "r");
    deserializeJson(doc, file);
    file.close();
    doc.remove("config_pw");
    serializeJson(doc, *response);
    request->send(response);
  });

  server.addHandler(new AsyncCallbackJsonWebHandler("/api/config", [](AsyncWebServerRequest *request, JsonVariant &json){
    JsonObject jsonData = json.as<JsonObject>();
    String ssid = jsonData["ssid"]|"";
    String password = jsonData["password"]|"";
    String debug = jsonData["debug"]|"off";
    String www = jsonData["www"]|"on";
    String game = jsonData["game"]|"off";
    String guestbook = jsonData["guestbook"]|"off";
    String apmode = jsonData["apmode"]|"off";
    String new_config_pw = jsonData["new_conf_pw"]|"";
    String old_config_pw = jsonData["old_conf_pw"]|"";

    JsonDocument config;
    File configFile = SD.open("/config.json","r");
    deserializeJson(config, configFile);
    configFile.close();
    String config_pw;
    if(!configFile) {config_pw = "kusch3lIsTheBest";}
    if (config_pw==old_config_pw){
      if(new_config_pw!=""){
        config["config_pw"]=new_config_pw;
      }
      if(ssid!=""){
        config["ssid"]=ssid;
      }
      config["password"]=password;
      config["debug"]=debug;
      config["www"]=www;
      config["game"]=game;
      config["guestbook"]=guestbook;
      config["apmode"]=apmode;

      serializeJsonPretty(config,Serial);

      File configFile = SD.open("/config.json","w");
      serializeJson(config, configFile);
      configFile.close();

      request->send(200,"application/json", "{\"ok\":true}");
      request->onDisconnect([](){ 
        Serial.println("restart ESP32");
        ESP.restart();
      });
      
    }else{
      serializeJsonPretty(jsonData,Serial);
      serializeJsonPretty(config,Serial);
      request->send(200,"application/json", "{\"ok\":false}");
    }
    configFile.close();
  }));
  
}

void configureWebsite() {

  server.serveStatic("/", SD, "/www")
        .setDefaultFile("index.html");

}

void configureGame() {

  server.serveStatic("/game/", SD, "/game")
        .setCacheControl("max-age=3600");
  
  server.on("/api/highscore", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    JsonDocument doc;
    File file = SD.open("/game/highscore.json", "r");
    deserializeJson(doc, file);
    file.close();
    serializeJson(doc, *response);
    request->send(response);
  });
  
  server.addHandler(new AsyncCallbackJsonWebHandler("/api/highscore", [](AsyncWebServerRequest *request, JsonVariant &json){
    JsonObject jsonData = json.as<JsonObject>();
    String name = jsonData["name"]|"unknown";
    int score = jsonData["score"]|-1;
    if (debug) {
      Serial.println("Highscore received:");
      serializeJsonPretty(jsonData,Serial);
      Serial.println();
    }
    if (name.length()>=16) name=name.substring(0,16);
    JsonDocument doc;

    // Read the file
    File file = SD.open("/game/highscore.json", "r");
    deserializeJson(doc, file);
    file.close();
    if (debug) Serial.println("File opened and Json deserialized");

    //Append new element
    JsonObject obj = doc.add<JsonObject>();
    obj["name"]=name;
    obj["score"]=score;
    if (debug) {
      serializeJsonPretty(doc,Serial);
      Serial.println();
      Serial.println("highscore appended");
    }

    //sorting everything
    struct Entry {
      int score;
      const char* name;
    };

    std::vector<Entry> entries;

    //write Json in vector  
    if (debug) Serial.println("create vector");
    for (JsonVariantConst v : doc.as<JsonArray>()) {
      entries.push_back({v["score"], v["name"]});
    }
    for (const auto& item : entries) {
      Serial.print("name: ");
      Serial.print(item.name);
      Serial.print(", score: ");
      Serial.println(item.score);
    }
    //if (debug) Serial.println(to_string(entries));

    //sort
    if (debug) Serial.println("Sortieren");
    std::sort(entries.begin(), entries.end(), [](const Entry &a, const Entry &b) {
      return a.score > b.score;
    });
    if (debug) Serial.println("fertig sortiert");

    for (const auto& item : entries) {
      Serial.print("name: ");
      Serial.print(item.name);
      Serial.print(", score: ");
      Serial.println(item.score);
    }

    //remove last entry if >=100
    if (entries.size()>=100){
      if (debug) Serial.println("100 Entries, remove last");
      entries.pop_back();
    }

    // write vektor to json
    doc.clear();
    if (debug) Serial.println("writing vektor to json");
    for (const Entry &entry : entries) {
      JsonObject v = doc.add<JsonObject>();
      v["name"] = entry.name;
      v["score"] = entry.score;
    }
    
    if (debug) serializeJsonPretty(doc, Serial);

    // Write the file
    if (debug) Serial.println("serialize json and write to sd card");
    file = SD.open("/game/highscore.json", "w");
    serializeJson(doc, file);
    file.close();
    
    request->send(200,"application/json", "{\"ok\":true}");
  }));
  /**/
}

void configureGuestbook() {
  
  server.serveStatic("/guestbook/", SD, "/guestbook")
        .setDefaultFile("index.html");

}

void configureDNSServer(DNSServer &dnsServer, AsyncWebServer &server, const IPAddress &localIP) {
	// Set the TTL for DNS response and start the DNS server
	dnsServer.setTTL(3600);
	dnsServer.start(53, "*", localIP);

  //======================== Webserver ========================
	// WARNING IOS (and maybe macos) WILL NOT POP UP IF IT CONTAINS THE WORD "Success" https://www.esp8266.com/viewtopic.php?f=34&t=4398
	// SAFARI (IOS) IS STUPID, G-ZIPPED FILES CAN'T END IN .GZ https://github.com/homieiot/homie-esp8266/issues/476 this is fixed by the webserver serve static function.
	// SAFARI (IOS) there is a 128KB limit to the size of the HTML. The HTML can reference external resources/images that bring the total over 128KB
	// SAFARI (IOS) popup browser has some severe limitations (javascript disabled, cookies disabled)

	// Required
	server.on("/connecttest.txt", [](AsyncWebServerRequest *request) { request->redirect("http://logout.net"); });	// windows 11 captive portal workaround
	server.on("/wpad.dat", [](AsyncWebServerRequest *request) { request->send(404); });								// Honestly don't understand what this is but a 404 stops win 10 keep calling this repeatedly and panicking the esp32 :)

	// Background responses: Probably not all are Required, but some are. Others might speed things up?
	// A Tier (commonly used by modern systems)
	server.on("/generate_204", [](AsyncWebServerRequest *request) { request->redirect(localIPURL); });		   // android captive portal redirect
	server.on("/redirect", [](AsyncWebServerRequest *request) { request->redirect(localIPURL); });			   // microsoft redirect
	server.on("/hotspot-detect.html", [](AsyncWebServerRequest *request) { request->redirect(localIPURL); });  // apple call home
	server.on("/canonical.html", [](AsyncWebServerRequest *request) { request->redirect(localIPURL); });	   // firefox captive portal call home
	server.on("/success.txt", [](AsyncWebServerRequest *request) { request->send(200); });					   // firefox captive portal call home
	server.on("/ncsi.txt", [](AsyncWebServerRequest *request) { request->redirect(localIPURL); });			   // windows call home

	// B Tier (uncommon)
	//  server.on("/chrome-variations/seed",[](AsyncWebServerRequest *request){request->send(200);}); //chrome captive portal call home
	//  server.on("/service/update2/json",[](AsyncWebServerRequest *request){request->send(200);}); //firefox?
	//  server.on("/chat",[](AsyncWebServerRequest *request){request->send(404);}); //No stop asking Whatsapp, there is no internet connection
	//  server.on("/startpage",[](AsyncWebServerRequest *request){request->redirect(localIPURL);});
}

void setup(){

  Serial.begin(115200);
  delay(5000);

  //mount SD Card
  if(!SD.begin(SPI_CS, SPI, 40000000, "/sd", 10)){
    Serial.println("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD.cardType();

  if(cardType == CARD_NONE){
    Serial.println("No SD card attached");
    return;
  }

  //read config from sd-card
  JsonDocument doc;
  File file = SD.open("/config.json", "r");
  deserializeJson(doc, file);
  file.close();
  String ssid = doc["ssid"]|"ESP32 AP Mode";
  String password = doc["password"]|"12345678";
  String www = doc["www"]|"on";
  String game = doc["game"]|"off";
  String guestbook = doc["guestbook"]|"off";
  String apmode = doc["apmode"]|"on";
  if(doc["debug"]=="on"){
    debug = true;
  }else{
    debug = false;
  }



  /*File configFile = SD.open("/config.txt");
  if (configFile) {
    while (configFile.available()) {
      String line = configFile.readStringUntil('\n');
      if (line.startsWith("ssid=")){
        ssid = line.substring(5);
      }
      if (line.startsWith("password=")) {
        password = line.substring(9); //needs to be empty or minimum 8 chars, otherwise standard AP
      }
      if (line.startsWith("debug=")) {
        if (line.substring(6)=="true"){
          debug = true;
        }
      }
      if (line.startsWith("www=")) {
        if (line.substring(4)=="true"){
          www = true;
        }else if(line.substring(4)=="false"){
          www = false;
        }
      }
      if (line.startsWith("game=")) {
        if (line.substring(5) == "true") {
          game = true;
        }
      }
      if (line.startsWith("guestbook=")) {
        if (line.substring(10) == "true") {
          guestbook = true;
        }
      }
      if (line.startsWith("apmode=")) {
        if (line.substring(7) == "false") {
          apmode = false;
        }
      }
    }
    configFile.close();
    Serial.println("WiFi config loaded from SD: " + ssid);
    if (debug&&www) {Serial.println("website activated");}
    if (debug&&game) {Serial.println("game activated");}
    if (debug&&guestbook) {Serial.println("guestbook activated");}
    if (debug&&!apmode) {Serial.println("station mode selected");}
  } else {
    Serial.println("No /config.txt - using hardcoded defaults!");
  }*/
  
  if (apmode=="on"){
    //start wifi in AP mode
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(localIP, gatewayIP, subnetMask); // Configure the soft access point with a specific IP and subnet mask
    WiFi.softAP(ssid, password);
    delay(3000);
    Serial.println("WiFi: " + WiFi.softAPSSID());
    auto ip=WiFi.softAPIP();
    Serial.print("IP-Adress: ");
    Serial.println(ip);
  }else{
    //log into existing wlan (for debugging purposes)
    WiFi.begin(ssid, password);
    Serial.print("Connecting to " + ssid);
    while (WiFi.status() != WL_CONNECTED) {  
      delay(500);
      Serial.print(".");
    }
    Serial.println("Succesfully connected to " + ssid);
    Serial.println("IP-Adress: " + WiFi.localIP());
  }
  
  //broken to fix!!! Not really broken, but not working on some devices???
  //start local mDNS for url keychainserver.local
  if (MDNS.begin("keychainserver")) {
    Serial.println("mDNS: http://keychainserver.local");
  }else{
    Serial.println("mDNS: failed to set up");
  }


  if (debug) {Serial.println("start server configuration");}

  configureDNSServer(dnsServer, server, localIP);

  //init different parts of webserver
  configureServer();
  if (www=="on") {
    if (debug) {Serial.println("start www configuration");}
    configureWebsite();
  }
  if (game=="on") {
    if (debug) {Serial.println("start game configuration");}
    configureGame();
  }
  if (guestbook=="on") {
    if (debug) {Serial.println("start guestbook configuration");}
    configureGuestbook();
  }

  //start webserver
  server.begin();
  MDNS.addService("http", "tcp", 80);
  Serial.println("Async Webserver läuft!");
}

void loop(){
	dnsServer.processNextRequest();	 // I call this atleast every 10ms in my other projects (can be higher but I haven't tested it for stability)
	delay(DNS_INTERVAL);	
}

