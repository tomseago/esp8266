#pragma once

#include <LEDArt.h>

class LAA_Flood : public LEDArtAnimation {
    LAColor background;
    LAColor foreground;    

public:
    LAA_Flood(char* szName, LAColor background);

    virtual void animate(LEDArtPiece& piece, LEDAnimationParam p);
};

class LAA_Sparkle : public LEDArtAnimation {
    uint16_t pixelCount;
    bool* ppGenerations[3];

public:
    LAA_Sparkle(char* szName, uint16_t pixelCount);

    virtual void animate(LEDArtPiece& piece, LEDAnimationParam p);
};

class LAA_Sparkle2 : public LEDArtAnimation {
    uint16_t pixelCount;
    float* absStartProgress;
    float pixelsUsed;

    float loopBase = 0.0f; // Base absolute progress of our current loop

public:
    LAA_Sparkle2(char* szName, uint16_t pixelCount);

    virtual void animate(LEDArtPiece& piece, LEDAnimationParam p);
};


class LAA_UnitMapper : public LEDArtAnimation {
public:
    LEDUnitType currentType = Unit_Single;
    LAA_UnitMapper(char* szName);

    uint16_t numUnits(LEDArtPiece& piece);
    uint16_t unitSize(LEDArtPiece& piece);

    void setFullUnitColor(LEDArtPiece& piece, uint16_t unitIx, LAColor color);
    void setUnitPixelColor(LEDArtPiece& piece, uint16_t unitIx, uint16_t pixelIx, LAColor color);
    void setAllUnitsPixelColor(LEDArtPiece& piece, uint16_t pixelIx, LAColor color);
};


class LAA_Rainbow : public LAA_UnitMapper {
public:
    LAA_Rainbow(char* szName);

    virtual void animate(LEDArtPiece& piece, LEDAnimationParam p);
};


class LAA_Line : public LAA_UnitMapper {
public:
    LAColor foreground;    

    LAA_Line(char* szName);

    virtual void animate(LEDArtPiece& piece, LEDAnimationParam p);
};

class LAA_BoxOutline : public LAA_UnitMapper {
public:
    LAA_BoxOutline(char* szName);

    virtual void animate(LEDArtPiece& piece, LEDAnimationParam p);
};

class LAA_AllWhite : public LAA_UnitMapper {
public:
    LAA_AllWhite(char* szName);

    virtual void animate(LEDArtPiece& piece, LEDAnimationParam p);
};

class LAA_HalfWhite : public LAA_UnitMapper {
public:
    LAA_HalfWhite(char* szName);

    virtual void animate(LEDArtPiece& piece, LEDAnimationParam p);
};


class LAA_UnitFill : public LAA_UnitMapper {
protected:
    uint8_t calculatedFor = (uint8_t)Unit_Last;
    uint16_t* unitOrder;

    virtual void calculateOrder(LEDArtPiece& piece);
public:
    LAA_UnitFill(char* szName);

    virtual void animate(LEDArtPiece& piece, LEDAnimationParam p);
    virtual bool canUseGeom(uint16_t width, uint16_t height);
};

class LAA_RandoFill : public LAA_UnitFill {
protected:
    virtual void calculateOrder(LEDArtPiece& piece);
public:
    LAA_RandoFill(char* szName);
};

class LAA_PaletteFill : public LAA_UnitFill {
public:
    LAA_PaletteFill(char* szName);
    virtual void animate(LEDArtPiece& piece, LEDAnimationParam p);
};



class LAA_DimDebug : public LAA_UnitMapper {
public:
    LAA_DimDebug(char* szName);

    virtual void animate(LEDArtPiece& piece, LEDAnimationParam p);
};



class LAA_Kitt : public LEDArtAnimation {

public:
    LAA_Kitt(char* szName);

    virtual void animate(LEDArtPiece& piece, LEDAnimationParam p);
};


class LAA_KittSmooth : public LEDArtAnimation {

public:
    LAA_KittSmooth(char* szName);

    virtual void animate(LEDArtPiece& piece, LEDAnimationParam p);
    virtual bool canUseGeom(uint16_t width, uint16_t height);
};

class LAA_KittPallete : public LEDArtAnimation {

public:
    LAA_KittPallete(char* szName);

    virtual void animate(LEDArtPiece& piece, LEDAnimationParam p);
    virtual bool canUseGeom(uint16_t width, uint16_t height);
};
