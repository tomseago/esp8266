#pragma once

#include <Arduino.h>

#include "LEDGeometry.h"

#define SFTopLeft 9
#define SFBotLeft 43
#define SFBotRight 59
#define SFTopRight 93
#define SFMax 100

class SFGeomPanelEdges : public LEDArtGeometry {
public:
    SFGeomPanelEdges(char* szName) :
        LEDArtGeometry(szName, false)
    {        
    }

    virtual uint16_t map(uint16_t x, uint16_t y) 
    { 
        switch(y)
        {
        case 0:
            // Top Edge, consists of 9 to the left and 7
            // on the right side that are actually the end of the strip
            // Start in back corner
            if (x < 7)
            {
                return SFTopRight + x;
            }
            else
            {
                return x - 7;
            }

        case 1:
            // Left Edge
            return SFTopLeft + x;

        case 2:
            // Bottom Edge, map in a circle
            return SFBotLeft + x;

        case 3:
            // Right Edge
            return SFBotRight + x;
        }
    }

    virtual uint16_t getWidth(int16_t y=-1) 
    { 
        switch(y)
        {
        case 0:
        case 2:
            return 16; // Short sides

        case 1:
        case 3:
            return 34;

        default:
            return 1;
        }
    }

    virtual uint16_t getHeight() { return 4; }

};

class SFGeomSideGrid : public LEDArtGeometry {
public:
    SFGeomSideGrid(char* szName) :
        LEDArtGeometry(szName, false)
    {        
    }

    virtual uint16_t map(uint16_t x, uint16_t y);


    virtual uint16_t getWidth(int16_t y=-1);

    virtual uint16_t getHeight() { return 36; }

};


class SFGeomSideMirrored : public LEDArtGeometry {
public:
    SFGeomSideMirrored(char* szName) :
        LEDArtGeometry(szName, false)
    {        
    }

    virtual uint16_t map(uint16_t x, uint16_t y);


    virtual uint16_t getWidth(int16_t y=-1);

    virtual uint16_t getHeight() { return 18 + 36; }

};

class SFGeomWrappedOver : public LEDArtGeometry {
public:
    SFGeomWrappedOver(char* szName) :
        LEDArtGeometry(szName, false)
    {        
    }

    virtual uint16_t map(uint16_t x, uint16_t y);


    virtual uint16_t getWidth(int16_t y=-1);

    virtual uint16_t getHeight() { return 36 * 3; }

};

class SFGeomWrappedLong : public LEDArtGeometry {
public:
    SFGeomWrappedLong(char* szName) :
        LEDArtGeometry(szName, false)
    {        
    }

    virtual uint16_t map(uint16_t x, uint16_t y);


    virtual uint16_t getWidth(int16_t y=-1);

    virtual uint16_t getHeight() { return 18 * 4; }

};