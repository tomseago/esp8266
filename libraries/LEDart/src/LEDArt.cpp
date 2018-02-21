#include "LEDArt.h"
#include "rand.h"
#include "nexus.h"
#include "log.h"

//////

const uint8_t LEDArtAnimation::paletteSizes[] = {
    2,  // RB
    3,  // RGB
    12, // RYB
    3,  // WHITE_BLACK
    5,  // MARDI_GRAS
    5,  // BLUES
    5,  // PINKS
    4,  // REDS
};

const RgbColor black(0,0,0);

const RgbColor Palette_RB[] = {
    RgbColor(255, 0, 0),
    RgbColor(0, 0, 255),
};

const RgbColor Palette_RGB[] = {
    RgbColor(255, 0, 0),
    RgbColor(0, 255, 0),
    RgbColor(0, 0, 255),
};

const RgbColor Palette_RYB[] = {
    RgbColor(255, 0, 0),
    RgbColor(255, 64, 0),
    RgbColor(255, 128, 0),
    RgbColor(255, 191, 0),
    RgbColor(255, 255, 0),
    RgbColor(128, 212, 25),
    RgbColor(0, 168, 51),
    RgbColor(21, 132, 102),
    RgbColor(42, 95, 153),
    RgbColor(85, 48, 140),
    RgbColor(128, 0, 128),
    RgbColor(191, 0, 64),
};

const RgbColor Palette_WHITE_BLACK[] = {
    RgbColor(0, 0, 0),
    RgbColor(96, 96, 96),
    RgbColor(255, 255, 255),
};

const RgbColor Palette_JORDAN[] = {
    RgbColor(0, 0, 0),
    RgbColor(96, 96, 96),
    RgbColor(255, 255, 255),
};

const RgbColor Palette_MARDI_GRAS[] = {
    RgbColor(176, 126, 9),
    RgbColor(176, 126, 9),
    RgbColor(4, 87, 22),
    RgbColor(45, 6, 56),
    RgbColor(45, 6, 56),
};

const RgbColor Palette_BLUES[] = {
    RgbColor(32, 74, 255),
    RgbColor(0, 23, 123),
    RgbColor(3, 19, 21),
    RgbColor(21, 18, 33),
    RgbColor(1, 1, 5),
};


const RgbColor Palette_PINKS[] = {
    RgbColor(255, 59, 51),
    RgbColor(191, 43, 87),
    RgbColor(127, 48, 27),
    RgbColor(64, 15, 13),
    RgbColor(229, 92, 87),
};

const RgbColor Palette_REDS[] = {
    RgbColor(255, 0, 0),
    RgbColor(225, 32, 5),
    RgbColor(35, 0, 0),
    RgbColor(0, 0, 0),
};

const RgbColor* LEDArtAnimation::paletteColors[] = {
    Palette_RB,
    Palette_RGB,
    Palette_RYB,
    Palette_WHITE_BLACK,
    Palette_MARDI_GRAS,
    Palette_BLUES,
    Palette_PINKS,
    Palette_REDS,
};

// const uint8_t LEDArtAnimation::rybRainbowColorsCount = 3;

// const RgbColor LEDArtAnimation::rybRainbowColors[] = {
//     RgbColor(0, 0, 255),
//     RgbColor(255, 0, 0),
//     RgbColor(0, 255, 0),
// };
LEDArtAnimation::LEDArtAnimation(char* szName) {
    this->szName = strdup(szName);
}

LEDArtAnimation::~LEDArtAnimation() {
    free(szName);
}

