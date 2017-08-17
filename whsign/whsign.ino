#include <ESP8266WiFi.h>

#include <ESPAsyncWebServer.h>

#include <WebSocketsClient.h>

#include <FS.h>

#include <whsign.h>


#define AP_NAME "Welcome Homo"
#define AP_PASS "Glamcocks!"

#define IS_MASTER false 

WHSign sign(0);

void setup() {

  Serial.begin(57600);
  Serial.setDebugOutput(true);

  if (IS_MASTER) {
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(IPAddress(192,168,10,10), IPAddress(192,168,10,10), IPAddress(255,255,255,0));
    WiFi.softAP(AP_NAME, AP_PASS);
    
    sign.enableMaster();  
    Serial.print("----- MASTER -----\n");
  } else {

    WiFi.mode(WIFI_STA);
    WiFi.begin(AP_NAME, AP_PASS);

    sign.enableSlave();
    Serial.print("----- SLAVE -----\n");
  }

//  WiFi.mode(WIFI_OFF);

//  Serial.print("Hello world\n");
  // This turns on wifi output

  sign.begin();
}

void loop() {
  // buttons.loop();
  sign.loop();
  
}
