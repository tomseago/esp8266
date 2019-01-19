#include "circles.h"
#include "tm_common.h"

// We have two colorspaces remember...

#define RED RgbColor(255,0,0)
#define GREEN RgbColor(0,255,0)
#define BLUE RgbColor(0,0,255)
// const LAColor red(255,0,0);
// const LAColor yellow(255,255,0);
// const LAColor green(0,255,0);
// const LAColor cyan(0,255,255);
// const LAColor blue(0,0,255);
// const LAColor purple(255,0,255);

// const LAColor white(255,255,255);
// const LAColor black(0,0,0);


// Num pixels is 24 per digit and 9 digits so 216
#define CIRCLES_PIN 15

Circles::Circles() :
    bus(216,CIRCLES_PIN)
{

}

void
Circles::begin()
{
    bus.Begin();
    bus.SetBrightness(32);
    bus.ClearTo(RED);
    bus.Show();
}

void
Circles::loop()
{

}

void
Circles::setText(char* txt)
{

}

const uint32_t font[] = {
    0x0, //  
    0x40041, // !
    0x60c, // "
    0xf3efbc, // #
    0x3bef8e, // $
    0xc11430, // %
    0xff9f80, // &
    0xc, // '
    0xf80, // (
    0x3e, // )
    0x2a9, // *
    0x411f1, // +
    0x38000, // ,
    0x803000, // -
    0x10000, // .
    0x101044, // /
    0x7fcf1f, // 0
    0x1c01c, // 1
    0xfe183f, // 2
    0xff87f, // 3
    0x1e7fc, // 4
    0xfefc3, // 5
    0xffff03, // 6
    0x1cc1f, // 7
    0xffffff, // 8
    0x1efff, // 9
    0x200200, // :
    0x38008, // ;
    0x741000, // <
    0xc071f0, // =
    0x5d000, // >
    0x41e7f, // ?
    0x7f8f7f, // @
    0xf1ff1f, // A
    0xffffdf, // B
    0x7f0f07, // C
    0x5f8d0f, // D
    0xfe1fc3, // E
    0xf01fc7, // F
    0x7fef03, // G
    0xf1f7fc, // H
    0xe1843, // I
    0x1fc01c, // J
    0xf0f7f0, // K
    0x7f0700, // L
    0x71c7fc, // M
    0x71f79c, // N
    0x7fcf1f, // O
    0x700fff, // P
    0x67cf1f, // Q
    0x707fff, // R
    0x3fff8f, // S
    0x41c47, // T
    0x7fc71c, // U
    0x44471c, // V
    0x6ad71c, // W
    0x71f7dc, // X
    0x417fc, // Y
    0xfa183e, // Z
};