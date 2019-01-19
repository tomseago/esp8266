#pragma once

#include <NeoPixelBrightnessBus.h>

class PhotoLights {
public:
    PhotoLights();

    NeoPixelBrightnessBus<NeoGrbwFeature,NeoEsp32I2s0800KbpsMethod> bus;

    void begin();
    void loop();

    void setMode(uint8_t mode);

private:
    uint8_t mode;

};