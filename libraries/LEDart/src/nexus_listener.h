#pragma once

#include <Arduino.h>

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

        PreparedState,
        
        Last
    };


    virtual void nexusValueUpdate(NexusValueType which, uint32_t source);

    // Can pass NULL as szName to ask for a random selection
    virtual void nexusUserAnimationRequest(char* szName, bool randomize, uint32_t source);

};