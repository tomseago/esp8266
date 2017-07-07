#define FASTLED_ESP8266_D1_PIN_ORDER
//#define FASTLED_ALL_PINS_HARDWARE_SPI

#include <SPI.h>
//#include <bitswap.h>
//#include <chipsets.h>
//#include <color.h>
//#include <colorpalettes.h>
//#include <colorutils.h>
//#include <controller.h>
//#include <cpp_compat.h>
//#include <dmx.h>
#include <FastLED.h>
//#include <fastled_config.h>
//#include <fastled_delay.h>
//#include <fastled_progmem.h>
//#include <fastpin.h>
//#include <fastspi.h>
//#include <fastspi_bitbang.h>
//#include <fastspi_dma.h>
//#include <fastspi_nop.h>
//#include <fastspi_ref.h>
//#include <fastspi_types.h>
//#include <hsv2rgb.h>
//#include <led_sysdefs.h>
//#include <lib8tion.h>
//#include <noise.h>
//#include <pixelset.h>
//#include <pixeltypes.h>
//#include <platforms.h>
//#include <power_mgt.h>

#define NUM_LEDS 149
CRGB leds[NUM_LEDS];
void setup() { FastLED.addLeds<NEOPIXEL, D3>(leds, NUM_LEDS); }

void setColor(CRGB color) {
  for(int i=0; i<NUM_LEDS; i++) {
    leds[i] = color;
  }
  FastLED.show();  
}
void loop() {
//  leds[0] = CRGB::White; FastLED.show(); delay(30);
//  leds[0] = CRGB::Red; FastLED.show(); delay(30);
  setColor(CRGB::Green);
  delay(100);
  setColor(CRGB::Red);
  delay(100);
}


