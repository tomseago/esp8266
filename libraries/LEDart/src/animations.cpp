#include "animations.h"
#include "rand.h"
#include "nexus.h"

const LAColor red(255,0,0);
const LAColor yellow(255,255,0);
const LAColor green(0,255,0);
const LAColor cyan(0,255,255);
const LAColor blue(0,0,255);
const LAColor purple(255,0,255);

const LAColor white(255,255,255);
const LAColor black(0,0,0);


LAA_Flood::LAA_Flood(char* szName, LAColor background) : 
    LEDArtAnimation(szName),
    background(background),
    foreground(background)
    // background(0,255,255),
    // foreground(255,0,0)
{

}

void
LAA_Flood::animate(LEDArtPiece& piece, LEDAnimationParam p) {
    if (p.state == LEDAnimationState_Started) {
        background = foreground;

        foreground = LAColor(HslColor(rand(255)/255.0, 0.6, 0.5));
    }

    uint16_t floodedDistance = piece.strip.PixelCount() * p.progress;

    clearTo(piece, foreground, 0, floodedDistance);
    clearTo(piece, background, floodedDistance+1, piece.strip.PixelCount());
    // piece.strip.ClearTo(foreground, 0, floodedDistance);
    // piece.strip.ClearTo(background, floodedDistance+1, piece.strip.PixelCount());
}

/////////

LAA_Sparkle::LAA_Sparkle(char* szName, uint16_t pixelCount) : 
    LEDArtAnimation(szName), pixelCount(pixelCount)
{
    loopDuration = 50;
    // maxDuration = 8000;

    for(int i=0; i<3; i++) {
        ppGenerations[i] = new bool[pixelCount];

        for(int j=0; j<pixelCount; j++) {
            ppGenerations[i][j] = pixelCount + 1;
        }
    }
}

const float SPARKLE_L_MAX = 0.5;
const float SPARKLE_L_1 = 0.2;
const float SPARKLE_L_2 = 0.1;

void
LAA_Sparkle::animate(LEDArtPiece& piece, LEDAnimationParam p) {
    if (p.state == LEDAnimationState_Started) {
        for(int i=0; i<2; i++) {
            memcpy(ppGenerations[i], ppGenerations[i+1], sizeof(bool) * pixelCount);
        }

        // Fill in the last generation
        bool *lastGen = ppGenerations[2];
        bool flash = rand(1000) < 50;
        for(int pix=0; pix<pixelCount; pix++) {
            lastGen[pix] = flash || (rand(1000) < 100);
        }
    }

    // Loop through all pixels giving them an appropriate lightness based
    // on what generation they are in
    HslColor black(0, 0, 0);
    HslColor color;
    for(int pix=0; pix<pixelCount; pix++) {
        if (ppGenerations[2][pix]) {
            // MAX to 1
            color = HslColor(0.0, 0.0, SPARKLE_L_MAX - (p.progress * (SPARKLE_L_MAX - SPARKLE_L_1)));
        } else if (ppGenerations[1][pix]) {
            // 1 to 2
            color = HslColor(0.0, 0.0, SPARKLE_L_1 - (p.progress * (SPARKLE_L_1 - SPARKLE_L_2)));
        } else if (ppGenerations[0][pix]) {
            // 2 to 0
            color = HslColor(0.0, 0.0, SPARKLE_L_2 - (p.progress * (SPARKLE_L_2)));
        } else {
            // no color
            color = black;
        }
        piece.strip.SetPixelColor(pix, color);
    }
}

/////////

LAA_Sparkle2::LAA_Sparkle2(char* szName, uint16_t pixelCount) : 
    LEDArtAnimation(szName), pixelCount(pixelCount)
{
    loopDuration = 1000;
    ignoreSpeedFactor = true;
    maxDuration = 8000;

    absStartProgress = new float[pixelCount];
    // for(int i=0; i<3; i++) {
    //     ppGenerations[i] = new bool[pixelCount];

    //     for(int j=0; j<pixelCount; j++) {
    //         ppGenerations[i][j] = pixelCount + 1;
    //     }
    // }
}

const float SPARKLE2_L_MAX = 0.5;
const float SPARKLE2_L_1 = 0.2;
const float SPARKLE2_L_2 = 0.1;

