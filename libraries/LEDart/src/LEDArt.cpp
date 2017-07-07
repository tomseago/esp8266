#include "LEDArt.h"

//////

const uint8_t LEDArtAnimation::paletteSizes[] = {
    2,  // RB
    3,  // RGB
    12, // RYB
};

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

const RgbColor* LEDArtAnimation::paletteColors[] = {
    Palette_RB,
    Palette_RGB,
    Palette_RYB,
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


LEDArtPiece::LEDArtPiece(uint16_t pixelCount, uint16_t width, uint16_t height, uint8_t port) : 
    strip(pixelCount, port), 
    topo(width, height),
    animator(3) 
{
    pSingleton = this;
}


void 
LEDArtPiece::begin() {
    // Be nice and zero it all out
    strip.Begin();
    strip.Show();

    // Start the first base animation
    LEDArtAnimation* pAnim = findNextBaseAnimation();
    startAnimation(pAnim);
}


void 
LEDArtPiece::loop() {
    animator.UpdateAnimations();
    strip.Show();
}


void
LEDArtPiece::startAnimation(LEDArtAnimation* pAnim) {

    if (!pAnim) {
        return;
    }

    // strip.SetBrightness(maxBrightness * pAnim->brightness);
    uint8_t b = maxBrightness * pAnim->brightness;
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
    animator.StartAnimation(pAnim->type, pAnim->loopDuration, func);

    startedAt[pAnim->type] = millis();
}

void 
LEDArtPiece::stopAnimation(AnimationType type) {
    animator.StopAnimation(type);
}

void 
LEDArtPiece::nextAnimation() {
    LEDArtAnimation* pAnim = findNextBaseAnimation();
    startAnimation(pAnim);   
}

void* 
LEDArtPiece::registerAnimation(LEDArtAnimation* pAnim) {

    if (!pAnim) return NULL;

    pAnim->pNext = pRegistrations[pAnim->type];
    pRegistrations[pAnim->type] = pAnim;

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

    // We believe there is something running, so let it do it's thing
    pCur->animate(*this, param);


    // Has it hit max time?
    uint32_t elapsed = millis() - startedAt[type];
    if ((pCur->maxDuration > 0 && elapsed >= pCur->maxDuration) ||
        (elapsed > maxDuration) ) {
        // It must stop!
        animator.StopAnimation(type);

        // If it is base, we try to start a next one though
        if (type == AnimationType_BASE) {
            nextAnimation();
        }

        return;
    }

    // If it has finished, it might be time to start something new though
    if (param.state == AnimationState_Completed) {
        if (pCur->loops) {
            // Restart it - woo hoo!
            animator.RestartAnimation(type);
        } else if (type == AnimationType_BASE) {
            // Only base will automatically move to a new animation

            // Need to find a next animation for this channel
            nextAnimation();
        }
    }
}

LEDArtAnimation*
LEDArtPiece::findNextBaseAnimation()
{
    LEDArtAnimation* pCur = pRunningAnims[AnimationType_BASE];

    if (!pCur) {
        return pRegistrations[AnimationType_BASE];
    }

    LEDArtAnimation* pNext = pCur->pNext;

    // First half of the search, from this point to the end of the list
    while(pNext) {
        if (pNext->isEnabled) {
            return pNext;
        }

        pNext = pNext->pNext;
    }

    // Second half we restart at the very head of the list and go until
    // we get to the current one
    pNext = pRegistrations[AnimationType_BASE];
    while(pNext != pCur) {
        if (pNext->isEnabled) {
            return pNext;
        }

        pNext = pNext->pNext;
    }

    // Oops! We didn't find a "next", but because we want to always have 
    // something, just return the current again...
    return pCur;
}
