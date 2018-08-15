#include "buttonwatcher.h"

#include "log.h"

// #include "rand.h"
// #include "nexus.h"

// THe original 2 header version
// #define BUTTON_1 12
// #define BUTTON_2 14
// #define BUTTON_3 5
// #define BUTTON_4 13

// // Which should be this with names...
// #define BUTTON_1 D6
// #define BUTTON_2 D5
// #define BUTTON_3 D1
// #define BUTTON_4 D7


// // The new 1 header version.
// // D4 is the built in LED so maybe this works???
// #define BUTTON_1 D3
// #define BUTTON_2 D4
// #define BUTTON_3 D1
// #define BUTTON_4 D2


// For vest
#define BUTTON_1 D4
#define BUTTON_2 D3
#define BUTTON_3 D2
#define BUTTON_4 D1

ButtonWatcher::ButtonWatcher(uint16_t longThreshold) :
    buttons({
        Bounce(BUTTON_1, 50),
        Bounce(BUTTON_2, 50),
        Bounce(BUTTON_3, 50),
        Bounce(BUTTON_4, 50),
    }),
    longThreshold(longThreshold)
{
}

void 
ButtonWatcher::begin() {
    pinMode(BUTTON_1, INPUT_PULLUP);
    pinMode(BUTTON_2, INPUT_PULLUP);
    pinMode(BUTTON_3, INPUT_PULLUP);
    pinMode(BUTTON_4, INPUT_PULLUP);
}

void
ButtonWatcher::loop() {

    // Serial.print("Hello?");
    // // for(int i=4; i<14; i++) {
    //     // bool readState = digitalRead(i);
    //     Serial.print(digitalRead(BUTTON_1) ? "X" : ".");
    //     Serial.print(digitalRead(BUTTON_2) ? "X" : ".");
    //     Serial.print(digitalRead(BUTTON_3) ? "X" : ".");
    //     Serial.print(digitalRead(BUTTON_4) ? "X" : ".");
    // // }
    // Serial.print("\n");

    uint32_t now = millis();

    for(int i=0; i<4; i++) {
        buttons[i].update();

        if (buttons[i].rose()) {
            Log.printf("BW: Button %d rose\n", i);
            buttonRose(i);
            downAt[i] = 0;            
        } else if (buttons[i].fell()) {
            Log.printf("BW: Button %d fell\n", i);
            buttonFell(i);
            downAt[i] = now;
            notified[i] = false;
        } else if ((!buttons[i].read()) && !notified[i] && (now - downAt[i] > longThreshold)) {
            Log.printf("BW: Button %d long press\n", i);
            buttonLongPress(i);
            notified[i] = true;
        }

        // bool isPressed = buttons[i].read();

        // if (isPressed) {
        //     for(int j=0; j<i; j++) {
        //         Serial.print(" ");
        //     }
        //     Serial.print("")
        // }
    }
}
