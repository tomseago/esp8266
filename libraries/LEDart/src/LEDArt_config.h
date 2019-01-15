#pragma once

#ifndef LEDART_COLOR_FEATURE

// strips
// #define LEDART_COLOR_FEATURE  NeoGrbFeature
// bullets
// #define LEDART_COLOR_FEATURE  NeoRgbFeature  

#define LEDART_RGBW true

// Infinity Bed
#define LEDART_COLOR_FEATURE  NeoGrbwFeature  


#if LEDART_RGBW
#define LAColor RgbwColor
#else
#define LAColor RgbColor
#endif



// Both???
// #define LEDART_COLOR_FEATURE NeoGrbRgbFeature
#endif

#ifndef LEDART_METHOD
#define LEDART_METHOD         Neo800KbpsMethod
#endif

#ifndef LEDART_TOPO_LAYOUT
#define LEDART_TOPO_LAYOUT    ColumnMajor270Layout
#endif