RgbColor
LEDArtAnimation::colorInPalette(LEDPaletteType palette, float progress) {
    if (progress > 1.0) {
        double iPart;
        progress = modf(progress, &iPart);
    }

    if (progress == 1.0 ) {
        // Serial.print("progress="); Serial.print(progress, 3);
        // Serial.print(" Resetting to 0----\n");
        progress = 0.0;
    }

    float pos = progress * (float)(paletteSizes[palette]);
    uint8_t lowIx = floor(pos);
    uint8_t highIx = lowIx + 1;
    if (highIx >= paletteSizes[palette]) highIx = 0;

    float intervalDistance = pos - (float)lowIx;

    // Serial.print("progress="); Serial.print(progress, 3);
    // Serial.print(" pos="); Serial.print(pos,3);
    // Serial.print(" idist="); Serial.print(intervalDistance,3);
    // Serial.printf(" lowIx=%d, highIx=%d", lowIx, highIx);

    RgbColor color = RgbColor::LinearBlend(paletteColors[palette][lowIx], paletteColors[palette][highIx], intervalDistance);

    // Serial.printf(" ==> (%3d, %3d, %3d)\n", color.R, color.G, color.B);
    return color;
}

void
LEDArtAnimation::clearTo(LEDArtPiece& piece, RgbColor color, uint16_t start, uint16_t end) {
    for(uint16_t ix=start; ix<end; ix++) {
        piece.strip.SetPixelColor(ix, color);
    }
}

/////

// LEDArtPiece *pSingleton = NULL;

// void AnimateBaseGlue(AnimationParam param) {
//     // Presume the singleton is set
//     pSingleton->animateChannel(param, AnimationType_BASE);
// }


// void AnimateOverlayGlue(AnimationParam param) {
//     // Presume the singleton is set
//     pSingleton->animateChannel(param, AnimationType_OVERLAY);
// }


// void AnimateStatusGlue(AnimationParam param) {
//     // Presume the singleton is set
//     pSingleton->animateChannel(param, AnimationType_STATUS);
// }

// // Can be set to our modified NeoPixelAnimator so that it uses the localTimeOffset
// unsigned long AnimateTimeFunction() {
//     return millis() + pSingleton->nexus.localTimeOffset;
// }


LEDArtPiece::LEDArtPiece(Nexus& nx, uint16_t pixelCount, uint8_t maxBrightness, uint16_t width, uint16_t height, uint8_t port) : 
    nexus(nx),
    strip(pixelCount, port),
    topo(width, height)
    // animator(3)
{
    // For offsets
    // animator.setTimeFunction(&AnimateTimeFunction);

    nx.maxBrightness = (float)maxBrightness;
    // pSingleton = this;
}

void* 
LEDArtPiece::registerAnimation(LEDArtAnimation* pAnim) 
{
    if (!pAnim) return NULL;

    LEDAnimationChannel* pChannel = channels + pAnim->type;

    pAnim->pNext = pChannel->pRegistrations;
    pChannel->pRegistrations = pAnim;

    if (pAnim->type == LEDAnimationType_BASE)
    {
        nexus.addAnimation(pAnim->szName);
    }

    return pAnim;
}

void 
LEDArtPiece::begin() 
{
    // Be nice and zero it all out
    strip.Begin();
    strip.Show();

    // Start the first base animation
    nextBaseAnimation(true);
    // LEDArtAnimation* pAnim = findNextBaseAnimation(true);
    // startAnimation(pAnim, false);
}


void 
LEDArtPiece::loop() 
{
    // animator.UpdateAnimations();
    uint32_t now = millis() + nexus.localTimeOffset;

    for(uint8_t i=0; i<3; i++) 
    {
        animateChannel((LEDAnimationType)i, now);
    }

    strip.Show();
}


