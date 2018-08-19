#include "nexus.h"
#include "rand.h"
#include "log.h"

Nexus::Nexus() 
{

}

// void
// Nexus::addGeometry(char *szName)
// {
//     char* szNameCpy = strdup(szName);
//     geomNames.push_back(szNameCpy);
// }

// uint8_t
// Nexus::numGeometries()
// {
//     return geomNames.size();
// }

// char *
// Nexus::geomName(uint8_t index)
// {
//     return geomNames.at(index);
// }

// void
// Nexus::addAnimation(char *szName)
// {
//     char* szNameCpy = strdup(szName);
//     animNames.push_back(szNameCpy);
// }

// uint8_t
// Nexus::numAnimations()
// {
//     return animNames.size();
// }

// char *
// Nexus::animName(uint8_t index)
// {
//     return animNames.at(index);
// }

void
Nexus::addListener(NexusListener* listener)
{
    if (!listener) return;

    listeners.push_back(listener);
}

void
Nexus::randomizeAll(uint32_t source)
{
    NexusState toApply;
    randomizeState(&toApply);
    applyState(&toApply);
}

void
Nexus::nextPalette(uint32_t source)
{
    uint8_t nextPalette = (uint8_t)palette + 1;
    palette = (LEDArtAnimation::LEDPaletteType)nextPalette;
    if (palette == LEDArtAnimation::LEDPalette_LAST) {
        palette = (LEDArtAnimation::LEDPaletteType)0;
    }

    sendValueUpdate(NexusListener::Palette, source);
    Log.printf("NX: nextPalette now=%d\n",(uint8_t)palette);
}

void
Nexus::setGeometry(char* szName, bool rotated, uint32_t source)
{
    if (!szName)
    {
        return;
    }

    if (szCurrentGeom)
    {
        free(szCurrentGeom);
    }

    szCurrentGeom = strdup(szName);
    geomRotated = rotated;

    sendValueUpdate(NexusListener::CurrentGeomName, source);
}

void
Nexus::setAnimation(char* szName, uint32_t source)
{
    if (!szName)
    {
        return;
    }

    if (szCurrentAnim)
    {
        free(szCurrentAnim);
    }

    szCurrentAnim = strdup(szName);

    sendValueUpdate(NexusListener::CurrentAnimName, source);
}

void 
Nexus::sendValueUpdate(NexusListener::NexusValueType which, uint32_t source)
{
    for(NexusListener* listener : listeners) 
    {
        listener->nexusValueUpdate(which, source);  
    } 

}


void 
Nexus::sendUserGeometryRequest(char* szName, bool rotated, uint32_t source)
{
    for(NexusListener* listener : listeners) 
    {
        listener->nexusUserGeometryRequest(szName, rotated, source);  
    } 
}

void 
Nexus::sendUserAnimationRequest(char* szName, bool randomize, uint32_t source)
{
    for(NexusListener* listener : listeners) 
    {
        listener->nexusUserAnimationRequest(szName, randomize, source);  
    } 
}

//////

void 
Nexus::prepareStateFor(uint32_t when, bool randomize, char* szGeomName, bool rotated, char* szAnimName, uint32_t source)
{
    clearPreparedState();

    gatherState(&nextState);
    if (randomize)
    {
        randomizeState(&nextState);
    }
    nextState.geomRotated = rotated;
    nextState.time = when;
    nextState.override = !randomize;

    if (szGeomName) 
    {
        nextState.geomNameLen = strlen(szGeomName);
        if (nextState.geomNameLen)
        {
            szNextGeom = strdup(szGeomName);
            if (!szNextGeom) {
                nextState.geomNameLen = 0;
            }
        }
        // else nothing to dup
    }

    if (szAnimName) 
    {
        nextState.animNameLen = strlen(szAnimName);
        if (nextState.animNameLen)
        {
            szNextAnim = strdup(szAnimName);
            if (!szNextAnim) {
                nextState.animNameLen = 0;
            }
        }
        // else nothing to dup
    }


    Log.printf("NEXUS: prepareStateFor(%d)\n", when);
    logState(&nextState, szGeomName, szAnimName);

    sendValueUpdate(NexusListener::NexusValueType::PreparedState, source);
}

