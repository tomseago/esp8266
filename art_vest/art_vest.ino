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
#include <vest.h>

#include <Bounce2.h>
#include <quickbuttons.h>
#include <webui.h>

Nexus nx;

//const uint16_t PixelCount = 168;

const uint16_t PixelCount = 168 + 4;

// nexus, pixelCount, maxBrightness, width=1, height=1, port=0
LEDArtPiece art(nx, PixelCount, 128, 16, 18);

LAA_Flood flood("Flood", RgbColor(64,0,0));
LAA_Sparkle sparkle("Sparkle", PixelCount);
LAA_Rainbow rainbow("Rainbow");
LAA_Line line("Line");
LAA_BoxOutline boxOutline("Box Outline");
LAA_AllWhite allWhite("All White");
//LAA_HalfWhite halfWhite("Half White");
LAA_RandoFill randoFill("Rando Fill");
//LAA_DimDebug dimDebug("Dim Debug");

VestGeometry vestGeom(16, 18, 6, 12);

QuickButtons buttons(art, &allWhite);

WebUI webui(nx);

void setup() {
  WiFi.mode(WIFI_OFF);
  Serial.begin(57600);
  Serial.print("Hello world\n");
  // This turns on wifi output
  Serial.setDebugOutput(true);

  buttons.begin();
  art.specificGeometry = &vestGeom;
  nx.unitType = 4;
//  nx.palette = LEDArtAnimation::LEDPalette_BLUES;
  nx.palette = LEDArtAnimation::LEDPalette_RYB;
//  nx.speedFactor = 0.25;
  nx.maxDuration = 300000;
  
  art.registerAnimation(&webui.statusAnim);

////  art.registerAnimation(&allWhite);
//  art.registerAnimation(&halfWhite);
//

  art.registerAnimation(&flood);
  art.registerAnimation(&sparkle);
  art.registerAnimation(&line);
  art.registerAnimation(&rainbow);
  art.registerAnimation(&randoFill);
//  art.registerAnimation(&boxOutline);

//  art.registerAnimation(&dimDebug);
  art.begin();

  buttons.begin();

  nx.addListener(&art);

  // Start the webui animation just so we don't have to deal with it
  // elsewhere right now
  art.startAnimation(&webui.statusAnim, false);
}

void loop() {
  buttons.loop();
  art.loop();
  
}