void
LAA_Sparkle2::animate(LEDArtPiece& piece, LEDAnimationParam p) {
    if (p.state == LEDAnimationState_Started) {
        loopBase += 1.0f;
    }

    // Normalize progress by removing the speedFactor from it

    float absProgress = loopBase + p.progress;

    HslColor black(0, 0, 0);
    HslColor color;
    // X pixels per second (because our normalized loop rate
    // is one per second)
    float pixelsAvailable = absProgress * 100.0f;

    int offset = rand(pixelCount);

    for(int ix=0; ix<pixelCount; ix++) {

        int pix = ix + offset;
        if (pix >= pixelCount) {
            pix -= pixelCount;
        }

        // These are the rules for pixels
        // 0-50ms Really bright

        float pixelAge = absProgress - absStartProgress[pix];

        if (pixelAge > 0.200) {
            // It has a chance to come back to life
            if (rand(1000) < 50) {
                // But have we exceeded the pixel rate?
                if (pixelsAvailable - pixelsUsed > 0) {
                    // Nope! do it!
                    absStartProgress[pix] = absProgress;
                    pixelAge = 0;

                    pixelsUsed += 1.0f;
                }
            }
        }

        if (pixelAge < 0.020) {
            // MAX to 1
            // color = HslColor(0.0, 0.0, SPARKLE2_L_MAX - (p.progress * (SPARKLE2_L_MAX - SPARKLE2_L_1)));
            color = HslColor(0.0, 0.0, SPARKLE2_L_MAX - (p.progress * (SPARKLE2_L_MAX - SPARKLE2_L_2)));
        // } else if (pixelAge < 0.010) {
        //     // 1 to 2
        //     color = HslColor(0.0, 0.0, SPARKLE2_L_1 - (p.progress * (SPARKLE2_L_1 - SPARKLE2_L_2)));
        // } else if (pixelAge < 0.020) {
        //     // 2 to 0
        //     color = HslColor(0.0, 0.0, SPARKLE2_L_2 - (p.progress * (SPARKLE2_L_2)));
        } else {
            // no color
            color = black;
        }
        piece.strip.SetPixelColor(pix, color);
    }
}


//////////

LAA_UnitMapper::LAA_UnitMapper(char* szName) :
    LEDArtAnimation(szName)
{

}

uint16_t
LAA_UnitMapper::numUnits(LEDArtPiece& piece) {
    return piece.geomPrimaryCount();
}


uint16_t 
LAA_UnitMapper::unitSize(LEDArtPiece& piece) {
    return piece.geomSecondaryCount();
}

void 
LAA_UnitMapper::setFullUnitColor(LEDArtPiece& piece, uint16_t unitIx, LAColor color)
{
    piece.setPrimaryColor(unitIx, color);
}

void 
LAA_UnitMapper::setUnitPixelColor(LEDArtPiece& piece, uint16_t unitIx, uint16_t pixelIx, LAColor color)
{
    piece.setSecondaryColorInPrimary(unitIx, pixelIx, color);
}

void 
LAA_UnitMapper::setAllUnitsPixelColor(LEDArtPiece& piece, uint16_t pixelIx, LAColor color)
{
    piece.setSecondaryColor(pixelIx, color);
}

///////////////////

LAA_Rainbow::LAA_Rainbow(char* szName) : 
    LAA_UnitMapper(szName)
{
    // loopDuration = 2000;
    // loops = false;

    // currentType = Unit_SpecificCols;
}

void
LAA_Rainbow::animate(LEDArtPiece& piece, LEDAnimationParam p) {

    // currentType = piece.nexus.unitType;

    uint16_t unitCount = numUnits(piece);
    for(uint16_t ix=0; ix<unitCount; ix++) {
        float prog = p.progress + ((float)ix / (float)unitCount);
        setFullUnitColor(piece, ix, colorInPalette(piece.nexus.palette, prog));
    }
    // uint16_t pixelCount = piece.strip.PixelCount();

    // for(int pix=0; pix<pixelCount; pix++) {
    //     // Serial.printf("pix=%d ", pix);
    //     float prog = p.progress + ((float)pix/(float)pixelCount);
    //     piece.strip.SetPixelColor(pix, rybRainbow(prog));
    // }

    // Serial.printf("-------------\n");
}

///////////////////

LAA_Line::LAA_Line(char* szName) : 
    LAA_UnitMapper(szName)
{
}

