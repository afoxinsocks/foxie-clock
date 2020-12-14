#pragma once
#include "Adafruit_NeoPixel.h"
#include "digit.hpp"
#include "rtc_hal.hpp"
#include "settings.hpp"

class Blinkers
{
  private:
    enum DigitSeparatorLEDs_e
    {
        // for edge lit digits, use the two LEDs between rows 2/3 and 4/5
        BLINK_DIGIT_TYPE_1_LED_1 = 120,
        BLINK_DIGIT_TYPE_1_LED_2 = 121,

        // for PXL digits, use 4 LEDs in rows 2 and 4
        BLINK_DIGIT_TYPE_2_LED_1 = 25,
        BLINK_DIGIT_TYPE_2_LED_2 = 33,
        BLINK_DIGIT_TYPE_2_LED_3 = 65,
        BLINK_DIGIT_TYPE_2_LED_4 = 73,
    };
    Adafruit_NeoPixel &m_leds;
    Settings &m_settings;

  public:
    Blinkers(Adafruit_NeoPixel &leds, Settings &settings) : m_leds(leds), m_settings(settings)
    {
    }

    void Update()
    {
        if (rtc_hal_second() % 2 != 0 && m_settings.Get(SETTING_BLINKING_SEPARATORS))
        {
            TurnOnBlinkers();
        }
        else
        {
            TurnOffBlinkers();
        }
    }

  private:
    void TurnOnBlinkers()
    {
        const int blinkColor = ColorWheel(m_settings.Get(SETTING_COLOR));
        if (m_settings.Get(SETTING_DIGIT_TYPE) == DT_EDGE_LIT)
        {
            m_leds.setPixelColor(BLINK_DIGIT_TYPE_1_LED_1, blinkColor);
            m_leds.setPixelColor(BLINK_DIGIT_TYPE_1_LED_2, blinkColor);
        }
        else if (m_settings.Get(SETTING_DIGIT_TYPE) == DT_PIXELS)
        {
            m_leds.setPixelColor(BLINK_DIGIT_TYPE_2_LED_1, blinkColor);
            m_leds.setPixelColor(BLINK_DIGIT_TYPE_2_LED_2, blinkColor);
            m_leds.setPixelColor(BLINK_DIGIT_TYPE_2_LED_3, blinkColor);
            m_leds.setPixelColor(BLINK_DIGIT_TYPE_2_LED_4, blinkColor);
        }
    }

    void TurnOffBlinkers()
    {
        if (m_settings.Get(SETTING_DIGIT_TYPE) == DT_EDGE_LIT)
        {
            m_leds.setPixelColor(BLINK_DIGIT_TYPE_1_LED_1, 0);
            m_leds.setPixelColor(BLINK_DIGIT_TYPE_1_LED_2, 0);
        }
        else
        {
            m_leds.setPixelColor(BLINK_DIGIT_TYPE_2_LED_1, 0);
            m_leds.setPixelColor(BLINK_DIGIT_TYPE_2_LED_2, 0);
            m_leds.setPixelColor(BLINK_DIGIT_TYPE_2_LED_3, 0);
            m_leds.setPixelColor(BLINK_DIGIT_TYPE_2_LED_4, 0);
        }
    }
};
