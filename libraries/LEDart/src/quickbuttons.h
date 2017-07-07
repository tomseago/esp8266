#include "LEDArt.h"

#include <Bounce2.h>

class QuickButtons {
public:
    LEDArtPiece& piece;
    Bounce buttons[4];
    LEDArtAnimation* flashLight;
    bool flashLightOn=false;

    QuickButtons(LEDArtPiece& piece, LEDArtAnimation* flashLight);

    void begin();
    void loop();
};