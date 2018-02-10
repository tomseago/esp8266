#include "LEDArt.h"
#include "rand.h"
#include "nexus.h"

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

LEDArtPiece *pSingleton = NULL;

void AnimateBaseGlue(AnimationParam param) {
    // Presume the singleton is set
    pSingleton->animateChannel(param, AnimationType_BASE);
}


void AnimateOverlayGlue(AnimationParam param) {
    // Presume the singleton is set
    pSingleton->animateChannel(param, AnimationType_OVERLAY);
}


void AnimateStatusGlue(AnimationParam param) {
    // Presume the singleton is set
    pSingleton->animateChannel(param, AnimationType_STATUS);
}


LEDArtPiece::LEDArtPiece(Nexus& nx, uint16_t pixelCount, uint8_t maxBrightness, uint16_t width, uint16_t height, uint8_t port) : 
    nexus(nx),
    strip(pixelCount, port),
    topo(width, height),
    animator(3)
{
    nx.maxBrightness = (float)maxBrightness;
    pSingleton = this;
}


void 
LEDArtPiece::begin() {
    // Be nice and zero it all out
    strip.Begin();
    strip.Show();

    // Start the first base animation
    LEDArtAnimation* pAnim = findNextBaseAnimation(true);
    startAnimation(pAnim, false);
}


void 
LEDArtPiece::loop() {
    animator.UpdateAnimations();
    strip.Show();
}


void
LEDArtPiece::startAnimation(LEDArtAnimation* pAnim, bool isLoop) {

    if (!pAnim) {
        return;
    }

    // strip.SetBrightness(maxBrightness * pAnim->brightness);
    uint8_t b = nexus.maxBrightness * pAnim->brightness;
    Serial.print("Set Brightness to "); Serial.print(b); Serial.print("\n");
    strip.SetBrightness(b);

    AnimUpdateCallback func = &AnimateBaseGlue;
    switch(pAnim->type) {
    case AnimationType_OVERLAY:
        func = &AnimateOverlayGlue;
        break;

    case AnimationType_STATUS:
        func = &AnimateStatusGlue;
        break;
    }

    pRunningAnims[pAnim->type] = pAnim;

    uint16_t duration = (pAnim->ignoreSpeedFactor) ? pAnim->loopDuration : (float)pAnim->loopDuration * nexus.speedFactor;
    animator.StartAnimation(pAnim->type, duration, func);

    nexus.currentAnim = pAnim->szName;

    if (!isLoop) {
        startedAt[pAnim->type] = millis();
    }
}

void 
LEDArtPiece::stopAnimation(AnimationType type) {
    animator.StopAnimation(type);
}

void 
LEDArtPiece::nextAnimation(bool randomize) {
    LEDArtAnimation* pAnim = findNextBaseAnimation(randomize);

    if (randomize) {
        nexus.randomizeAll((uint32_t)this);
    }

    startAnimation(pAnim, false);   
}

void* 
LEDArtPiece::registerAnimation(LEDArtAnimation* pAnim) {

    if (!pAnim) return NULL;

    pAnim->pNext = pRegistrations[pAnim->type];
    pRegistrations[pAnim->type] = pAnim;

    nexus.addAnimation(pAnim->szName);

    return pAnim;
}

void
LEDArtPiece::animateChannel(AnimationParam param, AnimationType type) {
    LEDArtAnimation *pCur = pRunningAnims[type];
    if (!pCur) {
        // How did we get here?
        animator.StopAnimation(type);
        return;
    }

    // Structs get copied right? I sure hope so...
    if (nexus.reverse) {
        param.progress = 1.0 - param.progress;
    }

    // We believe there is something running, so let it do it's thing
    pCur->animate(*this, param);

    // Has it hit max time?
    // All animations are bound by their declared maxDuration.
    // Base and Overlay are also bound by the system level 
    // maxDuration (Status ones are not).
    uint32_t elapsed = millis() - startedAt[type];
    if ((pCur->maxDuration > 0 && elapsed >= pCur->maxDuration) ||
        (nexus.maxDuration > 0 && type != AnimationType_STATUS && elapsed > nexus.maxDuration) ) {
        // It must stop!
        Serial.printf("StopAnimation(%d)\n", type);

        animator.StopAnimation(type);

        // If it is base, we try to start a next one though
        if (type == AnimationType_BASE) {
            nextAnimation(true);
        }

        return;
    }

    // If it has finished, it might be time to start something new though
    if (param.state == AnimationState_Completed) {
        if (pCur->loops) {
            // Restart it - woo hoo!
            startAnimation(pRunningAnims[type], true);
            // animator.RestartAnimation(type);
        } else if (type == AnimationType_BASE) {
            // Only base will automatically move to a new animation

            // Need to find a next animation for this channel
            nextAnimation(true);
        }
    }
}

LEDArtAnimation*
LEDArtPiece::findNextBaseAnimation(bool randomize)
{
    LEDArtAnimation* pCur = pRunningAnims[AnimationType_BASE];

    if (!pCur) {
        return pRegistrations[AnimationType_BASE];
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
    pNext = pRegistrations[AnimationType_BASE];
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
    // Just do something else
    if (!szName)
    {
        nextAnimation(randomize);
        return;
    }

    // Try to find something specific to do
    LEDArtAnimation* pCursor = pRegistrations[AnimationType_BASE];

    while(pCursor) {
        if (strcmp(pCursor->szName, szName) == 0)
        {
            // Found it! Start it!
            startAnimation(pCursor, false);
            return;
        }
        pCursor = pCursor->pNext;
    }

    // Else, did not find. Tell someone??
}