void
LEDArtPiece::startAnimation(LEDArtAnimation* pAnim, bool isLoop, uint32_t now) 
{

    if (!pAnim) {
        return;
    }

    // strip.SetBrightness(maxBrightness * pAnim->brightness);
    uint8_t b = nexus.maxBrightness * pAnim->brightness;
    // Serial.print("Set Brightness to "); Serial.print(b); Serial.print("\n");
    strip.SetBrightness(b);

    if (!now) 
    {
        now = millis() + nexus.localTimeOffset;
    }

    LEDAnimationChannel* pChannel = channels + pAnim->type;
    pChannel->pRunning = pAnim;
    pChannel->loopStartedAt = now;    
    if (!isLoop)
    {        
        pChannel->channelStartedAt = now;
        Log.printf("PIECE: Start animation %s\n", pAnim->szName);
    } else {
        Log.printf("PIECE: Loop animation %s\n", pAnim->szName);
    }
    nexus.currentAnim = pAnim->szName;

    // Dispatch time 0
    LEDAnimationParam param;
    param.state = LEDAnimationState_Started;
    param.progress = nexus.reverse ? 1.0f : 0.0f;
    pAnim->animate(*this, param);

    // If we are a channel start, we might need to prepare for the end of times
    if (!isLoop && pAnim->type == LEDAnimationType_BASE && nexus.shouldPrepareRandomStatesFor)
    {
        uint32_t minDuration = pAnim->maxDuration;

        if (!minDuration)
        {
            // Nothing specified by the anim, so use what's in the nexus
            minDuration = nexus.maxDuration;
        }
        else
        {
            // Is the nexus one existant and lower?
            if (nexus.maxDuration && nexus.maxDuration < minDuration)
            {
                minDuration = nexus.maxDuration;
            }
        }

        // Oh hey, nothing specified a duration, so ignore this
        if (!minDuration)
        {
            Log.printf("PIECE: shouldPrepareRandomStatesFor set but not maxDuration anywhere. Ignoring it.\n");
        }
        else
        {
            uint32_t nextStateTime = now + minDuration - nexus.shouldPrepareRandomStatesFor;
            LEDArtAnimation* pNext = findNextBaseAnimation(true);
            nexus.prepareRandomStateFor(nextStateTime, pNext->szName, (uint32_t)this);
        }
    }
}

void 
LEDArtPiece::stopAnimation(LEDAnimationType type) 
{
    LEDAnimationChannel* pChannel = channels + type;

    pChannel->pRunning = NULL;
}

void 
LEDArtPiece::nextBaseAnimation(bool randomize, uint32_t now) 
{
    LEDArtAnimation* pAnim = findNextBaseAnimation(randomize);

    if (randomize) {
        nexus.randomizeAll((uint32_t)this);
    }    

    startAnimation(pAnim, false, now);
}