void
LAA_Line::animate(LEDArtPiece& piece, LEDAnimationParam p) {
    // currentType = piece.nexus.unitType;

    piece.strip.ClearTo(black);

    setFullUnitColor(piece, p.progress * numUnits(piece), piece.nexus.foreground);

}

///////////////////

LAA_BoxOutline::LAA_BoxOutline(char* szName) : 
    LAA_UnitMapper(szName)
{
    loopDuration = 200000;
}

void
LAA_BoxOutline::animate(LEDArtPiece& piece, LEDAnimationParam p) {

    piece.strip.ClearTo(black);


    uint16_t hPrimary = piece.geomPrimaryCount();
    for(uint16_t prim = 0; prim < hPrimary; prim++) 
    {

        if (prim == 0)
        {
            // Top 
            piece.setPrimaryColor(prim, green);
            continue;
        }
        if (prim == hPrimary - 1)
        {
            // Bottom
            piece.setPrimaryColor(prim, cyan);
            continue;
        }

        // A middle section, so we want it's end pieces

        // Rows may vary in length
        uint16_t rowLength = piece.geomSecondaryCount(prim);
        piece.setSecondaryColorInPrimary(prim, 0, red);
        piece.setSecondaryColorInPrimary(prim, 1, blue);
        piece.setSecondaryColorInPrimary(prim, rowLength-1, yellow);
    }
}


///////////////////

LAA_AllWhite::LAA_AllWhite(char* szName) : 
    LAA_UnitMapper(szName)
{
    maxDuration = 10000;
    brightness = 0.6;
}

void
LAA_AllWhite::animate(LEDArtPiece& piece, LEDAnimationParam p) {

    piece.strip.ClearTo(white);
}


///////////////////

LAA_HalfWhite::LAA_HalfWhite(char* szName) : 
    LAA_UnitMapper(szName)
{
    type = LEDAnimationType_OVERLAY;
    // currentType = Unit_SpecificCols;
}

void
LAA_HalfWhite::animate(LEDArtPiece& piece, LEDAnimationParam p) {

    uint16_t nUnits = numUnits(piece);

    for(int i=0; i< nUnits; i++) { 
        setFullUnitColor(piece, i, (i < nUnits / 2) ? white : black);
    }

}


///////////////////

// UnitFill
//
// At the start of a loop it determins an order for the primary units
// and then fills that order of the course of the loop using current
// foreground and background. This base animation will always use
// the "natural" ordering, and will respect the reverse setting.

LAA_UnitFill::LAA_UnitFill(char* szName) : 
    LAA_UnitMapper(szName),
    unitOrder(NULL)
{
    type = LEDAnimationType_BASE;
}

void
LAA_UnitFill::animate(LEDArtPiece& piece, LEDAnimationParam p) 
{
    // if (p.state == LEDAnimationState_Started)
    // {
    //     // Make sure the unit type isn't boring
    //     if (piece.nexus.unitType == Unit_Single)
    //     {
    //         piece.nexus.nextUnitType((uint32_t)this);
    //     }
    // }

    if (p.state == LEDAnimationState_Started || !unitOrder || calculatedFor != piece.geomId()) {

        calculateOrder(piece);
    }

    uint16_t nUnits = numUnits(piece);

    // Adding 1 to the number to flood gives us both a start of no units and an end of all
    // units. Think of the case of either 1 unit or of 2 units. These cases make it apparent
    // that we actually want 2 and 3 states respectively.
    uint16_t floodedDistance = (nUnits + 1) * p.progress;

    for(uint16_t i=0; i< nUnits; i++) { 
        setFullUnitColor(piece, unitOrder[i], (i < floodedDistance) ? piece.nexus.foreground : piece.nexus.background);
    }
}

bool
LAA_UnitFill::canUseGeom(uint16_t width, uint16_t height)
{ 
    return true;
    // If you want it to be more picky for single strip scenarios
    // return height > width; 
}

void
LAA_UnitFill::calculateOrder(LEDArtPiece& piece) 
{
    // For the moment the calculated order is identical to the native order
    if (unitOrder) {
        delete unitOrder;
    }

    uint16_t nUnits = numUnits(piece);
    unitOrder = new uint16_t[nUnits];
    for(uint16_t unitIx=0; unitIx < nUnits; unitIx++)
    {
        // This base implementation always just respects the natural order
        uint16_t target = unitIx;
        if (piece.nexus.reverse)
        {
            target = nUnits - 1 - unitIx;
        }
        unitOrder[target] = unitIx;
    }

    calculatedFor = piece.geomId();
}

