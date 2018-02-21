#pragma once

#include "LEDArt_config.h"
#include "nexus_listener.h"

#include <NeoPixelBrightnessBus.h>
// #include <NeoPixelAnimator.h>

class LEDArtPiece;
class LEDArtAnimation;
class Nexus;

enum LEDAnimationType {
    LEDAnimationType_BASE = 0,
    LEDAnimationType_OVERLAY = 1,
    LEDAnimationType_STATUS = 2,
};

enum LEDAnimationState
{
    LEDAnimationState_Started,
    LEDAnimationState_Progress,
    LEDAnimationState_Completed
};

struct LEDAnimationParam
{
    float progress;
    LEDAnimationState state;
};


class LEDArtGeometry {
public:
    uint16_t width;
    uint16_t height;

    LEDArtGeometry(uint16_t width, uint16_t height) : width(width), height(height) {}
    virtual uint16_t Map(uint16_t x, uint16_t y) = 0;
};

class LEDArtAnimation {
public:
    char *szName;
    uint16_t loopDuration = 16000; // 8bars at 120bpm
    uint16_t maxDuration = 0;
    bool loops = true;
    bool isEnabled = true;
    bool ignoreSpeedFactor = false;
    LEDAnimationType type = LEDAnimationType_BASE;

    float brightness = 1.0;

    LEDArtAnimation* pNext = NULL;

    LEDArtAnimation(char* szName);
    ~LEDArtAnimation();

    virtual void animate(LEDArtPiece& piece, LEDAnimationParam p) = 0;

    enum LEDPaletteType {
        LEDPalette_RB = 0,
        LEDPalette_RGB,
        LEDPalette_RYB, 
        LEDPalette_WHITE_BLACK, 
        LEDPalette_MARDI_GRAS, 
        LEDPalette_BLUES,
        LEDPalette_PINKS,
        LEDPalette_REDS,
        
        LEDPalette_LAST      
    };

    const static uint8_t paletteSizes[];
    const static RgbColor* paletteColors[];

    RgbColor colorInPalette(LEDPaletteType palette, float progress);
    void clearTo(LEDArtPiece& piece, RgbColor color, uint16_t start, uint16_t end); 
};

// typedef void (*LEDArtAnimation)(LEDArtPiece& piece, void* context, uint16_t *duration, bool *loops, AnimationParam p);


class LEDArtPiece : public NexusListener {
public:
    NeoPixelBrightnessBus<LEDART_COLOR_FEATURE, LEDART_METHOD> strip;
    NeoTopology<LEDART_TOPO_LAYOUT> topo;
    LEDArtGeometry* specificGeometry = NULL;
    Nexus& nexus;

    LEDArtPiece(Nexus& nexus, uint16_t pixelCount, uint8_t maxBrightness, uint16_t width=1, uint16_t height=1, uint8_t port=0);

    void* registerAnimation(LEDArtAnimation* pAnim);

    virtual void begin();
    virtual void loop();

    virtual void startAnimation(LEDArtAnimation* pAnim, bool isLoop=false, uint32_t now=0);
    void stopAnimation(LEDAnimationType type);

    void nextBaseAnimation(bool randomize, uint32_t now=0);


    // These are not for general use
    void nexusValueUpdate(NexusValueType which, uint32_t source);

    // Can pass NULL as szName to ask for a random selection
    void nexusUserAnimationRequest(char* szName, bool randomize, uint32_t source);

protected:
    // typedef struct RegistrationNode {
    //     char* name;
    //     LEDArtAnimation animation;
    //     void *context;
    //     uint16_t duration;
    //     bool loops;
    //     bool isEnabled;

    //     struct RegistrationNode *next;
    // } RegistrationNode;

    //LEDArtAnimation* pRegistrations[3];

    typedef struct LEDAnimationChannel {
        LEDArtAnimation* pRegistrations;

        LEDArtAnimation* pRunning;
        uint32_t channelStartedAt;
        uint32_t loopStartedAt;
    } LEDAnimationChannel;

    LEDAnimationChannel channels[3];

    // LEDArtAnimation* pRunningAnims[3];

    // uint32_t startedAt[3];
    // uint32_t endsAt[3];

    void animateChannel(LEDAnimationType type, uint32_t now);

    LEDArtAnimation* findNextBaseAnimation(bool randomize);
    LEDArtAnimation* baseAnimForName(char* szName);
};
