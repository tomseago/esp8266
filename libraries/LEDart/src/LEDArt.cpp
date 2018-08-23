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

const LAColor black(0,0,0);

const LAColor Palette_RB[] = {
    LAColor(255, 0, 0),
    LAColor(0, 0, 255),
};

const LAColor Palette_RGB[] = {
    LAColor(255, 0, 0),
    LAColor(0, 255, 0),
    LAColor(0, 0, 255),
};

const LAColor Palette_RYB[] = {
    LAColor(255, 0, 0),
    LAColor(255, 64, 0),
    LAColor(255, 128, 0),
    LAColor(255, 191, 0),
    LAColor(255, 255, 0),
    LAColor(128, 212, 25),
    LAColor(0, 168, 51),
    LAColor(21, 132, 102),
    LAColor(42, 95, 153),
    LAColor(85, 48, 140),
    LAColor(128, 0, 128),
    LAColor(191, 0, 64),
};

const LAColor Palette_WHITE_BLACK[] = {
    LAColor(0, 0, 0),
    LAColor(96, 96, 96),
    LAColor(255, 255, 255),
};

const LAColor Palette_WHITE_BLACK2[] = {
    LAColor(10, 10, 10),
    LAColor(96, 96, 96),
    LAColor(255, 255, 255),
};

const LAColor Palette_JORDAN[] = {
    LAColor(0, 0, 0),
    LAColor(96, 96, 96),
    LAColor(255, 255, 255),
};

const LAColor Palette_MARDI_GRAS[] = {
    LAColor(176, 126, 9),
    LAColor(176, 126, 9),
    LAColor(4, 87, 22),
    LAColor(45, 6, 56),
    LAColor(45, 6, 56),
};

const LAColor Palette_BLUES[] = {
    LAColor(32, 74, 255),
    LAColor(0, 23, 123),
    LAColor(3, 19, 21),
    LAColor(21, 18, 33),
    LAColor(1, 1, 5),
};

const LAColor Palette_BLUES2[] = {
    LAColor(32, 74, 255),
    LAColor(0, 23, 123),
    LAColor(3, 19, 21),
    LAColor(21, 18, 33),
    LAColor(32, 74, 255),
};

const LAColor Palette_PINKS[] = {
    LAColor(255, 59, 51),
    LAColor(191, 43, 87),
    LAColor(127, 48, 27),
    LAColor(64, 15, 13),
    LAColor(229, 92, 87),
};

const LAColor Palette_REDS[] = {
    LAColor(255, 0, 0),
    LAColor(225, 32, 5),
    LAColor(35, 0, 0),
    LAColor(0, 0, 0),
};

const LAColor Palette_REDS2[] = {
    LAColor(255, 0, 0),
    LAColor(225, 32, 5),
    LAColor(35, 0, 0),
    LAColor(40, 0, 0),
};

const LAColor* LEDArtAnimation::paletteColors[] = {
    Palette_RB,
    Palette_RGB,
    Palette_RYB,
    Palette_WHITE_BLACK2,
    Palette_MARDI_GRAS,
    Palette_BLUES2,
    Palette_PINKS,
    Palette_REDS2,
};

// const uint8_t LEDArtAnimation::rybRainbowColorsCount = 3;

// const LAColor LEDArtAnimation::rybRainbowColors[] = {
//     LAColor(0, 0, 255),
//     LAColor(255, 0, 0),
//     LAColor(0, 255, 0),
// };
LEDArtAnimation::LEDArtAnimation(char* szName) {
    this->szName = strdup(szName);
}

LEDArtAnimation::~LEDArtAnimation() {
    free(szName);
}

LAColor
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

    LAColor color = LAColor::LinearBlend(paletteColors[palette][lowIx], paletteColors[palette][highIx], intervalDistance);

    // Serial.printf(" ==> (%3d, %3d, %3d)\n", color.R, color.G, color.B);
    return color;
}

void
LEDArtAnimation::clearTo(LEDArtPiece& piece, LAColor color, uint16_t start, uint16_t end) {
    for(uint16_t ix=start; ix<end; ix++) {
        piece.strip.SetPixelColor(ix, color);
    }
}