// RandoFill, an extension of UnitFill
//
// Overrides the calculateOrder function to determine a random order
// instead of using the natural order.

LAA_RandoFill::LAA_RandoFill(char* szName) : 
    LAA_UnitFill(szName)
{
}

void
LAA_RandoFill::calculateOrder(LEDArtPiece& piece) 
{

    // For the moment the calculated order is identical to the native order
    if (unitOrder) {
        delete unitOrder;
    }

    uint16_t nUnits = numUnits(piece);
    unitOrder = new uint16_t[nUnits];
    memset(unitOrder, 0xff, nUnits * sizeof(uint16_t));
    for(uint16_t unitIx=0; unitIx < nUnits; unitIx++)
    {
        // Pick a random location for it
        int target = rand(nUnits);
        while(unitOrder[target] != 0xffff)
        {
            target++;
            if (target == nUnits) target = 0;
        }
        unitOrder[target] = unitIx;
    }

    calculatedFor = piece.geomId();
}


// PaletteFill, an extension of UnitFill
//
// Instead of simply filling with foreground, it fills with the palette
// over a black background

LAA_PaletteFill::LAA_PaletteFill(char* szName) : 
    LAA_UnitFill(szName)
{
}

void
LAA_PaletteFill::animate(LEDArtPiece& piece, LEDAnimationParam p) 
{
    // if (p.state == LEDAnimationState_Started)
    // {
    //     // Make sure the unit type isn't boring
    //     if (piece.nexus.unitType == Unit_Single)
    //     {
    //         piece.nexus.nextUnitType((uint32_t)this);
    //     }
    // }

    if (p.state == LEDAnimationState_Started || !unitOrder || calculatedFor != piece.geomId()) {

        calculateOrder(piece);
    }

    uint16_t nUnits = numUnits(piece);

    // Adding 1 to the number to flood gives us both a start of no units and an end of all
    // units. Think of the case of either 1 unit or of 2 units. These cases make it apparent
    // that we actually want 2 and 3 states respectively.
    uint16_t floodedDistance = (nUnits + 1) * p.progress;

    for(uint16_t i=0; i< nUnits; i++) { 
        setFullUnitColor(piece, unitOrder[i], (i < floodedDistance) ? colorInPalette(piece.nexus.palette, ((float)i / (float)floodedDistance)) : black);
    }
}
///////////////////

LAA_DimDebug::LAA_DimDebug(char* szName) : 
    LAA_UnitMapper(szName)
{
    loopDuration = 8000;
    brightness = 0.2;
}

void
LAA_DimDebug::animate(LEDArtPiece& piece, LEDAnimationParam p) {

    if (p.progress > 0.5) {
        clearTo(piece, blue, 0, piece.strip.PixelCount());
        // for(int i=0; i< piece.strip.PixelCount(); i++) {
        //     piece.strip.SetPixelColor(i, blue);
        // }
    } else {
        clearTo(piece, red, 0, piece.strip.PixelCount());
        // for(int i=0; i< piece.strip.PixelCount(); i++) {
        //     piece.strip.SetPixelColor(i, red);
        // }
    }
}

///////////////////

LAA_Kitt::LAA_Kitt(char* szName) : 
    LEDArtAnimation(szName)
{
    loopDuration = 8000;
}

void
LAA_Kitt::animate(LEDArtPiece& piece, LEDAnimationParam p) {

    // Map progress to a full back and forth loop
    float prog;
    if (p.progress < 0.5) {
        prog = p.progress * 2.0;
    } else {
        // Second half of things
        prog = 2.0 - (p.progress * 2.0);
    }

    uint16_t primaryCount = piece.geomPrimaryCount();
    int16_t pos = prog * primaryCount;

    for(int16_t pIx=0; pIx<primaryCount; pIx++) {
        LAColor color = piece.nexus.background;
        if (pIx > pos-3 && pIx < pos+3) {
            color = piece.nexus.foreground;
        }
        piece.setPrimaryColor(pIx, color);
    } 
}

///////////////////

LAA_KittSmooth::LAA_KittSmooth(char* szName) : 
    LEDArtAnimation(szName)
{
    loopDuration = 8000;
}

