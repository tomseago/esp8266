#include "quickbuttons.h"
#include "rand.h"
#include "nexus.h"

// THe original 2 header version
// #define BUTTON_1 12
// #define BUTTON_2 14
// #define BUTTON_3 5
// #define BUTTON_4 13

#ifdef ESP32

// Translate for ESP32
#define D7 -1
#define D6 -1
#define D5 -1

#define D4 analogInputToDigitalPin(A14)
#define D3 analogInputToDigitalPin(A15)
#define D2 analogInputToDigitalPin(A16)
#define D1 analogInputToDigitalPin(A17)

#endif


// Which should be this with names...
#define BUTTON_1 D6
#define BUTTON_2 D5
#define BUTTON_3 D1
#define BUTTON_4 D7


// The new 1 header version.
// D4 is the built in LED so maybe this works???
#define BUTTON_1 D3
#define BUTTON_2 D4
#define BUTTON_3 D1
#define BUTTON_4 D2


// For vest
#define BUTTON_1 D4
#define BUTTON_2 D3
#define BUTTON_3 D2
#define BUTTON_4 D1


QuickButtons::QuickButtons(LEDArtPiece& piece, LEDArtAnimation* flashLight) :
    piece(piece),
    buttons({
        Bounce(BUTTON_1, 50),
        Bounce(BUTTON_2, 50),
        Bounce(BUTTON_3, 50),
        Bounce(BUTTON_4, 50),
    }),
    flashLight(flashLight)
{
    // Make sure it is overlay type
    flashLight->type = LEDAnimationType_OVERLAY;
    flashLight->maxDuration = 0x0FFFFFFF; // A really long overlay yeah?

    // This is not at all the right way to do a flashlight, but whatever...
    // It doesn't really work right.
}

void 
QuickButtons::begin() {
    pinMode(BUTTON_1, INPUT_PULLUP);
    pinMode(BUTTON_2, INPUT_PULLUP);
    pinMode(BUTTON_3, INPUT_PULLUP);
    pinMode(BUTTON_4, INPUT_PULLUP);
}

void
QuickButtons::loop() {

    // Serial.print("Hello?");
    // // for(int i=4; i<14; i++) {
    //     // bool readState = digitalRead(i);
    //     Serial.print(digitalRead(BUTTON_1) ? "X" : ".");
    //     Serial.print(digitalRead(BUTTON_2) ? "X" : ".");
    //     Serial.print(digitalRead(BUTTON_3) ? "X" : ".");
    //     Serial.print(digitalRead(BUTTON_4) ? "X" : ".");
    // // }
    // Serial.print("\n");

    for(int i=0; i<4; i++) {
        buttons[i].update();

        if (buttons[i].rose()) {
            Serial.printf("Button %d rose\n", i);
        }

        if (buttons[i].fell()) {
            Serial.printf("Button %d fell\n", i);
        }

        // bool isPressed = buttons[i].read();

        // if (isPressed) {
        //     for(int j=0; j<i; j++) {
        //         Serial.print(" ");
        //     }
        //     Serial.print("")
        // }
    }

    if (buttons[0].fell()) {
        if (flashLightOn) {
            piece.stopAnimation(LEDAnimationType_OVERLAY);
        } else {
            piece.startAnimation(flashLight, false);
        }
        flashLightOn = !flashLightOn;
    }

    if (buttons[1].fell()) {
        piece.nexus.nextAnimation((uint32_t)this);
    }

    if (buttons[2].fell()) {
        piece.nexus.nextPalette((uint32_t)this);
    }

    if (buttons[3].fell()) {
        piece.nexus.nextGeometry((uint32_t)this);
        // piece.nexus.nextUnitType((uint32_t)this);
    }

}
