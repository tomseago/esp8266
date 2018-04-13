#include "vest.h"

// // all vest strips are 12
// // front inside is 5 above front outside

// // left back 0-12
// // left front inside 13-24
// // left front outside 25-36
// // right back 37-48
// // right front inside 49-60
// // right front outside 61-72

// // Skirt
// // Grid 6x16 starts on left front and wraps around back to right front
// // 96 active leds (4 extra at end of strand

// // 168 total

// VestGeometry::VestGeometry(uint16_t width, uint16_t height, uint16_t vestX, uint16_t vestY) :
//     LEDArtGeometry(width, height),
//     vestTopo(vestX, vestY),
//     skirtTopo(width, height-vestY),
//     vestX(vestX),
//     vestY(vestY),
//     vestXDiv(width / vestX),
//     vestSize(vestX * vestY)
//     // LEDArtGeometry((3 * width)-2, height),
//     // topo((3 * width)-2, height)
// {

// }

// uint16_t 
// VestGeometry::Map(uint16_t x, uint16_t y) {

//     if (y < vestY) {
//         // In the vest part so use that topo
//         uint16_t xM = x / vestXDiv;
//         if (x==width-1) {
//             xM = vestX - 1;
//         }
//         return vestTopo.Map(xM, y);
//     } else {
//         // In the skirt
//         uint16_t yS = y - vestY;
//         return skirtTopo.Map(x,yS) + (vestX * vestY);
//     }


//     // // return topo.Map(x,y);
//     // uint16_t mapped = topo.Map(x,y);
//     // uint16_t out = mapped / 3;
//     // if (mapped == out * 3) {
//     //     // Ok to use this pixel
//     //     return out;
//     // }

//     // // Not okay, send an out of range value
//     // return width * height;
// }