void
LAA_KittSmooth::animate(LEDArtPiece& piece, LEDAnimationParam p) {

    // Map progress to a full back and forth loop
    float prog;
    if (p.progress < 0.5) {
        prog = p.progress * 2.0;
    } else {
        // Second half of things
        prog = 2.0 - (p.progress * 2.0);
    }

    uint16_t primaryCount = piece.geomPrimaryCount();

    for(int16_t pIx=0; pIx<primaryCount; pIx++) {
        float ixProg = (float)pIx/(float)primaryCount;
        float distance = fabsf(prog - ixProg) * 20.0;
        //distance *= distance;

        LAColor color;
        if (distance < 1.0) {
            color = LAColor::LinearBlend(piece.nexus.foreground, piece.nexus.background, distance);
        } else {
            color = piece.nexus.background;
        }

        piece.setPrimaryColor(pIx, color);
    } 
}


bool
LAA_KittSmooth::canUseGeom(uint16_t width, uint16_t height)
{ 
    return height > width; 
}

///////////////////

LAA_KittPallete::LAA_KittPallete(char* szName) : 
    LEDArtAnimation(szName)
{
    loopDuration = 16000;
}

void
LAA_KittPallete::animate(LEDArtPiece& piece, LEDAnimationParam p) 
{

    // Map progress to a full back and forth loop
    float prog;
    if (p.progress < 0.5) {
        prog = p.progress * 2.0;
    } else {
        // Second half of things
        prog = 2.0 - (p.progress * 2.0);
    }

    uint16_t primaryCount = piece.geomPrimaryCount();

    for(int16_t pIx=0; pIx<primaryCount; pIx++) {
        float ixProg = (float)pIx/(float)primaryCount;
        float distance = fabsf(prog - ixProg) * 8.0;
        //distance *= distance;

        LAColor color;
        if (distance < 1.0) {
            color = colorInPalette(piece.nexus.palette, distance);
        } else {
            color = black;
        }

        piece.setPrimaryColor(pIx, color);
    } 
}

bool
LAA_KittPallete::canUseGeom(uint16_t width, uint16_t height)
{ 
    return height > width; 
}


///////////////////

LAA_RowScan::LAA_RowScan(char* szName) : 
    LEDArtAnimation(szName)
{
}

void
LAA_RowScan::animate(LEDArtPiece& piece, LEDAnimationParam p) 
{

    // Map progress to a full back and forth loop
    // float prog;
    // if (p.progress < 0.5) {
    //     prog = p.progress * 2.0;
    // } else {
    //     // Second half of things
    //     prog = 2.0 - (p.progress * 2.0);
    // }

    float prog = p.progress;

    uint16_t primaryCount = piece.geomPrimaryCount();

    float rowHeight = 1.0 / (float)primaryCount;

    for(int16_t pIx=0; pIx<primaryCount; pIx++) {
        float ixProg = (float)pIx/(float)primaryCount;
        float distance = fabsf(prog - ixProg);
        //distance *= distance;

        LAColor color;
        if (distance < rowHeight) {
            color = piece.nexus.foreground;
            // color = red;
        } else {
            color = piece.nexus.background;
            // color = blue;
        }

        piece.setPrimaryColor(pIx, color);
    } 
}

bool
LAA_RowScan::canUseGeom(uint16_t width, uint16_t height)
{ 
    return true;
}

///////////////////

/*
LAA_RowScan::LAA_RowScan(char* szName) : 
    LEDArtAnimation(szName)
{
}

void
LAA_RowScan::animate(LEDArtPiece& piece, LEDAnimationParam p) 
{

    // Map progress to a full back and forth loop
    // float prog;
    // if (p.progress < 0.5) {
    //     prog = p.progress * 2.0;
    // } else {
    //     // Second half of things
    //     prog = 2.0 - (p.progress * 2.0);
    // }

    float prog = p.progress;

    uint16_t primaryCount = piece.geomPrimaryCount();

    float rowHeight = 1.0 / (float)primaryCount;

    for(int16_t pIx=0; pIx<primaryCount; pIx++) {
        float ixProg = (float)pIx/(float)primaryCount;
        float distance = fabsf(prog - ixProg);
        //distance *= distance;

        LAColor color;
        if (distance < rowHeight) {
            color = piece.nexus.foreground;
            // color = red;
        } else {
            color = piece.nexus.background;
            // color = blue;
        }

        piece.setPrimaryColor(pIx, color);
    } 
}

bool
LAA_RowScan::canUseGeom(uint16_t width, uint16_t height)
{ 
    return true;
}*/