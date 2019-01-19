#include "photo_lights.h"
#include "tm_common.h"

// We have two colorspaces remember...

#define RED RgbwColor(255,0,0,0)
#define GREEN RgbwColor(0,255,0,0)
#define BLUE RgbwColor(0,0,255,0)
// const LAColor red(255,0,0);
// const LAColor yellow(255,255,0);
// const LAColor green(0,255,0);
// const LAColor cyan(0,255,255);
// const LAColor blue(0,0,255);
// const LAColor purple(255,0,255);

// const LAColor white(255,255,255);
// const LAColor black(0,0,0);


// Num pixels is 24 per digit and 9 digits so 216
#define PHOTO_LIGHTS_PIN 2

PhotoLights::PhotoLights() :
    bus(5,PHOTO_LIGHTS_PIN)
{

}

void
PhotoLights::begin()
{
    bus.Begin();
    bus.SetBrightness(32);
    bus.ClearTo(BLUE);
    bus.Show();
}

void
PhotoLights::loop()
{
    // TODO: Switch based on mode
}

void
PhotoLights::setMode(uint8_t mode)
{
    this->mode = mode;
}