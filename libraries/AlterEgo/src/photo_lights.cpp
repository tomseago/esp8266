// #include "photo_lights.h"
// #include "tm_common.h"

// // We have two colorspaces remember...

// #define RED RgbwColor(255,0,0,0)
// #define GREEN RgbwColor(0,255,0,0)
// #define BLUE RgbwColor(0,0,255,0)
// // const LAColor red(255,0,0);
// // const LAColor yellow(255,255,0);
// // const LAColor green(0,255,0);
// // const LAColor cyan(0,255,255);
// // const LAColor blue(0,0,255);
// // const LAColor purple(255,0,255);

// // const LAColor white(255,255,255);
// // const LAColor black(0,0,0);



// PhotoLights::PhotoLights<T_METHOD>(uint8_t pin) :
//     bus(300,pin)
// {

// }

// void
// PhotoLights::begin()
// {
//     bus.Begin();
//     bus.SetBrightness(32);
//     bus.ClearTo(BLUE);
//     bus.Show();
// }

// void
// PhotoLights::loop()
// {
//     // TODO: Switch based on mode
//     uint32_t now = millis();

//     uint32_t sinceLast = now - this->lastUpdateAt;

//     if (this->dirty || sinceLast > 1000) {
//         updatePixels();
//         this->dirty = false;
//         this->lastUpdateAt = now;
//     }
// }

// void
// PhotoLights::setMode(uint8_t mode)
// {
//     this->mode = mode;
// }

// void
// PhotoLights::setHue(float hue)
// {
//     if (hue < 0.0) {
//         hue = 0.0;
//     }
//     if (hue > 1.0) {
//         hue = 1.0;
//     }
//     this->hue = hue;
//     this->dirty = true;
// }

// void
// PhotoLights::setSat(float sat)
// {
//     if (sat < 0.0) {
//         sat = 0.0;
//     }
//     if (sat > 1.0) {
//         sat = 1.0;
//     }
//     this->sat = sat;
//     this->dirty = true;
// }

// void
// PhotoLights::setBrightness(float bright)
// {
//     if (bright < 0.0) {
//         bright = 0.0;
//     }
//     if (bright > 1.0) {
//         bright = 1.0;
//     }
//     this->bright = bright;
//     this->dirty = true;
// }

// void
// PhotoLights::recalcColor()
// {
// }

// void
// PhotoLights::updatePixels()
// {
//     if (hue == 1.0) {
//         bus.ClearTo(RED);
//     } else {
//         bus.ClearTo(BLUE);
//     }

//     if (flipFlop) {
//         bus.SetPixelColor(1, GREEN);
//     } else {
//         bus.SetPixelColor(0, GREEN);
//     }
//     flipFlop = !flipFlop;

//     bus.Show();
// }
