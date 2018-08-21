#define ForceDefaults false
#define NodeId 1
#define FIRMWARE_VERSION  50

// The master node id is 1 to avoid weirdness with IP addresses
//#define NODE_ID 1

#define MAX_NODE_ID 14

#include <Arduino.h>
//#include <WiFi.h>
//
#include <ESPAsyncWebServer.h>

#include <LEDArt.h>
#include <nexus.h>
#include <animations.h>
#include <log.h>

#include <spaceframe.h>

#include <node_config.h>
//#include <haus_fan.h> // instead of msg_tube when it won't have peers
#include <msg_tube.h>
#include <wifisync.h>
#include <pinger.h>

#include <Bounce2.h>
#include <quickbuttons.h>
#include <webui.h>
#include <lase.h>
#include <ota_updater.h>

#include <evil_doer.h>

#include <tagged_buffer.h>

// Force use of specific geometry instead of natural rows and cols because it's not square
Nexus nx;


const uint8_t MaxBrightness = 255;  // Hell to the yeah!

// 100 leds total
// 16 on short sides, 34 on long sides

const uint16_t PixelCount = 100;

LEDArtPiece art(nx, PixelCount, MaxBrightness);


LEDArtSingleGeometry geomAll("All", PixelCount, false);
SFGeomPanelEdges geomPanelEdges("Panel Edges");
//SFGeomSideGrid geomSideGrid("Side Grid");
SFGeomWrappedOver geomWrappedOver("Wrapped Over");
SFGeomWrappedLong geomWrappedLong("Wrapped Long");


// No    
//LAA_Flood flood("Flood", RgbColor(128,0,0));
LAA_Sparkle sparkle("Sparkle", PixelCount);
//LAA_Line line("Line");
LAA_BoxOutline boxOutline("Box Outline");
LAA_AllWhite allWhite("All White");
//LAA_HalfWhite halfWhite("Half White");
//
LAA_RandoFill randoFill("Rando Fill");
LAA_PaletteFill paletteFill("Palette Fill");
//
//LAA_Kitt kitt("Kitt");
//LAA_KittSmooth kittSmooth("Kitt Smooth");
//
//LAA_KittPallete kittPallete("Kitt Pallete");

// Yes
LAA_UnitFill unitFill("Unit Fill");
LAA_Rainbow rainbow("Rainbow");
LAA_RowScan rowScan("Row Scan");

QuickButtons buttons(art, &allWhite);

WebUI webui(nx, art);
WiFiSync wifiSync(nx);

//Pinger pinger;

Lase lase(nx, art);
OtaUpdater ota(FIRMWARE_VERSION);

EvilDoer evil(nx);

void setup() {

  ////// Configure logging / debugging
  Serial.begin(57600);
  // Serial.print("Hello world\n");
  // This turns on wifi debug output to the serial line
  Serial.setDebugOutput(true);
  Log.setSerialEnabled(true);
  Log.printf("DB Log start\n");

  TaggedBuffer::testCase();

  NodeConfig.begin(
    ForceDefaults, 
    NodeId, 
    "SpaceFrame", "password", 
    (uint32_t)IPAddress(10,0,1,10),  // Lase Host
    (uint32_t)IPAddress(10,10,9,10), // Base address for peers in mesh mode
    
    (uint32_t)IPAddress(10,10,10,100),  // Static mode address of the master
    (uint32_t)IPAddress(10,10,10,254)  // Static mode gateway for the master
  );

  // Things we want to override stored values for in this firmware
  NodeConfig.setLaseHost(IPAddress(10,10,10,4));

  /////// Configure network and hardware UI
  // msgTube.enableStatic();
  msgTube.enableUDP(MAX_NODE_ID);
  msgTube.begin();

//  hausFan.configure("InfinityBed", "Password");
//  hausFan.setPossibleNet(false, "Haus", "GundamWing");
//  hausFan.begin();
  
  buttons.begin();

  /////// Configure the art piece
  art.registerGeometry(&geomAll);
//  art.registerGeometry(&geomSideGrid);
  art.registerGeometry(&geomPanelEdges);
  art.registerGeometry(&geomWrappedOver);
  art.registerGeometry(&geomWrappedLong);


  // Could set different defaults here if we care
  // nx.unitType = 1;
//  nx.palette = LEDArtAnimation::LEDPalette_BLUES;
  //nx.palette = LEDArtAnimation::LEDPalette_RYB;
//  nx.speedFactor = 0.25;

  // Default animation time is 16 seconds which is 8 bars at 120bpm
  // so when using a small duration setting it to a multiple of this is good
  nx.maxDuration = 256000;

  // For testing message tube we want small duration
  // nx.maxDuration = 3000;
  
  art.registerAnimation(&webui.statusAnim);

  // Generally these shouldn't be registered because they're for flashlight mode  
//  art.registerAnimation(&allWhite);
//  art.registerAnimation(&halfWhite);


//  art.registerAnimation(&boxOutline);

//  art.registerAnimation(&flood);
  art.registerAnimation(&randoFill);
  art.registerAnimation(&paletteFill);
  art.registerAnimation(&sparkle);
//  art.registerAnimation(&line);
//  art.registerAnimation(&kitt);
//  art.registerAnimation(&kittSmooth);

//  art.registerAnimation(&kittPallete);

  art.registerAnimation(&unitFill);
  art.registerAnimation(&rainbow);
  art.registerAnimation(&rowScan);

  // Things we don't want randomly selected
  paletteFill.isEnabled = false;
  sparkle.isEnabled = false;
  
  art.begin();
//
    nx.addListener(&art);

//  // Start the webui animation just so we don't have to deal with it
//  // elsewhere right now
    art.startAnimation(&webui.statusAnim, false);

//    art.startAnimation(&rainbow, false);
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
  ota.begin();
//  evil.begin();
}

void loop() {
   msgTube.loop();
   wifiSync.loop();
//  hausFan.loop();
  webui.loop();
//  pinger.loop();

  buttons.loop();
  art.loop();

  lase.loop();
  ota.loop();
//  evil.loop();
}
