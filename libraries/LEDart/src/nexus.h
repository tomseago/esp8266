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


#include "nexus_listener.h"


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

    // uint32_t maxDuration = 128000;
    uint32_t maxDuration = 1280000;
    bool reverse = false;

    // For pringles, 180
    // float maxBrightness = 180; // This has a range of 0 to 255, unlike brightness elsewhere which is 0.0 to 1.0
    float maxBrightness = 180.0; // This has a range of 0 to 255, unlike brightness elsewhere which is 0.0     

    Nexus(bool forceSpecificGeometry=false);

    void randomizeAll(uint32_t source);
    void nextUnitType(uint32_t source);
    void nextPalette(uint32_t source);

    void addAnimation(char *szName);
    uint8_t numAnimations();
    char* animName(uint8_t index);

    void addListener(NexusListener* listener);

    void sendValueUpdate(NexusListener::NexusValueType which, uint32_t source);
    void sendUserAnimationRequest(char* szName, bool randomize, uint32_t source);

    // Makes sure the unit type is valid
    void checkUnitType();

    // Just updates your value, doesn't change the unit type
    void updateUnitTypeVal(uint8_t* val);

    // Adjusted by WiFiSync to make our time base match master
    int32_t localTimeOffset = 0;

    // Set by WiFiSync when not master so that the only way base animations stop
    // is because of a prepared changed
    bool forcedForeverLoop = false;

    // Set by WiFiSync when master to a non-zero value which causes prepared states to
    // be generated. This is milliseconds before the end of the current max duration
    // when a animation is started. 
    uint32_t shouldPrepareRandomStatesFor = 0;

    void prepareRandomStateFor(uint32_t when, char* szAnimName, uint32_t source);
    uint32_t nextPreparedState(char **pszAnimName);
    void clearPreparedState();
    void usePreparedState();
    uint16_t serializePreparedState(uint8_t* into);
    bool deserializePreparedState(uint16_t length, const uint8_t* from);

private:
    std::vector<char *> animNames;

    std::vector<NexusListener*> listeners;

    bool _forceSpecificGeometry;

    struct NexusState {
        uint8_t unitType;
        uint8_t palette;
        float speedFactor;
        RgbColor foreground;
        RgbColor background;
        bool reverse;

        uint32_t time = 0;
        uint8_t nameLen;
    };

    NexusState nextState;
    char* szNextAnim = NULL;

    void randomizeState(NexusState* state);
    void applyState(NexusState* state);

    uint16_t serializeState(NexusState* ns, char* szName, uint8_t* into);
    bool deserializeState(NexusState* ns, char** pszName, uint16_t length, const uint8_t* from);

    void logState(NexusState* state, char *szName);
    void testSerializer();
};
