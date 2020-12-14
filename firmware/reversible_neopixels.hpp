#pragma once

#include "Adafruit_NeoPixel.h"
#include "settings.hpp"

class ReversibleNeopixels : public Adafruit_NeoPixel
{
  private:
    using Adafruit_NeoPixel::Adafruit_NeoPixel;
    Settings &m_settings;

    enum
    {
        NUM_BLINKER_LEDS = 2,
    };

  public:
    // See Adafruit_Neopixel constructor for more details
    ReversibleNeopixels(Settings &settings, uint16_t numPixels, uint16_t pin, neoPixelType type)
        : m_settings(settings), Adafruit_NeoPixel(numPixels, pin, type)
    {
    }

    virtual void setPixelColor(uint16_t n, uint32_t c) override
    {
        const int lastPixelNum = (numPixels() - 1 - NUM_BLINKER_LEDS);
        if (m_settings.Get(SETTING_FLIP_DISPLAY) && n <= lastPixelNum)
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
        for (int brightness = m_settings.Get(SETTING_CUR_BRIGHTNESS); brightness >= 0; brightness -= 3)
        {
            setBrightness(brightness);
            show();
        }
        setBrightness(0);
        show();
    }

    void SetToCurrentBrightness()
    {
        setBrightness(m_settings.Get(SETTING_CUR_BRIGHTNESS));
        show();
    }
};