uint32_t
Nexus::nextPreparedState(char **pszGeomName, bool* pGeomRotated, char **pszAnimName)
{
    if (!nextState.time)
    {
        // There is no next state
        if (pszGeomName)
        {
            *pszGeomName = NULL;
        }
        if (pGeomRotated)
        {
            *pGeomRotated = false;
        }
        if (pszAnimName)
        {
            *pszAnimName = NULL;
        }
        return 0;
    }

    if (pszGeomName)
    {
        *pszGeomName = szNextGeom;
    }
    if (pGeomRotated)
    {
        *pGeomRotated = nextState.geomRotated;
    }

    if (pszAnimName)
    {
        *pszAnimName = szNextAnim;
    }

    return nextState.time;
}

void
Nexus::usePreparedState() 
{
    applyState(&nextState);
}

void
Nexus::clearPreparedState() 
{
    // We have used the active one we had
    nextState.time = 0;

    if (szNextGeom)
    {
        free(szNextGeom);
        szNextGeom = NULL;
    }

    if (szNextAnim)
    {
        free(szNextAnim);
        szNextAnim = NULL;
    }

    // Do we have a queued state though?
    if (nextNextStateData) {
        deserializeState(false, nextNextStateLength, nextNextStateData);

        free(nextNextStateData);
        nextNextStateData = NULL;
    }

    // testSerializer();
}

uint16_t
Nexus::serializeState(bool isCurrent, bool withOverride, uint8_t* into)
{
    if (isCurrent)
    {
        // Our current state
        NexusState current;

        gatherState(&current);

        return serializeState(&current, szCurrentGeom, szCurrentAnim, into);
    }
    else
    {
        // The prepared state
        if (!nextState.time)
        {
            return 0;
        }

        return serializeState(&nextState, szNextGeom, szNextAnim, into);       
    }
}

bool
Nexus::deserializeState(bool isCurrent, uint16_t length, const uint8_t* from)
{
    if (isCurrent) 
    {
        // We're going to deserialize this and use it right away
        NexusState update;
        if (!deserializeState(&update, NULL, NULL, length, from)) 
        {
            return false;
        }

        applyState(&update);

        // Let's assume we don't need to notify anyone. The animation
        // will poll the stuff it needs.
        return true;
    }

    // Cheat and use the knowledge that the first byte is the override
    // if (from[0])
    // {
    //     Log.printf("NEXUS: --- Override state\n");
    // }

    if (nextState.time && !from[0]) {
        Log.printf("NEXUS: Got a new prepared state before using the last one. Queing it up...\n");
        stateQEvents++;

        // If it's on top of something else, remove the old one
        if (nextNextStateData) {
            free(nextNextStateData);
        }
        nextNextStateData = (uint8_t*)malloc(length);
        if (!nextNextStateData) {
            Log.printf("NEXUS: OOM Error queing new state\n");
            return false;
        }
        nextNextStateLength = length;
        memcpy(nextNextStateData, from, length);
        return true;
    }

    // Free to do a regular deserialization

    if (szNextGeom)
    {
        free(szNextGeom);
        szNextGeom = NULL;
    }

    if (szNextAnim)
    {
        free(szNextAnim);
        szNextAnim = NULL;
    }

    return deserializeState(&nextState, &szNextGeom, &szNextAnim, length, from);
}


