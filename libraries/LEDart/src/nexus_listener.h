#pragma once

#include <Arduino.h>

class NexusListener {
public:
    enum NexusValueType {
        Everything = 0,
        Palette,
        SpeedFactor,
        Foreground,
        Background,
        CurrentGeomName,
        CurrentAnimName,
        MaxDuration,
        Reverse,
        MaxBrightness,

        PreparedState,
        
        Last
    };


    virtual void nexusValueUpdate(NexusValueType which, uint32_t source);

    virtual void nexusUserGeometryRequest(char* szName, bool rotated, uint32_t source) { }

    // Can pass NULL as szName to ask for a random selection
    virtual void nexusUserAnimationRequest(char* szName, bool randomize, uint32_t source);

};