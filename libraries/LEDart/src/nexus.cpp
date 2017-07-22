#include "nexus.h"
#include "rand.h"

Nexus::Nexus(int i) 
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
Nexus::randomizeAll(uint32_t source)
{
    unitType = rand(5);

    palette = (LEDArtAnimation::LEDPaletteType)rand((uint8_t)LEDArtAnimation::LEDPalette_LAST);

    // Base unit is 8 bar loop. 120bpm, 4 beats = 2s = 1 bar. 8bars = 16s
    switch(rand(6)) {
        case 0: // 1 bar = 1/8 speed
            speedFactor = 0.125;
            break;

        case 1: // 4 bar = 1/2 speed
            speedFactor = 0.5;
            break;

        case 2: // 8 bar = 1.0
            speedFactor = 1.0;
            break;

        case 3: // 16 bar = 2.0
            speedFactor = 2.0;
            break;

        case 4: // 32 bar = 4.0
            speedFactor = 4.0;
            break;
    }

    // speedFactor = 1.0;
    foreground = RgbColor(HslColor(((float)rand(1000))/1000.0, 0.6, 0.5));
    background = RgbColor(HslColor(((float)rand(1000))/1000.0, 0.3, 0.2));

    // Even steven on reversie
    reverse = rand(10) < 5;

}

void
Nexus::nextUnitType(uint32_t source)
{
    unitType++;
    if (unitType >= 6 ) {
        unitType = 0;
    }
}

void
Nexus::nextPalette(uint32_t source)
{
    uint8_t nextPalette = (uint8_t)palette + 1;
    palette = (LEDArtAnimation::LEDPaletteType)nextPalette;
    if (palette == LEDArtAnimation::LEDPalette_LAST) {
        palette = (LEDArtAnimation::LEDPaletteType)0;
    }
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
Nexus::sendUserAnimationRequest(char* szName, uint32_t source)
{
    for(NexusListener* listener : listeners) 
    {
        listener->nexusUserAnimationRequest(szName, source);  
    } 
}

