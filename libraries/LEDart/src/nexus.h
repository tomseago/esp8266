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
    bool reverse = false;
    LAColor foreground = LAColor(255, 0, 0);
    LAColor background = LAColor(0, 0, 255);

    // Brightness has a float range of 0.0 to 255.0 
    // It is initially set by the art piece when the piece is constructed.
    float maxBrightness; 


    // uint32_t maxDuration = 128000;
    uint32_t maxDuration = 1280000;

    int stateQEvents;

    Nexus();

    void randomizeAll(uint32_t source);
    void nextPalette(uint32_t source);

    void addListener(NexusListener* listener);

    char* getCurrentGeomName() { return szCurrentGeom; }
    bool isGeomRotated() { return geomRotated; }
    char* getCurrentAnimName() { return szCurrentAnim; }

    void setGeometry(char* szName, bool rotated, uint32_t source);
    void setAnimation(char* szName, uint32_t source);



    // If something directly modifies one of the values it is their responsibility to call
    // this. This saves us the overhead of setters and getters.
    void sendValueUpdate(NexusListener::NexusValueType which, uint32_t source);

    // UI's uses theses to notify the system that the user has made a request
    // for a specific geometry or animation. Pass NULL for szName to request
    // the next geometry, which may simply be a rotation of the current geometry.
    // If requesting the next geometry the rotated parameter is interpreted the
    // same as the randomize parameter for animation requests - that is, it
    // is used to decide if the natural orderering or a random selection is 
    // used.
    void sendUserGeometryRequest(char* szName, bool rotated, uint32_t source);

    inline void nextGeometry(uint32_t source) {
        sendUserGeometryRequest(NULL, true, source);
    }

    // If the requested animation isn't found, randomize is consulted to determine
    // if the fallback strategy is to just go with the "next" animation or to
    // select a random animation. This means that a caller can pass NULL for szName
    // to get either "next" or "random" passed on the value of randomize.
    void sendUserAnimationRequest(char* szName, bool randomize, uint32_t source);

    inline void nextAnimation(uint32_t source) {
        sendUserAnimationRequest(NULL, true, source);
    }


    // Adjusted by WiFiSync to make our time base match master
    int32_t localTimeOffset = 0;

    // Set by WiFiSync when not master so that the only way base animations stop
    // is because of a prepared changed
    bool forcedForeverLoop = false;



    // Set by WiFiSync when master to a non-zero value which causes prepared states to
    // be generated. This is milliseconds before the end of the current max duration
    // when a animation is started. 
    uint32_t shouldPrepareRandomStatesFor = 0;

    // This is called by the master to create a new prepared state
    // to be used at a specified time in the future.
    void prepareStateFor(uint32_t when, bool randomize, char* szGeomName, bool rotated, char* szAnimName, uint32_t source);

    // This is a query function to figure out if there _is_ a prepared state we should
    // be using. It returns 0 if there isn't or the time after which the prepared state
    // should be put into use
    uint32_t nextPreparedState(char **pszGeomName, bool* pGeomRotated, char **pszAnimName);

    // Applies any prepared state so that it becomes the current state
    void usePreparedState();

    // Clears any stored prepared state, which will also trigger any queued state state
    // waiting in the wings to become the new prepared state. 
    void clearPreparedState();

    // Used to get the data for the prepared state so that it can be sent to other
    // parties
    uint16_t serializeState(bool isCurrent, bool withOverride, uint8_t* into);

    // Called when a new state has been received from the network.
    bool deserializeState(bool isCurrent, uint16_t length, const uint8_t* from);

private:
    std::vector<NexusListener*> listeners;

    struct NexusState {
        uint8_t override; // indicates this should be applied immediately not queued

        uint8_t palette;
        float speedFactor = 1.0;
        bool reverse;
        LAColor foreground;
        LAColor background;
        uint8_t maxBrightness;
        bool geomRotated;

        uint32_t time = 0;
        uint8_t geomNameLen;
        uint8_t animNameLen;
    };

    NexusState nextState;
    char* szNextGeom = NULL;
    char* szNextAnim = NULL;

    // If we get a new prepared state before we used the last one, we have to
    // hold onto it here before we _actually_ deserialize it...
    uint8_t* nextNextStateData = NULL;
    uint16_t nextNextStateLength;

    void randomizeState(NexusState* state);
    void applyState(NexusState* state);
    void gatherState(NexusState* state); // opposite of apply

    uint16_t serializeState(NexusState* ns, char* szGeomName, char* szAnimName, uint8_t* into);
    bool deserializeState(NexusState* ns, char** pszGeomName, char** pszAnimName, uint16_t length, const uint8_t* from);

    void logName(uint16_t len, char* sz);
    void logState(NexusState* state, char *szGeomName, char *szAnimName);
    void testSerializer();
};
