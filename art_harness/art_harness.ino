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
#include <harness.h>
#include <log.h>
//#include <msg_tube.h>
#include <wifisync.h>

#include <Bounce2.h>
#include <quickbuttons.h>
#include <webui.h>
//#include <pinger.h>

// Force use of specific geometry instead of natural rows and cols because it's not square
Nexus nx;

//#define TEST_FOUR

#ifdef TEST_FOUR
const uint8_t MaxBrightness = 128;
const uint16_t PixelCount = 4;
const uint16_t Width = 1;
const uint16_t Height = 4;
#else

// Top straps 14 x 2 = 28
// Bottom straps 15 x 2 = 30
// Total =  58

const uint8_t MaxBrightness = 170;
const uint16_t PixelCount = 58;
const uint16_t Width = 30;
const uint16_t Height = 2;
#endif

LEDArtPiece art(nx, PixelCount, MaxBrightness);

LEDArtSingleGeometry geomAll("All", PixelCount);
//LEDArtTopoGeometry<ColumnMajor270Layout> geomTopo("Topo", NeoTopology<ColumnMajor270Layout>(30,2));
HarnessGeometry geomHarness;
HarnessZGeometry geomHarnessZ;

// Bottom straps size
//HarnessGeometry geom(30, 2);
    
//LAA_Flood flood("Flood", RgbColor(128,0,0));
LAA_Sparkle sparkle("Sparkle", PixelCount);
LAA_Rainbow rainbow("Rainbow");
LAA_Line line("Line");
LAA_BoxOutline boxOutline("Box Outline");
LAA_AllWhite allWhite("All White");
LAA_HalfWhite halfWhite("Half White");

LAA_UnitFill unitFill("Unit Fill");
LAA_RandoFill randoFill("Rando Fill");
LAA_PaletteFill paletteFill("Palette Fill");

QuickButtons buttons(art, &halfWhite);

WebUI webui(nx, art);
WiFiSync wifiSync(nx);

//Pinger pinger;

void setup() {

  ////// Configure logging / debugging
  Serial.begin(57600);
  // Serial.print("Hello world\n");
  // This turns on wifi debug output to the serial line
  Serial.setDebugOutput(true);
  Log.setSerialEnabled(true);
  Log.printf("DB Log start\n");

  /////// Configure network and hardware UI
  msgTube.configure(NODE_ID, "TomArt", "ILoveTwinks");
  msgTube.begin();
  
  buttons.begin();

  /////// Configure the art piece
  art.registerGeometry(&geomHarnessZ);
  art.registerGeometry(&geomHarness);
  art.registerGeometry(&geomAll);
  //art.registerGeometry(&geomTopo);

  // Could set different defaults here if we care
  // nx.unitType = 1;
//  nx.palette = LEDArtAnimation::LEDPalette_BLUES;
  //nx.palette = LEDArtAnimation::LEDPalette_RYB;
//  nx.speedFactor = 0.25;

#ifdef TEST_FOUR
  nx.maxDuration = 10000;
#else
  // Default animation time is 16 seconds which is 8 bars at 120bpm
  // so when using a small duration setting it to a multiple of this is good
  nx.maxDuration = 32000;
//  nx.maxDuration = 3000000;
#endif
  
  art.registerAnimation(&webui.statusAnim);

//  art.registerAnimation(&allWhite);
//  art.registerAnimation(&halfWhite);
//
//  art.registerAnimation(&flood);
  art.registerAnimation(&unitFill);
  art.registerAnimation(&randoFill);
  art.registerAnimation(&paletteFill);
  art.registerAnimation(&sparkle);
//  art.registerAnimation(&line);
  art.registerAnimation(&rainbow);
//  art.registerAnimation(&boxOutline);
  art.begin();
//
    nx.addListener(&art);

//  // Start the webui animation just so we don't have to deal with it
//  // elsewhere right now
    art.startAnimation(&webui.statusAnim, false);

    art.startAnimation(&unitFill, false);
//    art.startAnimation(&wifiSync.statusAnim, false);

// Need to begin the strip when debugging, but art.begin() does this on it's own
//    art.strip.Begin();

  webui.begin();
  wifiSync.begin();
  
//  pinger.begin();
//  if (NODE_ID == 2)
//  {
//    pinger.startPings(0, 5000);
//  }
}

uint8_t count = 0;

void loop() {
  msgTube.loop();
  wifiSync.loop();
  webui.loop();
//  pinger.loop();

  buttons.loop();
  art.loop();
  

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
