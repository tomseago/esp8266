#include "animations.h"
#include "rand.h"
#include "nexus.h"

const RgbColor red(255,0,0);
const RgbColor yellow(255,255,0);
const RgbColor green(0,255,0);
const RgbColor cyan(0,255,255);
const RgbColor blue(0,0,255);
const RgbColor purple(255,0,255);

const RgbColor white(255,255,255);
const RgbColor black(0,0,0);


LAA_Flood::LAA_Flood(char* szName, RgbColor background) : 
    LEDArtAnimation(szName),
    background(background),
    foreground(background)
{

}

void
LAA_Flood::animate(LEDArtPiece& piece, AnimationParam p) {
    if (p.state == AnimationState_Started) {
        background = foreground;

        foreground = RgbColor(HslColor(rand(255)/255.0, 0.6, 0.5));
    }

    uint16_t floodedDistance = piece.strip.PixelCount() * p.progress;

    piece.strip.ClearTo(foreground, 0, floodedDistance);
    piece.strip.ClearTo(background, floodedDistance+1, piece.strip.PixelCount());
}

/////////

LAA_Sparkle::LAA_Sparkle(char* szName, uint16_t pixelCount) : 
    LEDArtAnimation(szName), pixelCount(pixelCount)
{
    loopDuration = 50;
    maxDuration = 8000;

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
LAA_Sparkle::animate(LEDArtPiece& piece, AnimationParam p) {
    if (p.state == AnimationState_Started) {
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
LAA_Sparkle2::animate(LEDArtPiece& piece, AnimationParam p) {
    if (p.state == AnimationState_Started) {
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
    switch(currentType) {
    case Unit_Single:
        return 1;

    case Unit_Each:
        return piece.strip.PixelCount();

    case Unit_Rows:
        return piece.topo.getHeight();

    case Unit_Cols:
        return piece.topo.getWidth();

    case Unit_SpecificRows:
        if (piece.specificGeometry) 
        {
            return piece.specificGeometry->height;
        }
        else
        {
            return piece.topo.getHeight();
        }

    case Unit_SpecificCols:
        if (piece.specificGeometry)
        {
            return piece.specificGeometry->width;
        }
        else
        {
            return piece.topo.getWidth();
        }
    }

    return 0;
}


uint16_t 
LAA_UnitMapper::unitSize(LEDArtPiece& piece) {
    switch(currentType) {
    case Unit_Single:
        return piece.strip.PixelCount();

    case Unit_Each:
        return 1;

    case Unit_Rows:
        return piece.topo.getWidth();

    case Unit_Cols:
        return piece.topo.getHeight();

    case Unit_SpecificRows:
        if (piece.specificGeometry)
        {
            return piece.specificGeometry->width;
        }
        else
        {
            return piece.topo.getWidth();
        }

    case Unit_SpecificCols:
        if (piece.specificGeometry)
        {
            return piece.specificGeometry->height;
        }
        else
        {
            return piece.topo.getHeight();
        }
    }

    return 0;
}

void 
LAA_UnitMapper::setFullUnitColor(LEDArtPiece& piece, uint16_t unitIx, RgbColor color)
{
    switch(currentType) {
    case Unit_Single:
        piece.strip.ClearTo(color);
        break;

    case Unit_Each:
        piece.strip.SetPixelColor(unitIx, color);
        break;

    case Unit_Rows:
        for(int16_t i=0; i<piece.topo.getWidth(); i++) {
            piece.strip.SetPixelColor(piece.topo.Map(i, unitIx), color);
        }
        break;

    case Unit_Cols:
        for(int16_t i=0; i<piece.topo.getHeight(); i++) {
            piece.strip.SetPixelColor(piece.topo.Map(unitIx, i), color);
        }
        break;

    case Unit_SpecificRows:
        if (piece.specificGeometry) {
            for(int16_t i=0; i<piece.topo.getWidth(); i++) {
                piece.strip.SetPixelColor(piece.specificGeometry->Map(i, unitIx), color);
            }
        }
        else
        {
            for(int16_t i=0; i<piece.topo.getWidth(); i++) {
                piece.strip.SetPixelColor(piece.topo.Map(i, unitIx), color);
            }            
        }
        break;

    case Unit_SpecificCols:
        if (piece.specificGeometry) {
            for(int16_t i=0; i<piece.topo.getHeight(); i++) {
                piece.strip.SetPixelColor(piece.specificGeometry->Map(unitIx, i), color);
            }
        }
        else
        {
            for(int16_t i=0; i<piece.topo.getHeight(); i++) {
                piece.strip.SetPixelColor(piece.topo.Map(unitIx, i), color);
            }            
        }
        break;
    }
}

void 
LAA_UnitMapper::setUnitPixelColor(LEDArtPiece& piece, uint16_t unitIx, uint16_t pixelIx, RgbColor color)
{
    switch(currentType) {
    case Unit_Single:
        piece.strip.SetPixelColor(pixelIx, color);
        break;

    case Unit_Each:
        piece.strip.SetPixelColor(unitIx, color);

    case Unit_Rows:
        piece.strip.SetPixelColor(piece.topo.Map(pixelIx, unitIx), color);
        break;

    case Unit_Cols:
        piece.strip.SetPixelColor(piece.topo.Map(unitIx, pixelIx), color);
        break;

    case Unit_SpecificRows:
        if (piece.specificGeometry)
        {
            piece.strip.SetPixelColor(piece.specificGeometry->Map(pixelIx, unitIx), color);
        }
        else
        {
            piece.strip.SetPixelColor(piece.topo.Map(pixelIx, unitIx), color);
        }
        break;

    case Unit_SpecificCols:
        if (piece.specificGeometry)
        {
            piece.strip.SetPixelColor(piece.specificGeometry->Map(unitIx, pixelIx), color);
        }
        else
        {
            piece.strip.SetPixelColor(piece.topo.Map(unitIx, pixelIx), color);            
        }
        break;
    }
}

void 
LAA_UnitMapper::setAllUnitsPixelColor(LEDArtPiece& piece, uint16_t pixelIx, RgbColor color)
{
    switch(currentType) {
    case Unit_Single:
        piece.strip.SetPixelColor(pixelIx, color);
        break;

    case Unit_Each:
        piece.strip.ClearTo(color);
        break;

    case Unit_Rows:
        for(int16_t i=0; i<piece.topo.getHeight(); i++) {
            piece.strip.SetPixelColor(piece.topo.Map(pixelIx, i), color);
        }
        break;

    case Unit_Cols:
        for(int16_t i=0; i<piece.topo.getWidth(); i++) {
            piece.strip.SetPixelColor(piece.topo.Map(i, pixelIx), color);
        }
        break;

    case Unit_SpecificRows:
        if (piece.specificGeometry)
        {
            for(int16_t i=0; i<piece.topo.getHeight(); i++) {
                piece.strip.SetPixelColor(piece.specificGeometry->Map(pixelIx, i), color);
            }            
        }
        else
        {
            for(int16_t i=0; i<piece.topo.getHeight(); i++) {
                piece.strip.SetPixelColor(piece.topo.Map(pixelIx, i), color);
            }            
        }
        break;

    case Unit_SpecificCols:
        if (piece.specificGeometry)
        {
            for(int16_t i=0; i<piece.topo.getWidth(); i++) {
                piece.strip.SetPixelColor(piece.specificGeometry->Map(i, pixelIx), color);
            }            
        }
        else
        {
            for(int16_t i=0; i<piece.topo.getWidth(); i++) {
                piece.strip.SetPixelColor(piece.topo.Map(i, pixelIx), color);
            }
        }
        break;    
    }
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
LAA_Rainbow::animate(LEDArtPiece& piece, AnimationParam p) {

    currentType = (LAA_UnitMapper::UnitType)piece.nexus.unitType;

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
LAA_Line::animate(LEDArtPiece& piece, AnimationParam p) {
    currentType = (LAA_UnitMapper::UnitType)piece.nexus.unitType;

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
LAA_BoxOutline::animate(LEDArtPiece& piece, AnimationParam p) {

    piece.strip.ClearTo(red);

    // uint16_t w = piece.topo.getWidth();
    // uint16_t h = piece.topo.getHeight();

    // for(int i=0; i<w; i++) {
    //     piece.strip.SetPixelColor(piece.topo.Map(i, 0), green);
    //     piece.strip.SetPixelColor(piece.topo.Map(i, h-1), green);
    // }
    // for(int i=0; i<h; i++) {
    //     piece.strip.SetPixelColor(piece.topo.Map(0, i), blue);
    //     piece.strip.SetPixelColor(piece.topo.Map(w-1, i), blue);
    // }

    currentType = Unit_SpecificCols;

    setFullUnitColor(piece, 0, green);
    setFullUnitColor(piece, numUnits(piece), blue);

    currentType = Unit_SpecificRows;

    setFullUnitColor(piece, 0, cyan);
    setFullUnitColor(piece, numUnits(piece), purple);
}


///////////////////

LAA_AllWhite::LAA_AllWhite(char* szName) : 
    LAA_UnitMapper(szName)
{
    maxDuration = 10000;
    brightness = 0.6;
}

void
LAA_AllWhite::animate(LEDArtPiece& piece, AnimationParam p) {

    piece.strip.ClearTo(white);
}


///////////////////

LAA_HalfWhite::LAA_HalfWhite(char* szName) : 
    LAA_UnitMapper(szName)
{
    type = AnimationType_OVERLAY;
    currentType = Unit_SpecificCols;
}

void
LAA_HalfWhite::animate(LEDArtPiece& piece, AnimationParam p) {

    uint16_t nUnits = numUnits(piece);

    for(int i=0; i< nUnits; i++) { 
        setFullUnitColor(piece, i, (i < nUnits / 2) ? white : black);
    }

}


///////////////////

LAA_RandoFill::LAA_RandoFill(char* szName) : 
    LAA_UnitMapper(szName),
    unitOrder(NULL)
{
    type = AnimationType_BASE;
}

void
LAA_RandoFill::animate(LEDArtPiece& piece, AnimationParam p) 
{
    if (p.state == AnimationState_Started || !unitOrder || calculatedFor != piece.nexus.unitType) {

        calculateOrder(piece);
    }

    uint16_t nUnits = numUnits(piece);
    uint16_t floodedDistance = nUnits * p.progress;

    for(uint16_t i=0; i< nUnits; i++) { 
        setFullUnitColor(piece, unitOrder[i], (i < floodedDistance) ? piece.nexus.foreground : piece.nexus.background);
    }
}

void
LAA_RandoFill::calculateOrder(LEDArtPiece& piece) 
{

    currentType = (LAA_UnitMapper::UnitType)piece.nexus.unitType;

    // For the moment the calculated order is identical to the native order
    if (unitOrder) {
        delete unitOrder;
    }

    uint16_t nUnits = numUnits(piece);
    unitOrder = new uint16_t[nUnits];
    for(uint16_t unitIx=0; unitIx < nUnits; unitIx++)
    {
        unitOrder[unitIx] = unitIx;
    }

    calculatedFor = (uint8_t)currentType;
}
