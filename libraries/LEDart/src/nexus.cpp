#include "nexus.h"
#include "rand.h"
#include "log.h"

Nexus::Nexus(bool forceSpecificGeometry) :
    _forceSpecificGeometry(forceSpecificGeometry)
{

}

void
Nexus::addAnimation(char *szName)
{
    char* szNameCpy = strdup(szName);
    animNames.push_back(szNameCpy);
}

uint8_t
Nexus::numAnimations()
{
    return animNames.size();
}

char *
Nexus::animName(uint8_t index)
{
    return animNames.at(index);
}

void
Nexus::addListener(NexusListener* listener)
{
    if (!listener) return;

    listeners.push_back(listener);
}

void 
Nexus::checkUnitType()
{
    updateUnitTypeVal(&unitType);
}

void
Nexus::updateUnitTypeVal(uint8_t* val)
{
    if (!_forceSpecificGeometry) return;

    if (*val == 2 || *val == 3) {
        *val += 2;
    }
}

void
Nexus::randomizeAll(uint32_t source)
{
    NexusState toApply;
    randomizeState(&toApply);
    applyState(&toApply);
}

void
Nexus::nextUnitType(uint32_t source)
{
    unitType++;
    if (unitType >= 6 ) {
        unitType = 0;
    }
    checkUnitType();

    Log.printf("NX: nextUnitType now=%d\n",unitType);
}

void
Nexus::nextPalette(uint32_t source)
{
    uint8_t nextPalette = (uint8_t)palette + 1;
    palette = (LEDArtAnimation::LEDPaletteType)nextPalette;
    if (palette == LEDArtAnimation::LEDPalette_LAST) {
        palette = (LEDArtAnimation::LEDPaletteType)0;
    }

    Log.printf("NX: nextPalette now=%d\n",(uint8_t)palette);
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
Nexus::sendUserAnimationRequest(char* szName, bool randomize, uint32_t source)
{
    for(NexusListener* listener : listeners) 
    {
        listener->nexusUserAnimationRequest(szName, randomize, source);  
    } 
}

//////

void 
Nexus::prepareRandomStateFor(uint32_t when, char* szAnimName, uint32_t source)
{
    clearPreparedState();

    randomizeState(&nextState);
    nextState.time = when;

    if (szAnimName) 
    {
        nextState.nameLen = strlen(szAnimName);
        if (nextState.nameLen)
        {
            szNextAnim = strdup(szAnimName);
            if (!szNextAnim) {
                nextState.nameLen = 0;
            }
        }
        // else nothing to dup
    }

    sendValueUpdate(NexusListener::NexusValueType::PreparedState, source);
}

uint32_t
Nexus::nextPreparedState(char **pszAnimName)
{
    if (!nextState.time)
    {
        if (pszAnimName)
        {
            *pszAnimName = NULL;
        }
        return 0;
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

    if (szNextAnim)
    {
        free(szNextAnim);
    }
    szNextAnim = NULL;

    // testSerializer();
}

uint16_t
Nexus::serializePreparedState(uint8_t* into)
{
    if (!nextState.time)
    {
        return 0;
    }

    return serializeState(&nextState, szNextAnim, into);
}

bool
Nexus::deserializePreparedState(uint16_t length, const uint8_t* from)
{
    if (szNextAnim)
    {
        free(szNextAnim);
        szNextAnim = NULL;
    }

    deserializeState(&nextState, &szNextAnim, length, from);
}


void
Nexus::randomizeState(NexusState* state)
{
    if (!state) return;

    state->unitType = rand(5);
    updateUnitTypeVal(&(state->unitType));

    state->palette =rand((uint8_t)LEDArtAnimation::LEDPalette_LAST);

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
}

void
Nexus::applyState(NexusState* state)
{
    unitType = state->unitType;
    palette = (LEDArtAnimation::LEDPaletteType)state->palette;
    speedFactor = state->speedFactor;

    foreground = state->foreground;
    background = state->background;

    reverse = state->reverse;

    // TODO: Update listeners????
}

void
Nexus::logState(NexusState* state, char* szName)
{
    Log.printf("NEXUS STATE: ut=%d p=%d, sf=%f, fg=(%d,%d,%d) bg=(%d,%d,%d)", state->unitType, state->palette, state->speedFactor, state->foreground.R, state->foreground.G, state->foreground.B, state->background.R, state->background.G, state->background.B);
    Log.printf(" len=%d szName=", state->nameLen);
    if (szName)
    {
        Log.printf("%s", szName);
    }
    else
    {
        Log.printf("NULL");
    }        
    Log.printf("\n");
}

uint16_t
Nexus::serializeState(NexusState* ns, char* szName, uint8_t* into)
{
    if (!ns) return 0;

    if (!szName)
    {
        ns->nameLen = 0;
    }
    else
    {
        ns->nameLen = strlen(szName);
    }

    memcpy(into, (uint8_t*)ns, sizeof(NexusState));
    memcpy(into+sizeof(NexusState), szName, ns->nameLen);

    return sizeof(NexusState) + ns->nameLen;
}

bool
Nexus::deserializeState(NexusState* ns, char** pszName, uint16_t length, const uint8_t* from)
{
    uint16_t toCopy = sizeof(NexusState);
    if (length < toCopy)
    {
        Log.printf("ERROR: Not enough bytes to be a NexusState\n");
        return false;
    }

    memcpy((uint8_t*)ns, from, toCopy);

    if (!pszName)
    {
        Log.printf("NEXUS: No output name pointer, skipping it\n");
        return true;
    }

    if (ns->nameLen == 0)
    {
        *pszName = NULL;
        return true;
    }

    // Log.printf("     : length=%d toCopy=%d ns->nameLen=%d\n", length, toCopy, ns->nameLen);
    if (length - toCopy < ns->nameLen)
    {
        Log.printf("ERROR: Not enough bytes to copy an anim name\n");
        return false;
    }

    // Allocate a string
    *pszName = (char*)malloc(ns->nameLen + 1);
    if (!*pszName)
    {
        return false;
    }

    memcpy(*pszName, from + toCopy, ns->nameLen);
    char* term = *pszName + ns->nameLen;
    *term = 0; // Null terminate please!
    return true;
}

void
Nexus::testSerializer()
{
    uint8_t buf[512];
    uint16_t len;
    NexusState st;
    memset(&st, 0, sizeof(NexusState));

    char* szNameIn = "BlergInsteim";
    char* szNameOut = NULL;

    randomizeState(&st);
    len = serializeState(&st, szNameIn, buf);
    logState(&st, szNameIn);

    Log.printf("NEXUS: Serialized length = %d\n", len);

    memset(&st, 0, sizeof(NexusState));
    logState(&st, szNameOut);


    Log.printf("NEXUS: Attempting to deserialize...\n");
    if (deserializeState(&st, &szNameOut, len, buf))
    {
        logState(&st, szNameOut);
    }
    else
    {
        Log.printf("NEXUS: Oops - failed\n");
    }

    if (szNameOut)
    {
        free(szNameOut);
    }
    Log.printf("-------- test done\n");
}
