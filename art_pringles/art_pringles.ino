#include <ESP8266WiFi.h>
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
#include <animations.h>
#include <pringles.h>

#include <Bounce2.h>
#include <quickbuttons.h>


const uint16_t PixelCount = 126;
//const uint16_t PixelCount = 36;
    
LEDArtPiece art(PixelCount, 7, 18);
LAA_Flood flood("Flood", RgbColor(64,0,0));
LAA_Sparkle sparkle("Sparkle", PixelCount);
LAA_RYBRainbow rainbow("Rainbow");
LAA_Line line("Line");
LAA_BoxOutline boxOutline("Box Outline");
LAA_AllWhite allWhite("All White");
LAA_HalfWhite halfWhite("Half White");

PringlesGeometry pringlesGeom(7, 18);

QuickButtons buttons(art, &halfWhite);
  
void setup() {
  WiFi.mode(WIFI_OFF);
  Serial.begin(57600);
  Serial.print("Hello world\n");
  // This turns on wifi output
  Serial.setDebugOutput(true);

  buttons.begin();
  art.specificGeometry = &pringlesGeom;
  
//  art.registerAnimation(&allWhite);
  art.registerAnimation(&halfWhite);

//  art.registerAnimation(&flood);
  art.registerAnimation(&sparkle);
  art.registerAnimation(&line);
  art.registerAnimation(&rainbow);
//  art.registerAnimation(&boxOutline);
  art.begin();

  buttons.begin();
}

void loop() {
  buttons.loop();
  art.loop();
  
}
