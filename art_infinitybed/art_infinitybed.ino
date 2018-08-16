// The master node id is 1 to avoid weirdness with IP addresses
#define NODE_ID 2

#include <ESP8266WiFi.h>
  
#include <ESPAsyncWebServer.h>

#include <LEDArt.h>
#include <nexus.h>
#include <animations.h>
#include <log.h>

//#include <haus_fan.h> // instead of msg_tube when it won't have peers
#include <msg_tube.h>
#include <wifisync.h>
#include <pinger.h>

#include <Bounce2.h>
#include <quickbuttons.h>
#include <webui.h>
#include <lase.h>

// Force use of specific geometry instead of natural rows and cols because it's not square
Nexus nx;


// 104 leds total
// 16 on short sides, 36 on long sides
const uint8_t MaxBrightness = 255;  // Hell to the yeah!
const uint16_t PixelCount = 104;

LEDArtPiece art(nx, PixelCount, MaxBrightness);

LEDArtSingleGeometry geomAll("All", PixelCount);


// Bottom straps size
//HarnessGeometry geom(30, 2);
    
//LAA_Flood flood("Flood", RgbColor(128,0,0));
LAA_Sparkle sparkle("Sparkle", PixelCount);
LAA_Rainbow rainbow("Rainbow");
//LAA_Line line("Line");
//LAA_BoxOutline boxOutline("Box Outline");
LAA_AllWhite allWhite("All White");
//LAA_HalfWhite halfWhite("Half White");
//
LAA_UnitFill unitFill("Unit Fill");
//LAA_RandoFill randoFill("Rando Fill");
//LAA_PaletteFill paletteFill("Palette Fill");
//
//LAA_Kitt kitt("Kitt");
//LAA_KittSmooth kittSmooth("Kitt Smooth");
//
//LAA_KittPallete kittPallete("Kitt Pallete");

QuickButtons buttons(art, &allWhite);

WebUI webui(nx, art);
WiFiSync wifiSync(nx);

//Pinger pinger;

Lase lase(IPAddress(10,0,1,10), NODE_ID-1, nx, art);

void setup() {

  ////// Configure logging / debugging
  Serial.begin(57600);
  // Serial.print("Hello world\n");
  // This turns on wifi debug output to the serial line
  Serial.setDebugOutput(true);
  Log.setSerialEnabled(true);
  Log.printf("DB Log start\n");

  /////// Configure network and hardware UI
//   msgTube.configure(NODE_ID, "InfinityBed", "Password");
   msgTube.configure(NODE_ID, "Haus", "GundamWing");
   msgTube.enableStatic();
   msgTube.begin();
//  hausFan.configure("InfinityBed", "Password");
//  hausFan.setPossibleNet(false, "Haus", "GundamWing");
//  hausFan.begin();
  
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
 //nx.maxDuration = 256000;

  // For testing message tube we want small duration
  nx.maxDuration = 8000;
  
  art.registerAnimation(&webui.statusAnim);

//  art.registerAnimation(&allWhite);
//  art.registerAnimation(&halfWhite);
//
//  art.registerAnimation(&flood);
  art.registerAnimation(&unitFill);
//  art.registerAnimation(&randoFill);
//  art.registerAnimation(&paletteFill);
//  art.registerAnimation(&sparkle);
//  art.registerAnimation(&line);
  art.registerAnimation(&rainbow);
//  art.registerAnimation(&boxOutline);
//  art.registerAnimation(&kitt);
//  art.registerAnimation(&kittSmooth);
//  art.registerAnimation(&kittPallete);
  art.begin();
//
    nx.addListener(&art);

//  // Start the webui animation just so we don't have to deal with it
//  // elsewhere right now
    art.startAnimation(&webui.statusAnim, false);

    art.startAnimation(&rainbow, false);
//    art.startAnimation(&wifiSync.statusAnim, false);
//    art.startAnimation(&kittPallete, false);

// Need to begin the strip when debugging, but art.begin() does this on it's own
//    art.strip.Begin();

  webui.begin();
   wifiSync.begin();
//  hausFan.begin();

  // Do these last so randomization doesn't affect it
  nx.foreground = RgbColor(255,0,0);
  nx.background = RgbColor(0, 0, 0);
  nx.speedFactor = 1.0;
  nx.palette = LEDArtAnimation::LEDPalette_RYB;
  
//  pinger.begin();
//  if (NODE_ID == 2)
//  {
//    pinger.startPings(1, 5000);
//  }

  lase.begin();
}

uint8_t count = 0;

void loop() {
   msgTube.loop();
   wifiSync.loop();
//  hausFan.loop();
  webui.loop();
//  pinger.loop();

  buttons.loop();
  art.loop();

  lase.loop();
}
