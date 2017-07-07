
#include <NeoPixelBus.h>

const uint16_t PixelCount = 126; // this example assumes 4 pixels, making it smaller will cause a failure

#define colorSaturation 64

// three element pixels, in different order and speeds
//NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip(PixelCount, PixelPin);
//NeoPixelBus<NeoRgbFeature, Neo400KbpsMethod> strip(PixelCount, PixelPin);

// For Esp8266, the Pin is omitted and it uses GPIO3 due to DMA hardware use.  
// There are other Esp8266 alternative methods that provide more pin options, but also have
// other side effects.
NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip(PixelCount);

RgbColor red(colorSaturation, 0, 0);
RgbColor green(0, colorSaturation, 0);
RgbColor blue(0, 0, colorSaturation);
RgbColor white(colorSaturation);
RgbColor black(0);

HslColor hslRed(red);
HslColor hslGreen(green);
HslColor hslBlue(blue);
HslColor hslWhite(white);
HslColor hslBlack(black);

void setup() {
    // this resets all the neopixels to an off state
    strip.Begin();
    strip.Show();

}

void loop() {
  strip.ClearTo(green);
  strip.Show();
  delay(3000);

  strip.ClearTo(blue);
  strip.Show();
  delay(3000);
}