/////


#if LEDART_RGBW

float saturation(LAColor& rgbw) {
    float low = std::min(rgbw.R, std::min(rgbw.G, rgbw.B));
    float high = std::max(rgbw.R, std::max(rgbw.G, rgbw.B));
    if (high == 0.0) return 0;

    // uint8_t s = round(100.0 * ((high - low) / high));
    uint8_t s = round(200.0 * ((high - low) / high));
    // Log.printf("\n   sat=%d\n",s);
    return s;
}

uint8_t getWhite(LAColor& rgbw) {
    float a = (255.0 - saturation(rgbw)) / 255.0;
    float b = ((uint32_t)rgbw.R + (uint32_t)rgbw.G + (uint32_t)rgbw.B) / 3;
    // Log.printf("\n %f * %f\n", a, b);
    return a * b;
}

uint8_t getWhite(LAColor& rgbw, uint8_t rMax, uint8_t gMax, uint8_t bMax) {
    rgbw.R = (float)rgbw.R / 255.0 * (float)rMax;
    rgbw.G = (float)rgbw.G / 255.0 * (float)gMax;
    rgbw.B = (float)rgbw.B / 255.0 * (float)bMax;
    return ((255.0 - saturation(rgbw)) / 255.0) * (rgbw.R + rgbw.G + rgbw.B) / 3.0;
}

void
convertLAColor(LAColor& in) {
    // Log.printf("Color %2x%2x%2x%2x",
    //     in.R, in.G, in.B, in.W);

    // Without correction
    in.W = getWhite(in);

    // With some max correction
    //out.W = getWhite(in, 255, 200, 200);

    // Log.printf(" -> %2x%2x%2x%2x\n",
    //     in.R, in.G, in.B, in.W);
}

#else
void
convertLAColor(LAColor& in) {
}

#endif


LEDArtPiece::LEDArtPiece(Nexus& nx, uint16_t pixelCount, uint8_t maxBrightness, uint8_t port) : 
    nexus(nx),
    strip(pixelCount, port)
{
    nx.maxBrightness = maxBrightness;
}

void* 
LEDArtPiece::registerGeometry(LEDArtGeometry* pGeom)
{
    if (!pGeom) return NULL;

    pGeom->pNext = pGeomRegistrations;
    pGeomRegistrations = pGeom;

    pGeom->baseId = ++lastGeomId;

    if (!pCurrentGeom)
    {
        // The first one, yay! Set it as default.
        pCurrentGeom = pGeom;
    }

    return pGeom;
}

void* 
LEDArtPiece::registerAnimation(LEDArtAnimation* pAnim) 
{
    if (!pAnim) return NULL;

    LEDAnimationChannel* pChannel = channels + pAnim->type;

    pAnim->pNext = pChannel->pRegistrations;
    pChannel->pRegistrations = pAnim;

    return pAnim;
}

void* 
LEDArtPiece::enumerateGeometries(void* cursor, char** pszName, bool* pCanRotate)
{
    // Presume the cursor value a pointer to the current thing we should return
    // some data for
    LEDArtGeometry* pGeom = (LEDArtGeometry*)cursor;

    if (!pGeom)
    {
        pGeom = pGeomRegistrations;

        // start a new enumeration
        if (!pGeom)
        {
            // Nothng to do at all
            if (pszName)
            {
                *pszName = NULL;
            }
            return NULL;
        }
    }

    if (pszName)
    {
        *pszName = pGeom->szName;
    }
    if (pCanRotate)
    {
        *pCanRotate = pGeom->canRotate;
    }

    return pGeom->pNext;
}

void*
LEDArtPiece::enumerateAnimations(void* cursor, char** pszName)
{
    LEDArtAnimation* pAnim = (LEDArtAnimation*)cursor;

    if (!pAnim)
    {
        pAnim = channels[LEDAnimationType_BASE].pRegistrations;
        if (!pAnim)
        {
            if (pszName)
            {
                *pszName = NULL;
            }
            return NULL;
        }
    }

    if (pszName)
    {
        *pszName = pAnim->szName;
    }

    return pAnim->pNext;
}

