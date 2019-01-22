#pragma once

#include <NeoPixelBrightnessBus.h>

class Circles {
public:
    Circles();

    NeoPixelBrightnessBus<NeoGrbFeature,Neo800KbpsMethod> bus;

    void begin();
    void loop();

    void setText(char* txt);

    uint32_t lastFrameAt;
};