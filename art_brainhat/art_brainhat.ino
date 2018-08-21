#define ForceDefaults false
#define NodeId 1
#define FIRMWARE_VERSION  50

#include <Arduino.h>
#include <ESPAsyncWebServer.h>

#include <LEDArt.h>
#include <nexus.h>
#include <animations.h>
#include <log.h>

#include <node_config.h>

#include <haus_fan.h> // instead of msg_tube when it won't have peers
//#include <msg_tube.h>
// #include <wifisync.h>
//#include <pinger.h>

#include <Bounce2.h>
#include <quickbuttons.h>
#include <webui.h>
#include <lase.h>

// Force use of specific geometry instead of natural rows and cols because it's not square
Nexus nx;


// Circle LED layout
// Outside to inside 24, 16, 12, 8, 1 = 61  
// 3.66 amps at full brightness, 2.7 amps @ 74% brightness = 188

const uint8_t MaxBrightness = 180;
const uint16_t PixelCount = 61;

LEDArtPiece art(nx, PixelCount, MaxBrightness);

LEDArtSingleGeometry geomAll("All", PixelCount);


uint16_t table[][24] = {
  {  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23 },
  { 24, 25, 25, 26, 27, 27, 28, 29, 29, 30, 31, 31, 32, 33, 33, 34, 35, 35, 36, 37, 37, 38, 39, 39 },
  { 40, 40, 41, 41, 42, 42, 43, 43, 44, 44, 45, 45, 46, 46, 47, 47, 48, 48, 49, 49, 50, 50, 51, 51 },
  { 52, 52, 53, 53, 53, 54, 54, 54, 55, 55, 55, 56, 56, 56, 57, 57, 57, 58, 58, 58, 59, 59, 59, 52 },
  { 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60 }
};
LEDArtTableGeometry geomRays("Rays", 5, 24, (uint16_t**)table);

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

LAA_Kitt kitt("Kitt");
LAA_KittSmooth kittSmooth("Kitt Smooth");

LAA_KittPallete kittPallete("Kitt Pallete");

QuickButtons buttons(art, &halfWhite);

WebUI webui(nx, art);
//WiFiSync wifiSync(nx);

//Pinger pinger;

Lase lase(nx, art);

void setup() {

  ////// Configure logging / debugging
  Serial.begin(57600);
  // Serial.print("Hello world\n");
  // This turns on wifi debug output to the serial line
  Serial.setDebugOutput(true);
  Log.setSerialEnabled(true);
  Log.printf("DB Log start\n");

  NodeConfig.begin(
    ForceDefaults, 
    NodeId, 
    "TomArtBrainHat", "ILoveTwinks", 
    (uint32_t)IPAddress(10,0,1,10),  // Lase Host
    (uint32_t)IPAddress(10,10,9,10), // Base address for peers in mesh mode
    
    (uint32_t)IPAddress(10,10,10,100),  // Static mode address of the master
    (uint32_t)IPAddress(10,10,10,254)  // Static mode gateway for the master
  );

  // Things we want to override stored values for in this firmware
  NodeConfig.setLaseHost(IPAddress(10,10,10,4));

  /////// Configure network and hardware UI
  // msgTube.configure(NODE_ID, "TomArtFIPStrip", "ILoveTwinks");
  // msgTube.begin();
  hausFan.setPossibleNet(false, "Haus", "GundamWing");
  hausFan.begin();
  
  buttons.begin();

  /////// Configure the art piece
  art.registerGeometry(&geomAll);
//  art.registerGeometry(&geomRays);

  // Could set different defaults here if we care
  // nx.unitType = 1;
//  nx.palette = LEDArtAnimation::LEDPalette_BLUES;
  //nx.palette = LEDArtAnimation::LEDPalette_RYB;
//  nx.speedFactor = 0.25;

  // Default animation time is 16 seconds which is 8 bars at 120bpm
  // so when using a small duration setting it to a multiple of this is good
  // nx.maxDuration = 32000;
 nx.maxDuration = 256000;
  
  art.registerAnimation(&webui.statusAnim);

//  art.registerAnimation(&allWhite);
//  art.registerAnimation(&halfWhite);
//
//  art.registerAnimation(&flood);
  art.registerAnimation(&unitFill);
  art.registerAnimation(&randoFill);
  art.registerAnimation(&paletteFill);
//  art.registerAnimation(&sparkle);
//  art.registerAnimation(&line);
  art.registerAnimation(&rainbow);
//  art.registerAnimation(&boxOutline);
//  art.registerAnimation(&kitt);
  art.registerAnimation(&kittSmooth);
  art.registerAnimation(&kittPallete);
  art.begin();
//
    nx.addListener(&art);

//  // Start the webui animation just so we don't have to deal with it
//  // elsewhere right now
    art.startAnimation(&webui.statusAnim, false);

    art.startAnimation(&unitFill, false);
//    art.startAnimation(&wifiSync.statusAnim, false);
    art.startAnimation(&kittPallete, false);

// Need to begin the strip when debugging, but art.begin() does this on it's own
//    art.strip.Begin();

  webui.begin();
  // wifiSync.begin();
  hausFan.begin();

  // Do these last so randomization doesn't affect it
  nx.foreground = RgbColor(255,0,0);
  nx.background = RgbColor(0, 0, 0);
  nx.speedFactor = 1.0;
  
//  pinger.begin();
//  if (NODE_ID == 2)
//  {
//    pinger.startPings(0, 5000);
//  }

  lase.begin();
}

uint8_t count = 0;

void loop() {
  // msgTube.loop();
  // wifiSync.loop();
  hausFan.loop();
  webui.loop();
//  pinger.loop();

  buttons.loop();
  art.loop();

  lase.loop();
}
