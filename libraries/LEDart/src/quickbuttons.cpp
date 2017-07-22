#include "quickbuttons.h"
#include "rand.h"
#include "nexus.h"

QuickButtons::QuickButtons(LEDArtPiece& piece, LEDArtAnimation* flashLight) :
    piece(piece),
    buttons({
        Bounce(12, 50),
        Bounce(14, 50),
        Bounce( 5, 50),
        Bounce(13, 50),
    }),
    flashLight(flashLight)
{
}

void 
QuickButtons::begin() {
    pinMode(12, INPUT_PULLUP);
    pinMode(14, INPUT_PULLUP);
    pinMode(5, INPUT_PULLUP);
    pinMode(13, INPUT_PULLUP);
}

void
QuickButtons::loop() {

    // Serial.print("Hello?");
    // // for(int i=4; i<14; i++) {
    //     // bool readState = digitalRead(i);
    //     Serial.print(digitalRead(12) ? "X" : ".");
    //     Serial.print(digitalRead(14) ? "X" : ".");
    //     Serial.print(digitalRead(5) ? "X" : ".");
    //     Serial.print(digitalRead(13) ? "X" : ".");
    // // }
    // Serial.print("\n");

    for(int i=0; i<4; i++) {
        buttons[i].update();

        // if (buttons[i].rose()) {
        //     Serial.printf("Button %d rose\n", i);
        // }

        // if (buttons[i].fell()) {
        //     Serial.printf("Button %d fell\n", i);
        // }

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
            piece.stopAnimation(AnimationType_OVERLAY);
        } else {
            piece.startAnimation(flashLight);
        }
        flashLightOn = !flashLightOn;
    }

    if (buttons[1].fell()) {
        piece.nextAnimation(false);
    }

    if (buttons[2].fell()) {
        piece.nexus.nextPalette((uint32_t)this);
    }

    if (buttons[3].fell()) {
        piece.nexus.nextUnitType((uint32_t)this);
    }

}
