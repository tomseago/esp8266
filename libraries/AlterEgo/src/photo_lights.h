#pragma once

#include <NeoPixelBrightnessBus.h>

#define RED RgbwColor(255,0,0,0)
#define GREEN RgbwColor(0,255,0,0)
#define BLUE RgbwColor(0,0,255,0)

template<typename T_METHOD> class PhotoLights {
public:
    PhotoLights(uint8_t pin) :
        bus(300,pin)
    {

    }

    // NeoPixelBrightnessBus<NeoGrbwFeature,NeoEsp32I2s0800KbpsMethod> bus;
    NeoPixelBrightnessBus<NeoGrbwFeature,T_METHOD> bus;

    void begin() {
        bus.Begin();
        // bus.SetBrightness(32);
        // bus.ClearTo(BLUE);
        // bus.Show();
    }

    void loop() {
        // TODO: Switch based on mode
        uint32_t now = millis();

        uint32_t sinceLast = now - this->lastUpdateAt;

        if (this->dirty || sinceLast > 1000) {
            updatePixels();
            this->dirty = false;
            this->lastUpdateAt = now;
        }
    }

    void setHue(float hue) {
        if (hue < 0.0) {
            hue = 0.0;
        }
        if (hue > 1.0) {
            hue = 1.0;
        }
        this->hue = hue;
        this->dirty = true;
    }

    void setSat(float sat) {
        if (sat < 0.0) {
            sat = 0.0;
        }
        if (sat > 1.0) {
            sat = 1.0;
        }
        this->sat = sat;
        this->dirty = true;
    }

    void setBright(float bright){
        if (bright < 0.0) {
            bright = 0.0;
        }
        if (bright > 1.0) {
            bright = 1.0;
        }
        this->bright = bright;
        this->dirty = true;
    }



private:
    uint8_t mode;

    float hue = 0.0;
    float sat = 1.0;
    float bright = 1.0;

    bool dirty = false;
    bool flipFlop;

    RgbwColor color;
    uint32_t lastUpdateAt = 0;

    void updatePixels() {
        // if (hue == 1.0) {
        //     bus.ClearTo(RED);
        // } else {
        //     bus.ClearTo(BLUE);
        // }

        // if (flipFlop) {
        //     bus.SetPixelColor(1, GREEN);
        // } else {
        //     bus.SetPixelColor(0, GREEN);
        // }
        // flipFlop = !flipFlop;

        bus.SetBrightness(this->bright * 255.0);

        HsbColor hsb(hue, 1.0, sat);
        RgbwColor rgbw(hsb);
        rgbw.W = (1.0 - sat)*255;
        bus.ClearTo(rgbw);
        bus.Show();
    }

};