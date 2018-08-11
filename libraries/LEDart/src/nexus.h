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
protected:
    char* szCurrentGeom = NULL;
    bool geomRotated = false;
    char* szCurrentAnim = NULL;

public:
    LEDArtAnimation::LEDPaletteType palette = (LEDArtAnimation::LEDPaletteType)0;
    float speedFactor = 1.0;
    RgbColor foreground = RgbColor(255, 0, 0);
    RgbColor background = RgbColor(0, 0, 255);

    // uint32_t maxDuration = 128000;
    uint32_t maxDuration = 1280000;
    bool reverse = false;

    // Brightness has a float range of 0.0 to 255.0 
    // It is initially set by the art piece when the piece is constructed.
    float maxBrightness; 

    Nexus();

    void randomizeAll(uint32_t source);
    void nextPalette(uint32_t source);

    // void addGeometry(char *szName);
    // uint8_t numGeometries();
    // char* geomName(uint8_t index);

    // void addAnimation(char *szName);
    // uint8_t numAnimations();
    // char* animName(uint8_t index);

    char* getCurrentGeomName() { return szCurrentGeom; }
    bool isGeomRotated() { return geomRotated; }
    char* getCurrentAnimName() { return szCurrentAnim; }

    void addListener(NexusListener* listener);

    void setGeometry(char* szName, bool rotated, uint32_t source);
    void setAnimation(char* szName, uint32_t source);

    //
    void sendValueUpdate(NexusListener::NexusValueType which, uint32_t source);
    void sendUserGeometryRequest(char* szName, bool rotated, uint32_t source);
    void sendUserAnimationRequest(char* szName, bool randomize, uint32_t source);

    // Adjusted by WiFiSync to make our time base match master
    int32_t localTimeOffset = 0;

    // Set by WiFiSync when not master so that the only way base animations stop
    // is because of a prepared changed
    bool forcedForeverLoop = false;

    // Set by WiFiSync when master to a non-zero value which causes prepared states to
    // be generated. This is milliseconds before the end of the current max duration
    // when a animation is started. 
    uint32_t shouldPrepareRandomStatesFor = 0;

    void prepareRandomStateFor(uint32_t when, char* szGeomName, bool rotated, char* szAnimName, uint32_t source);
    uint32_t nextPreparedState(char **pszGeomName, bool* pGeomRotated, char **pszAnimName);
    void clearPreparedState();
    void usePreparedState();
    uint16_t serializePreparedState(uint8_t* into);
    bool deserializePreparedState(uint16_t length, const uint8_t* from);

private:
    // std::vector<char *> geomNames;
    // std::vector<char *> animNames;

    std::vector<NexusListener*> listeners;

    struct NexusState {
        uint8_t palette;
        float speedFactor = 1.0;
        RgbColor foreground;
        RgbColor background;
        bool reverse;
        bool geomRotated;

        uint32_t time = 0;
        uint8_t geomNameLen;
        uint8_t animNameLen;
    };

    NexusState nextState;
    char* szNextGeom = NULL;
    char* szNextAnim = NULL;

    void randomizeState(NexusState* state);
    void applyState(NexusState* state);

    uint16_t serializeState(NexusState* ns, char* szGeomName, char* szAnimName, uint8_t* into);
    bool deserializeState(NexusState* ns, char** pszGeomName, char** pszAnimName, uint16_t length, const uint8_t* from);

    void logName(uint16_t len, char* sz);
    void logState(NexusState* state, char *szGeomName, char *szAnimName);
    void testSerializer();
};
