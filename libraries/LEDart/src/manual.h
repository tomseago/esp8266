#pragma once

#include <LEDArt.h>

// class HarnessGeometry : public LEDArtGeometry {
//     const uint16_t centerX = 15;
//     const uint16_t width = 30;
//     const uint16_t height = 2;

// public:
//     HarnessGeometry() :
//         LEDArtGeometry("Basic", true)
//     {

//     }
//     virtual uint16_t map(uint16_t x, uint16_t y);

//     virtual uint16_t getWidth(int16_t rowIx=-1) { return width; }
//     virtual uint16_t getHeight() { return height; }
// };


// class HarnessZGeometry : public LEDArtGeometry {

// public:
//     HarnessZGeometry() :
//         LEDArtGeometry("PlanesZ", true)
//     {

//     }
//     virtual uint16_t map(uint16_t x, uint16_t y);

//     virtual uint16_t getWidth(int16_t rowIx=-1) { return 15; }
//     virtual uint16_t getHeight() {return 4;}
// };