void
LEDArtPiece::animateChannel(LEDAnimationType type, uint32_t now) 
{
    LEDAnimationChannel* pChannel = channels + type;
    LEDArtAnimation* pAnim = pChannel->pRunning;
    if (!pAnim)
    {
        // Nothing on the channel is running
        return;
    }

    LEDAnimationParam param;
    param.state = LEDAnimationState_Progress;

    // Have we exceeded the single loop duration?
    uint32_t channelElapsed = now - pChannel->channelStartedAt;
    uint32_t loopElapsed = now - pChannel->loopStartedAt;

    uint32_t loopAdjustedDuration = pAnim->loopDuration;
    if (!pAnim->ignoreSpeedFactor) 
    {
        loopAdjustedDuration = (float)loopAdjustedDuration * nexus.speedFactor;
    }

    if (loopElapsed > loopAdjustedDuration)
    {
        // This loop has ended, so definitely need to dispatch this concept to the animation
        param.progress = 1.0f;
        param.state = LEDAnimationState_Completed;
    }
    else
    {
        // Normal loop interval
        param.progress = (float)loopElapsed / (float)loopAdjustedDuration;        
    }

    // Structs get copied right? I sure hope so...
    if (nexus.reverse) {
        param.progress = 1.0 - param.progress;
    }

    // Dispatch for the update
    pAnim->animate(*this, param);


    // Now let's see if we've exceeded a max duration
    if (type != LEDAnimationType_STATUS && !nexus.forcedForeverLoop &&
        ( (pAnim->maxDuration && channelElapsed >= pAnim->maxDuration) ||
          (nexus.maxDuration && channelElapsed >= nexus.maxDuration ) ) )
    {
        // This base or overlay is OVER
        if (type == LEDAnimationType_BASE)
        {
            nextBaseAnimation(true, now);
        }
        else
        {
            stopAnimation(type);
        }
        return;
    }

    // Do we have a prepared thing to do next?
    char* szPreparedName = NULL;
    uint32_t preparedReadyAt = nexus.nextPreparedState(&szPreparedName);
    if (preparedReadyAt && preparedReadyAt < now)
    {
        // Oh hey! try to do the next animation
        if (szPreparedName)
        {
            LEDArtAnimation* pNext = baseAnimForName(szPreparedName);
            if (!pNext)
            {
                Log.printf("PIECE: Did not find requested animation '%s' from prepared state\n", szPreparedName);
                nexus.clearPreparedState();
            }
            else
            {
                Log.printf("PIECE: Starting prepared animation '%s'\n", szPreparedName);
                nexus.usePreparedState();
                nexus.clearPreparedState();
                startAnimation(pNext, false, now);
                return;
            }
        }
        else
        {
            Log.printf("PIECE: Prepared state had no anim name, ignoring\n");
            nexus.clearPreparedState();
        }
    }

    // Was that completed though?
    // If it has finished, it might be time to start something new though
    if (param.state == LEDAnimationState_Completed) {
        if (pAnim->loops || nexus.forcedForeverLoop) {
            // Restart it - woo hoo!
            startAnimation(pAnim, true, now);
        } else if (type == LEDAnimationType_BASE) {
            // Only base will automatically move to a new animation

            // Need to find a next animation for this channel
            nextBaseAnimation(true, now);
        }
        return;
    }
}

LEDArtAnimation*
LEDArtPiece::findNextBaseAnimation(bool randomize)
{
    LEDAnimationChannel* pChannel = channels + LEDAnimationType_BASE;
    LEDArtAnimation* pCur = pChannel->pRunning;

    if (!pCur) {
        // Nothing running, use the first registration
        return pChannel->pRegistrations;
    }

    LEDArtAnimation* pNext = pCur->pNext;

    // First half of the search, from this point to the end of the list
    while(pNext) {
        if (pNext->isEnabled && 
            (!randomize ||
                (rand(100) < 60) )) {
            return pNext;
        }

        pNext = pNext->pNext;
    }

    // Second half we restart at the very head of the list and go until
    // we get to the current one
    pNext = pChannel->pRegistrations;
    while(pNext != pCur) {
        if (pNext->isEnabled && 
            (!randomize ||
                (rand(100) < 60) )) {
            return pNext;
        }

        pNext = pNext->pNext;
    }

    // Oops! We didn't find a "next", but because we want to always have 
    // something, just return the current again...
    return pCur;
}

void
LEDArtPiece::nexusValueUpdate(NexusValueType which, uint32_t source) 
{

}

// Can pass NULL as szName to ask for a random selection
void
LEDArtPiece::nexusUserAnimationRequest(char* szName, bool randomize, uint32_t source)
{
    LEDArtAnimation* pRequested = baseAnimForName(szName);

    // Just do something else
    if (!pRequested)
    {
        nextBaseAnimation(randomize);
        return;
    }

    startAnimation(pRequested);
}

LEDArtAnimation*
LEDArtPiece::baseAnimForName(char* szName)
{
    if (!szName)
    {
        return NULL;
    }

    // Try to find something specific to do
    LEDAnimationChannel* pChannel = channels + LEDAnimationType_BASE;
    LEDArtAnimation* pCursor = pChannel->pRegistrations;

    while(pCursor) {
        if (strcmp(pCursor->szName, szName) == 0)
        {
            // Found it! Start it!
            return pCursor;
        }
        pCursor = pCursor->pNext;
    }

    // Else, did not find. Tell someone??
    return NULL;
}
