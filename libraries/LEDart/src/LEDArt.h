#pragma once

#include "LEDArt_config.h"

#include <NeoPixelBrightnessBus.h>
#include <NeoPixelAnimator.h>

class LEDArtPiece;
class LEDArtAnimation;

enum AnimationType {
    AnimationType_BASE = 0,
    AnimationType_OVERLAY = 1,
    AnimationType_STATUS = 2,
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
    uint16_t loopDuration = 10000;
    uint16_t maxDuration = 0;
    bool loops = true;
    bool isEnabled = true;
    AnimationType type = AnimationType_BASE;

    float brightness = 1.0;

    LEDArtAnimation* pNext = NULL;

    LEDArtAnimation(char* szName);
    ~LEDArtAnimation();

    virtual void animate(LEDArtPiece& piece, AnimationParam p) = 0;

    enum LEDPaletteType {
        LEDPalette_RB = 0,
        LEDPalette_RGB,
        LEDPalette_RYB, 
        LEDPalette_LAST      
    };

    const static uint8_t paletteSizes[];
    const static RgbColor* paletteColors[];

    RgbColor colorInPalette(LEDPaletteType palette, float progress);
};

// typedef void (*LEDArtAnimation)(LEDArtPiece& piece, void* context, uint16_t *duration, bool *loops, AnimationParam p);

class LEDArtPiece {
public:
    NeoPixelBrightnessBus<LEDART_COLOR_FEATURE, LEDART_METHOD> strip;
    NeoTopology<LEDART_TOPO_LAYOUT> topo;
    LEDArtGeometry* specificGeometry = NULL;

    //// TEMP
    uint8_t unitType = 0;
    LEDArtAnimation::LEDPaletteType palette = (LEDArtAnimation::LEDPaletteType)0;
    ////



    LEDArtPiece(uint16_t pixelCount, uint16_t width=1, uint16_t height=1, uint8_t port=0);

    void* registerAnimation(LEDArtAnimation* pAnim);

    virtual void begin();
    virtual void loop();

    virtual void startAnimation(LEDArtAnimation* pAnim);
    void stopAnimation(AnimationType type);

    void nextAnimation();


    // These are not for general use
    void animateChannel(AnimationParam param, AnimationType type);

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

    LEDArtAnimation* pRegistrations[3];
    LEDArtAnimation* pRunningAnims[3];

    uint32_t startedAt[3];

    uint32_t maxDuration = 30000;
    float maxBrightness = 180; // This has a range of 0 to 255, unlike brightness elsewhere which is 0.0 to 1.0

    NeoPixelAnimator animator;

    LEDArtAnimation* findNextBaseAnimation();
};

