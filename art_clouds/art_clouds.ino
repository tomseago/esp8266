#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>

#include <LEDArt.h>
#include <nexus.h>
#include <animations.h>
#include <webui.h>

#include <FS.h>

const uint16_t PixelCount = 100;
//const uint16_t PixelCount = 36;

Nexus nx(2);
LEDArtPiece art(nx, PixelCount, 192, 20, 5);
//LAA_Flood flood("Flood", RgbColor(64,0,0));
LAA_Sparkle2 sparkle("Sparkle", PixelCount);
LAA_Rainbow rainbow("Rainbow");
//LAA_Line line("Line");
//LAA_BoxOutline boxOutline("Box Outline");
//LAA_AllWhite allWhite("All White");
//LAA_HalfWhite halfWhite("Half White");
LAA_RandoFill randoFill("Rando Fill");

// PringlesGeometry pringlesGeom(7, 18);

// QuickButtons buttons(art, &halfWhite);

WebUI webui(nx);
  
void setup() {
//  WiFi.mode(WIFI_OFF);
  WiFi.mode(WIFI_STA);
  WiFi.begin("Toms Little Helper", "ILoveTwinks");

  Serial.begin(57600);
  Serial.print("Hello world\n");
  // This turns on wifi output
  Serial.setDebugOutput(true);

  webui.begin();

//  buttons.begin();
  // art.specificGeometry = &pringlesGeom;
  nx.unitType = 3;
//  nx.palette = LEDArtAnimation::LEDPalette_BLUES;
  nx.palette = LEDArtAnimation::LEDPalette_RYB;
//  nx.speedFactor = 0.25;
  nx.maxDuration = 300000;

  art.registerAnimation(&webui.statusAnim);
  art.registerAnimation(&sparkle);
  art.registerAnimation(&randoFill);
  art.registerAnimation(&rainbow);
  art.begin();

  nx.addListener(&art);

  // Start the webui animation just so we don't have to deal with it
  // elsewhere right now
  art.startAnimation(&webui.statusAnim, false);
}

void loop() {
  // buttons.loop();
  art.loop();
  
}
