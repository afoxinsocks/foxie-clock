#pragma once

#include "Adafruit_NeoPixel.h"
#include "settings.hpp"

class FlippableLEDStrip : public Adafruit_NeoPixel
{
  private:
    using Adafruit_NeoPixel::Adafruit_NeoPixel;

    enum
    {
        NUM_BLINKER_LEDS = 2,
    };

  public:
    virtual void setPixelColor(uint16_t n, uint32_t c) override
    {
        const int lastPixelNum = (numPixels() - 1 - NUM_BLINKER_LEDS);
        if (Settings::Get(SETTING_FLIP_DISPLAY) && n <= lastPixelNum)
        {
            Adafruit_NeoPixel::setPixelColor(lastPixelNum - n, c);
        }
        else
        {
            Adafruit_NeoPixel::setPixelColor(n, c);
        }
    }

    void FadeToOff()
    {
        for (int brightness = Settings::Get(SETTING_CUR_BRIGHTNESS); brightness >= 0; brightness -= 3)
        {
            setBrightness(brightness);
            show();
        }
        setBrightness(0);
        show();
    }

    void SetToCurrentBrightness()
    {

        setBrightness(Settings::Get(SETTING_CUR_BRIGHTNESS));
        show();
    }
};
