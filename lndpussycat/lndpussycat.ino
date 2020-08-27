//////////////LOAD LIBRARIES////////////////

#include "FS.h"
#include <WiFiManager.h> 
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>
#include "SPIFFS.h"

/////////////////SOME VARIABLES///////////////////

int new_balance = 0;
int balance = 0;
int new_confirmed_balance = 0;
int confirmed_balance = 0;
bool synced_to_chain = false;
bool first_check = true;
bool first_check1 = true;
const char* lnd_check;

char lnd_server[40];
char lnd_port[6]  = "8080";
char lnd_macaroon[500] = "";
char static_ip[16] = "10.0.1.56";
char static_gw[16] = "10.0.1.1";
char static_sn[16] = "255.255.255.0";

bool shouldSaveConfig = false;

const char* spiffcontent = "";
String spiffing; 
/////////////////////SETUP////////////////////////

void setup() {
  Serial.begin(115200);
  
// CATHODE RGB SETUP, FOR ANNODE CHANGE 18 TO LOW
  pinMode(16, OUTPUT);
  pinMode(17, OUTPUT); 
  pinMode(5, OUTPUT);

  pinMode(18, OUTPUT);
  digitalWrite(18, HIGH); 
  pink();
  
// START PORTAL 
  pussy_portal();

  blue();
}


///////////////////MAIN LOOP//////////////////////

void loop() {
  getinfo();
  delay(3000);
  getonchainbalance();
  delay(3000);
  getlnbalance();
  delay(3000);
}


//////////////////LED SEQUENCES///////////////////

//GREEN -- running
void green(){
  digitalWrite(16, HIGH); digitalWrite(17, HIGH); digitalWrite(5, LOW);
}

//RED -- down
void red(){
  digitalWrite(16, HIGH); digitalWrite(17, LOW); digitalWrite(5, HIGH);
}

// PINK -- synching
void pink(){
  digitalWrite(16, LOW); digitalWrite(17, LOW); digitalWrite(5, HIGH);
}

// PINK -- synching
void blue(){
  digitalWrite(16, LOW); digitalWrite(17, HIGH); digitalWrite(5, HIGH);
}

//DISCO LOOP // transaction
void disco(){
  for (int i = 0; i <= 10; i++) {
    digitalWrite(16, LOW); digitalWrite(18, HIGH); digitalWrite(5, HIGH);
    delay(100);
    digitalWrite(16, HIGH); digitalWrite(18, HIGH); digitalWrite(5, LOW);
    delay(100);
    digitalWrite(16, HIGH); digitalWrite(18, LOW); digitalWrite(5, HIGH);
    delay(100);
    digitalWrite(16, LOW); digitalWrite(18, LOW); digitalWrite(5, HIGH);
    delay(100);
    digitalWrite(16, LOW); digitalWrite(18, HIGH); digitalWrite(5, LOW);
    delay(100);
    digitalWrite(16, HIGH); digitalWrite(18, LOW); digitalWrite(5, LOW);
    delay(100);
  }
  green();
}


//////////////////NODE CALLS///////////////////

void getinfo() {
  WiFiClientSecure client;
  const char* lndserver = lnd_server;
  const char* macaroon = lnd_macaroon;
  int lndport = atoi( lnd_port );
  Serial.println(lndserver);
  Serial.println(lndport);
  if (!client.connect(lndserver, lndport)){
    red();
    return;   
  }
  client.print(String("GET ")+ "https://" + lndserver +":"+ lndport + "/v1/getinfo HTTP/1.1\r\n" +
                 "Host: "  + lndserver +":"+ lndport +"\r\n" +
                 "User-Agent: ESP322\r\n" +
                 "Grpc-Metadata-macaroon:" + macaroon + "\r\n" +
                 "Content-Type: application/json\r\n" +
                 "Connection: close\r\n" +
                 "\n");
    String line = client.readStringUntil('\n');
    while (client.connected()) {
     String line = client.readStringUntil('\n');
     if (line == "\r") {    
       break;
     }
    }
    String content = client.readStringUntil('\n');
    client.stop();
    const size_t capacity = JSON_OBJECT_SIZE(3) + 620;
    DynamicJsonDocument doc(capacity);
    deserializeJson(doc, content); 
    lnd_check = doc["alias"];
    if (!lnd_check){
      red();
      delay(1000);
      return;   
    }
    synced_to_chain = doc["synced_to_chain"]; 
    if(synced_to_chain == true){
      green();
    }
    else{
      pink();
    }
}

