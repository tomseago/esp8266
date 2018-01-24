#pragma once

#include <LEDArt.h>

class LAA_Flood : public LEDArtAnimation {
    RgbColor background;
    RgbColor foreground;    

public:
    LAA_Flood(char* szName, RgbColor background);

    virtual void animate(LEDArtPiece& piece, AnimationParam p);
};

class LAA_Sparkle : public LEDArtAnimation {
    uint16_t pixelCount;
    bool* ppGenerations[3];

public:
    LAA_Sparkle(char* szName, uint16_t pixelCount);

    virtual void animate(LEDArtPiece& piece, AnimationParam p);
};

class LAA_Sparkle2 : public LEDArtAnimation {
    uint16_t pixelCount;
    float* absStartProgress;
    float pixelsUsed;

    float loopBase = 0.0f; // Base absolute progress of our current loop

public:
    LAA_Sparkle2(char* szName, uint16_t pixelCount);

    virtual void animate(LEDArtPiece& piece, AnimationParam p);
};


class LAA_UnitMapper : public LEDArtAnimation {
public:
    enum UnitType {
        Unit_Single = 0,
        Unit_Each,
        Unit_Rows,
        Unit_Cols,
        Unit_SpecificRows,
        Unit_SpecificCols,

        Unit_Last,
    };

    UnitType currentType = Unit_Single;
    LAA_UnitMapper(char* szName);

    uint16_t numUnits(LEDArtPiece& piece);
    uint16_t unitSize(LEDArtPiece& piece);

    void setFullUnitColor(LEDArtPiece& piece, uint16_t unitIx, RgbColor color);
    void setUnitPixelColor(LEDArtPiece& piece, uint16_t unitIx, uint16_t pixelIx, RgbColor color);
    void setAllUnitsPixelColor(LEDArtPiece& piece, uint16_t pixelIx, RgbColor color);
};


class LAA_Rainbow : public LAA_UnitMapper {
public:
    LAA_Rainbow(char* szName);

    virtual void animate(LEDArtPiece& piece, AnimationParam p);
};


class LAA_Line : public LAA_UnitMapper {
public:
    RgbColor foreground;    

    LAA_Line(char* szName);

    virtual void animate(LEDArtPiece& piece, AnimationParam p);
};

class LAA_BoxOutline : public LAA_UnitMapper {
public:
    LAA_BoxOutline(char* szName);

    virtual void animate(LEDArtPiece& piece, AnimationParam p);
};

class LAA_AllWhite : public LAA_UnitMapper {
public:
    LAA_AllWhite(char* szName);

    virtual void animate(LEDArtPiece& piece, AnimationParam p);
};

class LAA_HalfWhite : public LAA_UnitMapper {
public:
    LAA_HalfWhite(char* szName);

    virtual void animate(LEDArtPiece& piece, AnimationParam p);
};

class LAA_RandoFill : public LAA_UnitMapper {
    uint8_t calculatedFor = (uint8_t)Unit_Last;
    uint16_t* unitOrder;

    void calculateOrder(LEDArtPiece& piece);
public:
    LAA_RandoFill(char* szName);

    virtual void animate(LEDArtPiece& piece, AnimationParam p);
};

class LAA_DimDebug : public LAA_UnitMapper {
public:
    LAA_DimDebug(char* szName);

    virtual void animate(LEDArtPiece& piece, AnimationParam p);
};
