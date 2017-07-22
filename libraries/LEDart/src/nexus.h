#pragma once

#include "LEDArt.h"
#include <vector>

// typedef enum NexusChangeType {
//     NexusChange_Everything = 0,
//     NexusChange_Unit,
//     NexusChange_Palette,
//     NexusChange_Speed,
//     NexusChange_Foreground,
//     NexusChange_Background,
//     NexusChange_CurrentAnimation
// } NexusChangeType;

// typedef std::function<void(NexusChangeType)> NexusChangeListener;


class NexusListener {
public:
    enum NexusValueType {
        Everything = 0,
        UnitType,
        Palette,
        SpeedFactor,
        Foreground,
        Background,
        CurrentAnimName,
        MaxDuration,
        Reverse,
        MaxBrightness,

        Last
    };


    virtual void nexusValueUpdate(NexusValueType which, uint32_t source);

    // Can pass NULL as szName to ask for a random selection
    virtual void nexusUserAnimationRequest(char* szName, uint32_t source);

};


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
    RgbColor background = RgbColor(0, 0, 255);
    char* currentAnim = "";

    uint32_t maxDuration = 128000;
    bool reverse = false;

    // For pringles, 180
    // float maxBrightness = 180; // This has a range of 0 to 255, unlike brightness elsewhere which is 0.0 to 1.0
    float maxBrightness = 180.0; // This has a range of 0 to 255, unlike brightness elsewhere which is 0.0     


    Nexus(int i);

    void randomizeAll(uint32_t source);
    void nextUnitType(uint32_t source);
    void nextPalette(uint32_t source);

    void addAnimation(char *szName);
    uint8_t numAnimations();
    char* animName(uint8_t index);

    void addListener(NexusListener* listener);

    void sendValueUpdate(NexusListener::NexusValueType which, uint32_t source);
    void sendUserAnimationRequest(char* szName, uint32_t source);

private:
    std::vector<char *> animNames;

    std::vector<NexusListener*> listeners;
};
