#define NODE_ID 1

#include <ESP8266WiFi.h>

#include <ESPAsyncWebServer.h>

//#include <ESP8266WiFiAP.h>
//#include <ESP8266WiFiGeneric.h>
//#include <ESP8266WiFiMulti.h>
//#include <ESP8266WiFiScan.h>
//#include <ESP8266WiFiSTA.h>
//#include <ESP8266WiFiType.h>
//#include <WiFiClient.h>
//#include <WiFiClientSecure.h>
//#include <WiFiServer.h>
//#include <WiFiUdp.h>

//#include <WiFi.h>

#include <LEDArt.h>
#include <nexus.h>
#include <animations.h>
#include <log.h>
#include <msg_tube.h>

#include <Bounce2.h>
#include <quickbuttons.h>
#include <webui.h>
#include <pinger.h>

Nexus nx;

//const uint16_t PixelCount = 60;
const uint16_t PixelCount = 144;  // 48 * 3
const uint8_t MaxBrightness = 128;
const uint16_t Width = 1;
const uint16_t Height = 4;
LEDArtPiece art(nx, PixelCount, MaxBrightness, Width, Height);

    
LAA_Flood flood("Flood", RgbColor(255,0,0));
LAA_Sparkle sparkle("Sparkle", PixelCount);
LAA_Rainbow rainbow("Rainbow");
LAA_Line line("Line");
LAA_BoxOutline boxOutline("Box Outline");
LAA_AllWhite allWhite("All White");
LAA_HalfWhite halfWhite("Half White");
LAA_RandoFill randoFill("Rando Fill");

// PringlesGeometry pringlesGeom(7, 18);

QuickButtons buttons(art, &halfWhite);

WebUI webui(nx);

Pinger pinger;

void setup() {

  Serial.begin(57600);
  // Serial.print("Hello world\n");
  // This turns on wifi output
  Serial.setDebugOutput(true);
  Log.setSerialEnabled(true);
  Log.printf("DB Log start\n");

  msgTube.configure(NODE_ID, "Pyramid", "ILoveTwinks");
  //msgTube.begin();
  
//  buttons.begin();
//  art.specificGeometry = &pringlesGeom;
  nx.unitType = 1;
//  nx.palette = LEDArtAnimation::LEDPalette_BLUES;
  nx.palette = LEDArtAnimation::LEDPalette_RYB;
//  nx.speedFactor = 0.25;
  nx.maxDuration = 300000;
  
  art.registerAnimation(&webui.statusAnim);

//  art.registerAnimation(&allWhite);
//  art.registerAnimation(&halfWhite);
//
  art.registerAnimation(&flood);
  art.registerAnimation(&sparkle);
//  art.registerAnimation(&line);
  art.registerAnimation(&rainbow);
//  art.registerAnimation(&randoFill);
//  art.registerAnimation(&boxOutline);
  art.begin();
//
    nx.addListener(&art);

//  // Start the webui animation just so we don't have to deal with it
//  // elsewhere right now
    art.startAnimation(&webui.statusAnim, false);
    art.startAnimation(&flood, false);

//    art.strip.Begin();
  webui.begin();

//  pinger.begin();
//  if (NODE_ID == 2)
//  {
//    pinger.startPings(0, 5000);
//  }

  pinMode(LED_BUILTIN, OUTPUT);
}

uint8_t count = 0;
uint32_t last = 0;
bool state = false;

void loop() {
  //msgTube.loop();
//  pinger.loop();

//  buttons.loop();
  art.loop();

  uint32_t now = millis();
  if (now - last > 1000) {
    state = !state;
    digitalWrite(LED_BUILTIN, state ? HIGH : LOW);
    last = now;
  }
  

// art.strip.ClearTo(RgbColor(255,0,0), 0, 2);
//  RgbColor red(255,0,0);
//  art.strip.SetPixelColor(0, red);
//  art.strip.Show();
//  Log.printf("L ");  
//  if (count++>=10) {
//    Log.printf("\n");
//    count = 0;
//  }
//  yield();
}
