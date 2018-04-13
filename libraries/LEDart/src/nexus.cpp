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
Nexus::prepareRandomStateFor(uint32_t when, char* szGeomName, bool rotated, char* szAnimName, uint32_t source)
{
    clearPreparedState();

    randomizeState(&nextState);
    nextState.geomRotated = rotated;
    nextState.time = when;

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

    // testSerializer();
}

uint16_t
Nexus::serializePreparedState(uint8_t* into)
{
    if (!nextState.time)
    {
        return 0;
    }

    return serializeState(&nextState, szNextGeom, szNextAnim, into);
}

bool
Nexus::deserializePreparedState(uint16_t length, const uint8_t* from)
{
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

    deserializeState(&nextState, &szNextGeom, &szNextAnim, length, from);
}


void
Nexus::randomizeState(NexusState* state)
{
    if (!state) return;

    state->palette = rand((uint8_t)LEDArtAnimation::LEDPalette_LAST);

    // Base unit is 8 bar loop. 120bpm, 4 beats = 2s = 1 bar. 8bars = 16s
    switch(rand(6)) {
        case 0: // 1 bar = 1/8 speed
            state->speedFactor = 0.125;
            break;

        case 1: // 4 bar = 1/2 speed
            state->speedFactor = 0.5;
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
    state->foreground = RgbColor(HslColor(((float)rand(1000))/1000.0, 0.6, 0.5));
    state->background = RgbColor(HslColor(((float)rand(1000))/1000.0, 0.3, 0.2));

    // Even steven on reversie
    state->reverse = rand(10) < 5;

    ///////////////////
    // DEBUGGING ANIMATIONS
    // state->palette = 2;
    // state->speedFactor = 1.0;
    // state->foreground = RgbColor(255, 0, 0);
    // state->background = RgbColor(0, 0, 255);
    // state->reverse = false;
}

void
Nexus::applyState(NexusState* state)
{
    palette = (LEDArtAnimation::LEDPaletteType)state->palette;
    speedFactor = state->speedFactor;

    foreground = state->foreground;
    background = state->background;

    reverse = state->reverse;

    // TODO: Update listeners????
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
    Log.printf("\n");

}

void
Nexus::logState(NexusState* state, char* szGeomName, char* szAnimName)
{
    Log.printf("NEXUS STATE: p=%d, sf=%f, fg=(%d,%d,%d) bg=(%d,%d,%d)", state->palette, state->speedFactor, state->foreground.R, state->foreground.G, state->foreground.B, state->background.R, state->background.G, state->background.B);

    logName(state->geomNameLen, szGeomName);
    logName(state->animNameLen, szAnimName);
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
