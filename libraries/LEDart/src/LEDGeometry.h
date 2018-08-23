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

    virtual uint16_t getWidth(int16_t rowIx=-1) = 0; // The default secondary
    virtual uint16_t getHeight() = 0; // The default primary
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

    virtual uint16_t map(uint16_t x, uint16_t y) { return y; }

    virtual uint16_t getWidth(int16_t rowIx=-1) { return 1; }
    virtual uint16_t getHeight() { return pixelCount; }
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
    uint16_t* table;

public:
    LEDArtTableGeometry(char *szName, uint16_t rowCount, uint16_t colCount, uint16_t* table, bool canRotate = true) : 
        LEDArtGeometry(szName, canRotate),
        rowCount(rowCount),
        colCount(colCount),
        table(table)
    {
    }

    virtual uint16_t map(uint16_t x, uint16_t y) { return *(table + (colCount * y) + x); }

    virtual uint16_t getWidth(int16_t rowIx=-1) { return colCount; }
    virtual uint16_t getHeight() { return rowCount; }
};

// This is like the table geometry but adds the concept of having a varying number
// of columns within each row.
class LEDArtRowGeometry : public LEDArtGeometry {
protected:
    uint16_t rowCount;
    uint16_t* colCounts;
    uint16_t** rows;

public:
    LEDArtRowGeometry(char *szName, uint16_t rowCount, uint16_t** rows, uint16_t* colCounts) : 
        LEDArtGeometry(szName, false),
        rowCount(rowCount),
        rows(rows),
        colCounts(colCounts)
    {
    }

    virtual uint16_t map(uint16_t x, uint16_t y) 
    { 
        uint16_t* row = rows[y];
        return row[x];
    }

    virtual uint16_t getWidth(int16_t rowIx=-1) 
    {
        if (rowIx<0) return 1;

        return colCounts[rowIx];
    }

    virtual uint16_t getHeight() { return rowCount; }
};

