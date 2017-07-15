#pragma once

#include "LEDArt.h"
#include <vector>

typedef enum NexusChangeType {
    NexusChange_Everything = 0,
    NexusChange_Unit,
    NexusChange_Palette,
    NexusChange_Speed,
    NexusChange_Foreground,
    NexusChange_Background,
    NexusChange_CurrentAnimation
} NexusChangeType;

typedef std::function<void(NexusChangeType)> NexusChangeListener;

/**
 The Nexus hold all controls that animation needs and services as a meeting
 point between things like UIs and animations that need to know what to do.
 */
class Nexus {
public:
    uint8_t unitType = 0;
    LEDArtAnimation::LEDPaletteType palette = (LEDArtAnimation::LEDPaletteType)0;
    float speedFactor = 1.0;
    RgbColor foreground = RgbColor(255, 0, 0);
    char* currentAnim = "";

    Nexus(int i);

    void randomizeAll();
    void nextUnitType();
    void nextPalette();

    void addAnimation(char *szName);
    uint8_t numAnimations();
    char* animName(uint8_t index);

    void registerListener(NexusChangeListener* listener);

private:
    std::vector<char *> animNames;
};