void 
LEDArtPiece::begin() 
{
    // Convert all the colors in the pallettes
    for(int i=0; i<LEDArtAnimation::LEDPalette_LAST; i++) 
    {
        const LAColor* palette = LEDArtAnimation::paletteColors[i];

        for(int j=0; j<LEDArtAnimation::paletteSizes[i]; j++)
        {
            LAColor* pC = (LAColor*)(palette + j);
            convertLAColor(*pC);
        }
    }

    // Be nice and zero it all out
    strip.Begin();
    strip.Show();

    // Start the first base animation
    nextRandomBaseAnimation();
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
LEDArtPiece::updateBrightness(LEDArtAnimation* pAnim)
{
    if (!pAnim) return;

    uint8_t b = nexus.maxBrightness * pAnim->brightness;

    // Log.printf("PIECE: strip.SetBrightness(%d)\n", b);
    strip.SetBrightness(b);
}

void
LEDArtPiece::startAnimation(LEDArtAnimation* pAnim, bool isLoop, uint32_t now) 
{

    if (!pAnim) {
        return;
    }

    if (!now) 
    {
        now = millis() + nexus.localTimeOffset;
    }

    LEDAnimationChannel* pChannel = channels + pAnim->type;
    pChannel->pRunning = pAnim;
    pChannel->loopStartedAt = now;    
    if (!isLoop)
    {       
        // First time through 
        pChannel->channelStartedAt = now;
        Log.printf("PIECE: Start animation '%s' type=%d mb=%f\n", pAnim->szName, pAnim->type, nexus.maxBrightness);

        if (pAnim->type == LEDAnimationType_BASE) {
            nexus.setAnimation(pAnim->szName, (uint32_t)this);
        }
    } else {
        //Log.printf("PIECE: Loop animation %s\n", pAnim->szName);
    }

    updateBrightness(pAnim);

    // Dispatch time 0
    LEDAnimationParam param;
    param.state = LEDAnimationState_Started;
    param.progress = nexus.reverse ? 1.0f : 0.0f;
    pAnim->animate(*this, param);

    // If we are a channel start, we might need to prepare for the end of times
    if (!isLoop && pAnim->type == LEDAnimationType_BASE && nexus.shouldPrepareRandomStatesFor)
    {
        // We are the master and thus need to create a new prepared state because 
        // of the start of a new animation.
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
            Log.printf("PIECE: shouldPrepareRandomStatesFor set but no maxDuration anywhere. Ignoring it.\n");
        }
        else
        {
            uint32_t nextStateTime = now + minDuration - nexus.shouldPrepareRandomStatesFor;
            bool rotated = false;
            LEDArtGeometry* pNextGeom = findNextGeometry(true, false, &rotated);
            LEDArtAnimation* pNextAnim = findNextBaseAnimation(true, pNextGeom, rotated);
            // Log.printf("PIECE: rotated=%s\n", rotated ? "true" : "false");
            nexus.prepareStateFor(nextStateTime, true, pNextGeom->szName, rotated, pNextAnim->szName, (uint32_t)this);
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
LEDArtPiece::nextRandomBaseAnimation(uint32_t now) 
{
    // Get a new candidate geometry
    bool rotated = false;
    LEDArtGeometry* pGeom = findNextGeometry(true, false, &rotated);

    // A random animation, that will work with that geometry
    LEDArtAnimation* pAnim = findNextBaseAnimation(true, pGeom, rotated);

    if (pAnim != channels->pRunning)
    {
        // Since it is a new pAnim we can assume that candidate geometry is a good
        // one, which means do a proper setting of it
        pCurrentGeom = pGeom;
        geomRotated = rotated;

        nexus.setGeometry(pCurrentGeom->szName, geomRotated, (uint32_t)this);
    }
    // else we failed to find a new animation, so assume we should also
    // keep using the old geometry, which means don't update it.

    // Randomize other stuff
    nexus.randomizeAll((uint32_t)this);

    // And finally, start it
    startAnimation(pAnim, false, now);
}


int logLimit = 10;

void
LEDArtPiece::animateChannel(LEDAnimationType type, uint32_t now) 
{
    LEDAnimationChannel* pChannel = channels + type;
    LEDArtAnimation* pAnim = pChannel->pRunning;
    if (!pAnim)
    {
        // Nothing on the channel is running
        // Log.printf("PIECE: AC nothing on %d.\n", type);
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

    if (logLimit)
    {
        logLimit--;
        Log.printf("PIECE: AC chan(%d) ce:%d le:%d ld: %d sf:%f lad:%d prog:%f state=%d\n", type, channelElapsed, loopElapsed, pAnim->loopDuration, nexus.speedFactor, loopAdjustedDuration, param.progress, param.state);
    }

    updateBrightness(pAnim);
    
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
            Log.printf("PIECE: AC nextRandomBaseAnimation().\n");
            nextRandomBaseAnimation(now);
        }
        else
        {
            Log.printf("PIECE: AC stopAnimation(%d).\n", type);
            stopAnimation(type);
        }
        return;
    }

    // Do we have a prepared thing to do next?
    char* szPreparedGeomName = NULL;
    bool rotated = false;
    char* szPreparedAnimName = NULL;
    uint32_t preparedReadyAt = nexus.nextPreparedState(&szPreparedGeomName, &rotated, &szPreparedAnimName);
    if (preparedReadyAt && preparedReadyAt < now)
    {
        // Oh hey! try to do the next geometry & animation
        if (szPreparedAnimName)
        {
            LEDArtAnimation* pNext = baseAnimForName(szPreparedAnimName);
            if (!pNext)
            {
                Log.printf("PIECE: AC Did not find requested animation '%s' from prepared state\n", szPreparedAnimName);
                nexus.clearPreparedState();
            }
            else
            {
                Log.printf("PIECE: AC Starting prepared animation '%s'\n", szPreparedAnimName);
                nexus.usePreparedState();

                // Set the geometry if we can
                LEDArtGeometry* pNextGeom = geomForName(szPreparedGeomName);
                if (pNextGeom)
                {
                    pCurrentGeom = pNextGeom;
                    if (rotated && pNextGeom->canRotate)
                    {
                        geomRotated = rotated;
                    }
                    else
                    {
                        geomRotated = false;
                    }
                    Log.printf("PIECE: AC Using prepared geom '%s' rotated=%s\n", szPreparedGeomName, geomRotated ? "true" : "false");

                    // The nexus may care that we changed things
                    nexus.setGeometry(pCurrentGeom->szName, geomRotated, (uint32_t)this);                   
                }
                else 
                {
                    Log.printf("PIECE: AC could not find prepared geom '%s'\n", szPreparedGeomName);
                }

                // Can't clear the prepared state until done using szPreparedGeomName
                nexus.clearPreparedState();                

                startAnimation(pNext, false, now);
                return;
            }
        }
        else
        {
            Log.printf("PIECE: AC Prepared state had no anim name, ignoring\n");
            nexus.clearPreparedState();
        }
    }

    // Was that completed though?
    // If it has finished, it might be time to start something new though
    if (param.state == LEDAnimationState_Completed) {
        if (logLimit) Log.printf("PIECE: AC Completed(%d) pAnim->loops=%d forced=%d\n", type, pAnim->loops, nexus.forcedForeverLoop);
        if (pAnim->loops || nexus.forcedForeverLoop) {
            // Restart it - woo hoo!
            if (logLimit) Log.printf("PIECE: AC Restart animation\n");
            startAnimation(pAnim, true, now);
        } else if (type == LEDAnimationType_BASE) {
            // Only base will automatically move to a new animation

            // Need to find a next animation for this channel
            if (logLimit) Log.printf("PIECE: AC do nextRandomBaseAnimation\n");
            nextRandomBaseAnimation();
        }
    }
}



bool
LEDArtPiece::testAnimGeomCompat(LEDArtAnimation* pAnim, LEDArtGeometry* pGeom, bool rotated)
{
    if (!pAnim || !pGeom) return false;

    if (rotated) {
        return pAnim->canUseGeom(pGeom->getHeight(), pGeom->getWidth());
    }
    else
    {
        return pAnim->canUseGeom(pGeom->getWidth(), pGeom->getHeight());        
    }
}



LEDArtGeometry*
LEDArtPiece::geomForName(char* szName)
{
    if (!szName)
    {
        return NULL;
    }

    // Try to find something specific to do
    LEDArtGeometry* pCursor = pGeomRegistrations;

    while(pCursor) {
        if (strcmp(pCursor->szName, szName) == 0)
        {
            // Found it!
            return pCursor;
        }
        pCursor = pCursor->pNext;
    }

    // Else, did not find. Tell someone??
    return NULL;
}


LEDArtGeometry*
LEDArtPiece::findNextGeometry(bool randomize, bool withCompat, bool* pRotated)
{
    LEDArtGeometry* pGeom = pCurrentGeom;
    LEDAnimationChannel* pChannel = channels + LEDAnimationType_BASE;
    LEDArtAnimation* pAnim = pChannel->pRunning;

    if (!pGeom) 
    {
        Log.printf("PIECE: ERROR: No geometries registered before findNextGeometry was called\n");
        return NULL;
    }

    // There is a degenerate situation with a single geometry where
    // if we try to randomize we would ignore the rotated version of
    // this geometry. To avoid that we special case ignoring randomize
    // in that situation.
    if (pGeomRegistrations == pGeom && !pGeom->pNext) 
    {
        // Return the alternate, or don't
        if (pGeom->canRotate && (!withCompat || testAnimGeomCompat(pAnim, pGeom, !geomRotated)))
        {
            // Return the alternative
            if (pRotated)
            {
                Log.printf("PIECE: Special case 1 geom, rotate  pAnim=%s\n", pAnim ? pAnim->szName : "NULL");
                *pRotated = !geomRotated;
            }
            return pGeom;
        }
        // else, sorry, only got the one, gotta live with it
        if (pRotated)
        {
            Log.printf("PIECE: Special case 1 geom, no change to rotation  pAnim=%s\n", pAnim ? pAnim->szName : "NULL");
            *pRotated = geomRotated;
        }
        return pGeom;
    }

    // Maybe the answer is to just rotate the current one?
    if (!geomRotated && pGeom->canRotate && !randomize)
    {
        // It is a candidate!
        if (!withCompat || testAnimGeomCompat(pAnim, pGeom, true))
        {
            // We either don't care compat or it is okay, so yeah, rotated version of the
            // current one is our winner
            if (pRotated)
            {
                *pRotated = true;
            }
            return pGeom;
        }
        // Aack! It didn't work out, fall through to at least the "next"
    }

    // Search from the current location to the end of the list
    LEDArtGeometry* pOut = _searchForGeom(randomize, withCompat, pRotated, pGeom->pNext, NULL);
    if (pOut) return pOut;

    // Search from the head of the list until the current location
    pOut = _searchForGeom(randomize, withCompat, pRotated, pGeomRegistrations, pGeom);
    if (pOut) return pOut;

    // If we didn't find anything NEW, we presume the current one is fine
    if (pRotated)
    {
        *pRotated = geomRotated;
    }
    return pGeom;
}

LEDArtGeometry*
LEDArtPiece::_searchForGeom(bool randomize, bool withCompat, bool* pRotated, LEDArtGeometry* pStart, LEDArtGeometry* pEnd)
{
    LEDArtGeometry* pNext = pStart;
    LEDAnimationChannel* pChannel = channels + LEDAnimationType_BASE;
    LEDArtAnimation* pAnim = pChannel->pRunning;

    // First half of the search, from this point to the end of the list
    while(pNext != pEnd) {
        if (!randomize)
        {
            // Since we're not randomizing, just use this one, as long as 
            // it is okay to do so
            if (!withCompat || testAnimGeomCompat(pAnim, pNext, false)) 
            {
                if (pRotated)
                {
                    *pRotated = false;
                }
                return pNext;
            }

            // But if we rotate it is okay?
            if (testAnimGeomCompat(pAnim, pNext, true)) 
            {
                if (pRotated)
                {
                    *pRotated = true;
                }
                return pNext;
            }

            // Nope, no matter what it sucks. Need to loop
        }
        else
        {
            // We desire a random selection, so maybe this one is it, but maybe not?
            if ( rand(100) < 60 ) {
                // Oh okay, we are selecting it, so randomly choose a rotation
                bool isRotated = pNext->canRotate ? (rand(100) < 50) : false;

                // See if that works
                if (!withCompat || testAnimGeomCompat(pAnim, pNext, isRotated)) 
                {
                    if (pRotated)
                    {
                        *pRotated = isRotated;
                    }
                    return pNext;
                }

                // Since our visitation counter is only to registrations, not registrations
                // plus states, we have to test all options while visiting this geometry. If
                // we didn't then a degenerate situation with a single strip geometry that didn't
                // get the right rotation selection wouldn't select anything
                if (testAnimGeomCompat(pAnim, pNext, !isRotated)) 
                {
                    if (pRotated)
                    {
                        *pRotated = !isRotated;
                    }
                    return pNext;
                }
            }
            // Didn't decide we liked the look of it, so just loop
        }

        pNext = pNext->pNext;
    }

    return NULL;
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


LEDArtAnimation*
LEDArtPiece::findNextBaseAnimation(bool randomize, LEDArtGeometry* pGeom, bool rotated)
{
    LEDAnimationChannel* pChannel = channels + LEDAnimationType_BASE;
    LEDArtAnimation* pAnim = pChannel->pRunning;

    Log.printf("PIECE: findNextBaseAnimation pGeom=%s rotated=%d\n", pGeom ? pGeom->szName : "NULL", rotated);

    if (!pAnim) {
        // Nothing running, use the first registration as long as it is compatible
        // with the current geometry.
        pAnim = pChannel->pRegistrations;

        if (!pGeom || testAnimGeomCompat(pAnim, pGeom, rotated))
        {
            return pAnim;
        }
        // Aack, it's not compatible, so go on a hunt
    }

    LEDArtAnimation* pNext = pAnim->pNext;

    // First half of the search, from this point to the end of the list
    while(pNext) {
        if (pNext->isEnabled && 
            (!randomize ||
                (rand(100) < 60) )) 
        {
            // We _want_ to select it, let's make sure we can
            if (!pGeom || testAnimGeomCompat(pNext, pGeom, rotated))
            {
                // neat
                return pNext;
            }
            // No, no, can't use that one!
        }

        pNext = pNext->pNext;
    }

    // Second half we restart at the very head of the list and go until
    // we get to the current one
    pNext = pChannel->pRegistrations;
    while(pNext != pAnim) {
        if (pNext->isEnabled && 
            (!randomize ||
                (rand(100) < 60) )) 
        {
            // We _want_ to select it, let's make sure we can
            if (!pGeom || testAnimGeomCompat(pNext, pGeom, rotated))
            {
                // neat
                return pNext;
            }
            // No, no, can't use that one!
        }

        pNext = pNext->pNext;
    }

    if (randomize)
    {
        // We maybe looked at everything and selected nothing, so try again
        // without the randomize, presuming _something_ is compatible with
        // the given geometry
        Log.printf("PIECE: findNextBaseAnimation without randomize\n");
        return findNextBaseAnimation(false, pGeom, rotated);
    }

    // Oops! We didn't find a "next", but because we want to always have 
    // something, just return the current again...
    Log.printf("PIECE: findNextBaseAnimation without random returning current anim, which may not be compatible with the requested geom\n");
    return pAnim;
}



///////// Nexus Listener

void
LEDArtPiece::nexusValueUpdate(NexusValueType which, uint32_t source) 
{

}


void
LEDArtPiece::nexusUserGeometryRequest(char* szName, bool rotated, uint32_t source)
{
    LEDArtGeometry* pRequested = geomForName(szName);

    if (pRequested)
    {
        // Let's make sure it's actually compatible with the current
        // anim though...        
        if (!testAnimGeomCompat(channels->pRunning, pRequested, rotated))
        {
            Log.printf("PIECE: Requested geometry is not compatible though, so doing a next operation instead");
            pRequested = NULL;
            rotated = false; // no reason to be random
        }
    }

    if (!pRequested)
    {
        // Find the next one then, using rotated to indicate random or not.
        // It really _should_ be compatible...
        pRequested = findNextGeometry(rotated, true, &rotated);

        Log.printf("PIECE: findNextGeom %s %s\n", pCurrentGeom->szName, geomRotated ? "true" : "false");
    }

    if (!nexus.shouldPrepareRandomStatesFor)
    {
        // We are not a master, just do the thing
        pCurrentGeom = pRequested;
        geomRotated = rotated;        

        nexus.setGeometry(pCurrentGeom->szName, geomRotated, source);
    }
    else
    {
        // Make a prepared state for this which will cause it to be
        // activated next loop through. Yes it will restart the animation,
        // but that feels like a small price to pay at this point.
        nexus.prepareStateFor(millis()-1, false, pRequested->szName, rotated, channels->pRunning->szName, (uint32_t)this);
    }
}


// Can pass NULL as szName to ask for a random selection
void
LEDArtPiece::nexusUserAnimationRequest(char* szName, bool randomize, uint32_t source)
{
    LEDArtAnimation* pRequested = baseAnimForName(szName);

    if (pRequested)
    {
        // But can it run in this geometry?
        if (!testAnimGeomCompat(pRequested, pCurrentGeom, geomRotated))
        {
            Log.printf("PIECE: Requested anim is not compatible with current geom. Doing a next opertaion.");
            pRequested = NULL;
        }
    }

    if (!pRequested)
    {
        // Just do something else that does exist and is compatible with
        // the current geometry.
        pRequested = findNextBaseAnimation(randomize, pCurrentGeom, geomRotated);
    }

    if (!nexus.shouldPrepareRandomStatesFor)
    {
        // We are not master, just do it
        startAnimation(pRequested);
    }
    else
    {
        nexus.prepareStateFor(millis()-1, false, pCurrentGeom->szName, geomRotated, pRequested->szName, (uint32_t)this);        
    }
}


///////// Geometries

uint8_t 
LEDArtPiece::geomId()
{
    if (!pCurrentGeom) return 0;

    uint8_t ret = pCurrentGeom->baseId;
    if (geomRotated)
    {
        ret |= 0x80;
    }

    return ret;
}

uint16_t 
LEDArtPiece::geomHeight()
{
    if (!pCurrentGeom) return 0;

    return geomRotated ? pCurrentGeom->getWidth() : pCurrentGeom->getHeight();
}

uint16_t
LEDArtPiece::geomWidth(int16_t rowIx)
{
    if (!pCurrentGeom) return 0;

    return geomRotated ? pCurrentGeom->getHeight() : pCurrentGeom->getWidth(rowIx);
}

// Careful, this is in the order secondary, primary
uint16_t
LEDArtPiece::geomMap(uint16_t x, uint16_t y)
{
    if (!pCurrentGeom) return 0xffff;

    return geomRotated ? pCurrentGeom->map(y,x) : pCurrentGeom->map(x,y);
}

void
LEDArtPiece::setPrimaryColor(uint16_t primaryIx, LAColor color)
{
    uint16_t width = geomWidth(primaryIx);
    for(uint16_t secIx=0; secIx<width; secIx++)
    {
        strip.SetPixelColor(geomMap(secIx, primaryIx), color);
    }
}

void
LEDArtPiece::setSecondaryColor(uint16_t secondaryIx, LAColor color)
{
    uint16_t height = geomHeight();
    for(uint16_t priIx=0; priIx<height; priIx++)
    {
        strip.SetPixelColor(geomMap(secondaryIx, priIx), color);
    }
}

void
LEDArtPiece::setSecondaryColorInPrimary(uint16_t primaryIx, uint16_t secondaryIx, LAColor color)
{
    strip.SetPixelColor(geomMap(secondaryIx, primaryIx), color);
}