void
Nexus::randomizeState(NexusState* state)
{
    if (!state) return;

    state->palette = rand((uint8_t)LEDArtAnimation::LEDPalette_LAST);

    // Base unit is 8 bar loop. 120bpm, 4 beats = 2s = 1 bar. 8bars = 16s
    switch(rand(6)) {
        case 0: // 1 bar = 1/8 duration (fast)
            // state->speedFactor = 0.125;
            state->speedFactor = 0.5;
            break;

        case 1: // 4 bar = 1/2 duration 
            // state->speedFactor = 0.5;
            state->speedFactor = 0.75;
            break;

        case 2: // 8 bar = 1.0
            state->speedFactor = 1.0;
            break;

        case 3: // 16 bar = 2.0
            state->speedFactor = 2.0;
            break;

        case 4: // 32 bar = 4.0
            state->speedFactor = 4.0;
            break;
    }

    // speedFactor = 1.0;
    // state->foreground = LAColor(HslColor(((float)rand(1000))/1000.0, 0.6, 0.5));
    // state->background = LAColor(HslColor(((float)rand(1000))/1000.0, 0.3, 0.2));
    state->foreground = LAColor(HslColor(((float)rand(1000))/1000.0, 1.0, (((float)rand(1000))/2000.0)+0.2));
    state->background = LAColor(HslColor(((float)rand(1000))/1000.0, 1.0, 0.2));

    // To convert to RGBW...
    convertLAColor(state->foreground);
    convertLAColor(state->background);

    // Even steven on reversie
    state->reverse = rand(10) < 5;

    // Don't want to randomize this but we need to pass through this
    // function with the current values
    state->maxBrightness = maxBrightness;

    ///////////////////
    // DEBUGGING ANIMATIONS
    // state->palette = 2;
    // state->speedFactor = 1.0;
    // state->foreground = LAColor(255, 0, 0);
    // state->background = LAColor(0, 0, 255);
    // state->reverse = false;
}

void
Nexus::applyState(NexusState* state)
{
    Log.printf("NEXUS: applyState state->mb=%d\n", state->maxBrightness);

    palette = (LEDArtAnimation::LEDPaletteType)state->palette;
    speedFactor = state->speedFactor;
    reverse = state->reverse;

    foreground = state->foreground;
    background = state->background;
    maxBrightness = state->maxBrightness;

    geomRotated = state->geomRotated;

    // TODO: Update listeners????
}

void
Nexus::gatherState(NexusState* state)
{
    state->palette = palette;
    state->speedFactor = speedFactor;
    state->reverse = reverse;

    state->foreground = foreground;
    state->background = background;
    state->maxBrightness = maxBrightness;
    Log.printf("NEXUS: mb=%f state->mb=%d\n", maxBrightness, state->maxBrightness);

    state->geomRotated = geomRotated;
}


void
Nexus::logName(uint16_t len, char* sz)
{
    Log.printf(" len=%d sz=", len);
    if (sz)
    {
        Log.printf("%s", sz);
    }
    else
    {
        Log.printf("NULL");
    }        
}

#define BOOL_STR(x) (x ? "T" : "F")
void
Nexus::logState(NexusState* state, char* szGeomName, char* szAnimName)
{
#if LAColor == RGBWColor
    Log.printf("NEXUS STATE: p=%d, sf=%f, fg=(%d,%d,%d,%d) bg=(%d,%d,%d,%d) rev=%s geomRot=%s mb=%d", 
        state->palette, state->speedFactor, 
        state->foreground.R, state->foreground.G, state->foreground.B, state->foreground.W, 
        state->background.R, state->background.G, state->background.B, state->background.W,
        BOOL_STR(state->reverse),
        BOOL_STR(state->geomRotated),
        state->maxBrightness
        );
#else
    Log.printf("NEXUS STATE: p=%d, sf=%f, fg=(%d,%d,%d) bg=(%d,%d,%d) rev=%s geomRot=%s mb=%d", 
        state->palette, state->speedFactor, 
        state->foreground.R, state->foreground.G, state->foreground.B, 
        state->background.R, state->background.G, state->background.B,
        BOOL_STR(state->reverse),
        BOOL_STR(state->geomRotated),
        state->maxBrightness
        );
#endif

    logName(state->geomNameLen, szGeomName);
    logName(state->animNameLen, szAnimName);
    Log.printf("\n");
}

