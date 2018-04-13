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
    // Really should be a friend...
    char *szName;
    LEDArtGeometry* pNext = NULL;
    uint8_t baseId;

    //
    bool canRotate;

    LEDArtGeometry(char* szName, bool canRotate = true) : szName(szName), canRotate(canRotate) {}
    virtual uint16_t map(uint16_t x, uint16_t y) = 0;

    virtual uint16_t getWidth(int16_t rowIx=-1) = 0;
    virtual uint16_t getHeight() = 0;
};

class LEDArtSingleGeometry : public LEDArtGeometry {
protected:
    uint16_t pixelCount;

public:
    LEDArtSingleGeometry(char *szName, uint16_t pixelCount, bool canRotate = true) : 
        LEDArtGeometry(szName, canRotate),
        pixelCount(pixelCount)
    {
    }

    virtual uint16_t map(uint16_t x, uint16_t y) { return x; }

    virtual uint16_t getWidth(int16_t rowIx=-1) { return pixelCount; }
    virtual uint16_t getHeight() { return 1; }
};

template <typename T> class LEDArtTopoGeometry : public LEDArtGeometry {
protected:
    NeoTopology<T> topo;

public:
    LEDArtTopoGeometry(char* szName, NeoTopology<T> topo ) :
        LEDArtGeometry(szName),
        topo(topo)
    {        
    }

    virtual uint16_t map(uint16_t x, uint16_t y) { return topo.Map(x,y); }

    virtual uint16_t getWidth(int16_t rowIx=-1) { return topo.getWidth(); }
    virtual uint16_t getHeight() { return topo.getHeight(); }
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

    enum LEDUnitType {
        Unit_Single = 0,
        Unit_Each,
        Unit_Rows,
        Unit_Cols,
        Unit_SpecificRows,
        Unit_SpecificCols,
        Unit_AltRows,
        Unit_AltCols,

        Unit_Last
    };

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
    // NeoTopology<LEDART_TOPO_LAYOUT> topo;
    // LEDArtGeometry* specificGeometry = NULL;
    // LEDArtGeometry* altGeometry = NULL;
    Nexus& nexus;

    LEDArtPiece(Nexus& nexus, uint16_t pixelCount, uint8_t maxBrightness, uint8_t port=0);

    void* registerGeometry(LEDArtGeometry* pGeom);
    void* registerAnimation(LEDArtAnimation* pAnim);

    virtual void begin();
    virtual void loop();

    virtual void startAnimation(LEDArtAnimation* pAnim, bool isLoop=false, uint32_t now=0);
    void stopAnimation(LEDAnimationType type);

    void nextGeometry(bool randomize);
    void nextBaseAnimation(bool randomize, uint32_t now=0);

    //////// Geometries
    uint8_t geomId();

    uint16_t geomHeight(); // Number of rows, the default primary
    uint16_t geomWidth(int16_t rowIx=-1);  // Number of cols, the default secondary

    // Careful, this is in the order secondary, primary
    uint16_t geomMap(uint16_t x, uint16_t y);

    // These are a little easier to keep straight
    uint16_t geomPrimaryCount() { return geomHeight(); }
    uint16_t geomSecondaryCount(int16_t primaryIx=-1) { return geomWidth(primaryIx); }

    void setPrimaryColor(uint16_t primaryIx, RgbColor color);
    void setSecondaryColor(uint16_t secondaryIx, RgbColor color);
    void setSecondaryColorInPrimary(uint16_t primaryIx, uint16_t secondaryIx, RgbColor color);


    // These are not for general use
    void nexusValueUpdate(NexusValueType which, uint32_t source);

    // Can pass NULL as szName to ask for a random selection
    void nexusUserGeometryRequest(char* szName, bool rotated, uint32_t source);

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

    LEDArtGeometry* pCurrentGeom;
    bool geomRotated;
    LEDArtGeometry* pGeomRegistrations;

    uint8_t lastGeomId;

    void animateChannel(LEDAnimationType type, uint32_t now);

    LEDArtGeometry* geomForName(char* szName);
    LEDArtGeometry* findNextGeometry(bool randomize, bool* pRotated);

    LEDArtAnimation* findNextBaseAnimation(bool randomize);
    LEDArtAnimation* baseAnimForName(char* szName);
};
