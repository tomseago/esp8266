#include "pringles.h"

PringlesGeometry::PringlesGeometry(uint16_t width, uint16_t height) :
    LEDArtGeometry((3*width)+1, height),
    topo((3*width)+1, height)
    // LEDArtGeometry((3 * width)-2, height),
    // topo((3 * width)-2, height)
{

}

uint16_t 
PringlesGeometry::Map(uint16_t x, uint16_t y) {
    // return topo.Map(x,y);
    uint16_t mapped = topo.Map(x,y);
    uint16_t out = mapped / 3;
    if (mapped == out * 3) {
        // Ok to use this pixel
        return out;
    }

    // Not okay, send an out of range value
    return width * height;
}