uint16_t
Nexus::serializeState(NexusState* ns, char *szGeomName, char* szAnimName, uint8_t* into)
{
    if (!ns) return 0;

    if (!szGeomName)
    {
        ns->geomNameLen = 0;
    }
    else
    {
        ns->geomNameLen = strlen(szGeomName);
    }

    if (!szAnimName)
    {
        ns->animNameLen = 0;
    }
    else
    {
        ns->animNameLen = strlen(szAnimName);
    }

    uint8_t* cursor = into;

    memcpy(cursor, (uint8_t*)ns, sizeof(NexusState));
    cursor += sizeof(NexusState);

    memcpy(cursor, szGeomName, ns->geomNameLen);
    cursor += ns->geomNameLen;

    memcpy(cursor, szAnimName, ns->animNameLen);
    cursor += ns->animNameLen;

    return cursor - into;
}

int16_t deserializeName(char* which, char** dest, uint8_t nameLen, uint16_t length, const uint8_t* from)
{
    if (!dest)
    {
        Log.printf("NEXUS: No output %s name pointer, skipping it\n", which);
        // Just cause we don't want it, we can still pretend to skip it 
        return nameLen;
    }

    if (nameLen == 0)
    {
        *dest = NULL;
        return 0;
    }

    // Log.printf("     : length=%d toCopy=%d ns->nameLen=%d\n", length, toCopy, ns->nameLen);
    if (length < nameLen)
    {
        Log.printf("ERROR: Not enough bytes to copy a %s name. Want %d, Have %d\n", which, nameLen, length);
        return -1;
    }

    // Allocate a string
    *dest = (char*)malloc(nameLen + 1);
    if (!*dest)
    {
        // No use logging OOM - things are about to splode...
        return -1;
    }

    memcpy(*dest, from, nameLen);
    char* term = *dest + nameLen;
    *term = 0; // Null terminate please!

    // How much we actually copied
    return nameLen;
}

bool
Nexus::deserializeState(NexusState* ns, char** pszGeomName, char** pszAnimName, uint16_t length, const uint8_t* from)
{
    uint16_t toCopy = sizeof(NexusState);
    if (length < toCopy)
    {
        Log.printf("ERROR: Not enough bytes to be a NexusState\n");
        return false;
    }

    memcpy((uint8_t*)ns, from, toCopy);

    int16_t copied = toCopy;
    copied = deserializeName("geom", pszGeomName, ns->geomNameLen, length - copied, from + copied);
    if (copied<0)
    { 
        return false;
    }
    copied += toCopy;

    copied = deserializeName("anim", pszAnimName, ns->animNameLen, length - copied, from + copied);
    if (copied<0)
    { 
        return false;
    }

    return true;
}

void
Nexus::testSerializer()
{
    uint8_t buf[512];
    uint16_t len;
    NexusState st;
    memset(&st, 0, sizeof(NexusState));

    char* szGeomNameIn = "BlergInsteim";
    char* szGeomNameOut = NULL;
    char* szAnimNameIn = "FJjowef";
    char* szAnimNameOut = NULL;

    randomizeState(&st);
    len = serializeState(&st, szGeomNameIn, szAnimNameIn, buf);
    logState(&st, szGeomNameIn, szAnimNameIn);

    Log.printf("NEXUS: Serialized length = %d\n", len);

    memset(&st, 0, sizeof(NexusState));
    logState(&st, szGeomNameOut, szAnimNameOut);


    Log.printf("NEXUS: Attempting to deserialize...\n");
    if (deserializeState(&st, &szGeomNameOut, &szAnimNameOut, len, buf))
    {
        logState(&st, szAnimNameOut, szGeomNameOut);
    }
    else
    {
        Log.printf("NEXUS: Oops - failed\n");
    }

    if (szGeomNameOut)
    {
        free(szGeomNameOut);
    }
    if (szAnimNameOut)
    {
        free(szAnimNameOut);
    }
    Log.printf("-------- test done\n");
}
