#pragma once

#include "LEDArt_config.h"
#include <NeoPixelBrightnessBus.h>

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

/////////

class LEDArtTableGeometry : public LEDArtGeometry {
protected:
    uint16_t rowCount;
    uint16_t colCount;
    uint16_t** table;

public:
    LEDArtTableGeometry(char *szName, uint16_t rowCount, uint16_t colCount, uint16_t** table, bool canRotate = true) : 
        LEDArtGeometry(szName, canRotate),
        rowCount(rowCount),
        colCount(colCount),
        table(table)
    {
    }

    virtual uint16_t map(uint16_t x, uint16_t y) { return table[y][x]; }

    virtual uint16_t getWidth(int16_t rowIx=-1) { return colCount; }
    virtual uint16_t getHeight() { return rowCount; }
};