void getonchainbalance() {
  WiFiClientSecure client;
  const char* lndserver = lnd_server;
  const char* macaroon = lnd_macaroon;
  int lndport = atoi( lnd_port );
  if (!client.connect(lndserver, lndport)){
    red();
    delay(1000);
    return;   
  }
  client.print(String("GET ")+ "https://" + lndserver +":"+ lndport + "/v1/balance/blockchain HTTP/1.1\r\n" +
                 "Host: "  + lndserver +":"+ lndport +"\r\n" +
                 "User-Agent: ESP322\r\n" +
                 "Grpc-Metadata-macaroon:" + macaroon + "\r\n" +
                 "Content-Type: application/json\r\n" +
                 "Connection: close\r\n" +
                 "\n");
    String line = client.readStringUntil('\n');
    while (client.connected()) {
     String line = client.readStringUntil('\n');
     if (line == "\r") {    
       break;
     }
    }
    String content = client.readStringUntil('\n');
    client.stop();
    const size_t capacity = JSON_OBJECT_SIZE(3) + 620;
    DynamicJsonDocument doc(capacity);
    deserializeJson(doc, content);
    new_confirmed_balance  = doc["confirmed_balance"];
    if(new_confirmed_balance > confirmed_balance){
      if(first_check == false){
        confirmed_balance = new_confirmed_balance;
        Serial.println(new_confirmed_balance);
        disco();
      }
      else{
        confirmed_balance = new_confirmed_balance;
        first_check = false;
      }
    }
    Serial.println("confirmed_balance " + String(confirmed_balance));
    Serial.println("new_confirmed_balance " + String(new_confirmed_balance));
}


    
void getlnbalance() {
  WiFiClientSecure client;
  const char* lndserver = lnd_server;
  const char* macaroon = lnd_macaroon;
  int lndport = atoi( lnd_port );
  if (!client.connect(lndserver, lndport)){
    red();
    return;   
  }
  client.print(String("GET ")+ "https://" + lndserver +":"+ lndport + "/v1/balance/channels HTTP/1.1\r\n" +
                 "Host: "  + lndserver +":"+ lndport +"\r\n" +
                 "User-Agent: ESP322\r\n" +
                 "Grpc-Metadata-macaroon:" + macaroon + "\r\n" +
                 "Content-Type: application/json\r\n" +
                 "Connection: close\r\n" +
                 "\n");
    String line = client.readStringUntil('\n');
    while (client.connected()) {
     String line = client.readStringUntil('\n');
     if (line == "\r") {    
       break;
     }
    }
    String content = client.readStringUntil('\n');
    client.stop();
    const size_t capacity = JSON_OBJECT_SIZE(3) + 620;
    DynamicJsonDocument doc(capacity);
    deserializeJson(doc, content);
    new_balance = doc["balance"];
    if(new_balance > balance){
      if(first_check1 == false){
        balance = new_balance;
        disco();
      }
      else{
        balance = new_balance;
        first_check1 = false;
      }
    }
    Serial.println("balance " + String(balance));
    Serial.println("new_balance " + String(new_balance));
}

void pussy_portal(){

  WiFiManager wm;
  Serial.println("mounting FS...");
  while(!SPIFFS.begin(true)){
    Serial.println("failed to mount FS");
    delay(200);
   }

//CHECK IF RESET IS TRIGGERED/WIPE DATA
  for (int i = 0; i <= 5; i++) {
    if(touchRead(4) < 50){
    File file = SPIFFS.open("/config.txt", FILE_WRITE);
    file.print("placeholder");
    wm.resetSettings();
    }
    delay(1000);
  }

//MOUNT FS AND READ CONFIG.JSON
  File file = SPIFFS.open("/config.txt");
  
  spiffing = file.readStringUntil('\n');
  spiffcontent = spiffing.c_str();
  DynamicJsonDocument json(1024);
  deserializeJson(json, spiffcontent);
  if(String(spiffcontent) != "placeholder"){
    strcpy(lnd_server, json["lnd_server"]);
    strcpy(lnd_port, json["lnd_port"]);
    strcpy(lnd_macaroon, json["lnd_macaroon"]);
  }

//ADD PARAMS TO WIFIMANAGER
  wm.setSaveConfigCallback(saveConfigCallback);
  
  WiFiManagerParameter custom_lnd_server("server", "LND server", lnd_server, 40);
  WiFiManagerParameter custom_lnd_port("port", "LND port", lnd_port, 6);
  WiFiManagerParameter custom_lnd_macaroon("macaroon", "LND readonly macaroon", lnd_macaroon, 500);
  wm.addParameter(&custom_lnd_server);
  wm.addParameter(&custom_lnd_port);
  wm.addParameter(&custom_lnd_macaroon);
  
//IF RESET WAS TRIGGERED, RUN PORTAL AND WRITE FILES
  if (!wm.autoConnect("LND Pussycat (･ω･)⚡", "pikachu1")) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    ESP.restart();
    delay(5000);
  }
  Serial.println("connected :)");
  strcpy(lnd_server, custom_lnd_server.getValue());
  strcpy(lnd_port, custom_lnd_port.getValue());
  strcpy(lnd_macaroon, custom_lnd_macaroon.getValue());
  if (shouldSaveConfig) {
    Serial.println("saving config");
    DynamicJsonDocument json(1024);
    json["lnd_server"] = lnd_server;
    json["lnd_port"]   = lnd_port;
    json["lnd_macaroon"]   = lnd_macaroon;

    File configFile = SPIFFS.open("/config.txt", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
      }
      serializeJsonPretty(json, Serial);
      serializeJson(json, configFile);
      configFile.close();
      shouldSaveConfig = false;
  }

  Serial.println("local ip");
  Serial.println(WiFi.localIP());
  Serial.println(WiFi.gatewayIP());
  Serial.println(WiFi.subnetMask());
}

void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}


///////////OTHER COLOURS YOU COULD USE////////////
//AQUA digitalWrite(16, LOW); digitalWrite(17, HIGH); digitalWrite(5, LOW);
//LIME digitalWrite(16, HIGH); digitalWrite(17, LOW); digitalWrite(5, LOW);
