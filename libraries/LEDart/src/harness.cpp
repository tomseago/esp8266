#include "harness.h"

uint16_t 
HarnessGeometry::map(uint16_t x, uint16_t y) 
{
    // The harness is simple so we just hard code this
    if (y == 0)
    {        
        return (x < centerX) ? x : (width - x + centerX - 1);
    }
    else
    {
        // Cut off 1 pixel at the beginning and at the end
        if (x==0 || x==width-1)
        {
            // invalid
            return width * height;
        }
        else
        {
            return (width * (y+1)) -
                ( (x < centerX) ? (centerX - x + 2) : (x + 2) );
        }
    }
}

const uint8_t planesZ[][4] = {
    { 14, 57, 43, 29 },
    { 13, 56, 42, 28 },
    { 12, 55, 41, 27 },
    { 11, 54, 40, 26 },
    { 10, 53, 39, 25 },
    {  9, 52, 38, 24 },
    {  8, 51, 37, 23 },
    {  7, 50, 36, 22 },
    {  6, 49, 35, 21 },
    {  5, 48, 34, 20 },
    {  4, 47, 33, 19 },
    {  3, 46, 32, 18 },
    {  2, 45, 31, 17 },
    {  1, 44, 30, 16 },
    {  0, 99, 99, 15 },
};


uint16_t 
HarnessZGeometry::map(uint16_t x, uint16_t y) 
{
    return planesZ[x][y];
}
