#pragma once
#include <memory>

#include "Adafruit_NeoPixel.h"
#include "buttons.hpp"
#include "digit_manager.hpp"
#include "rtc_hal.hpp"
#include "settings.hpp"

class Clock
{
  private:
    enum ClockState_e
    {
        STATE_NORMAL,
        STATE_SET_TIME,
        STATE_DISPLAY_VALUE,
    };

    Adafruit_NeoPixel m_leds{NUM_LEDS, PIN_FOR_LEDS, NEO_GRB + NEO_KHZ400};
    Settings m_settings;
    Buttons m_buttons;
    DigitManager m_digitMgr;
    ClockState_e m_state{STATE_NORMAL};

  public:
    Clock() : m_digitMgr(m_leds, m_settings)
    {
        Serial.begin(115200);
        rtc_hal_init();

        m_leds.begin();
        m_leds.setBrightness(m_settings.Get(SETTING_CUR_BRIGHTNESS));
    }

    void Loop()
    {
        m_buttons.CheckForEvents();

        DisplayDigits();
    }

  private:
    void DisplayDigits()
    {
        if (m_state == STATE_NORMAL)
        {
            const auto numbers = GetNumbersFromRTC();
            m_digitMgr.Display(numbers);
        }
        else
        {
            m_digitMgr.Display();
        }

        m_leds.show();
    }

    Numbers_t GetNumbersFromRTC() const
    {
        Numbers_t numbers(NUM_DIGITS, 0);

        // on a Foxie Clock, the numbers are laid out this way:
        // numbers[0] is the leftmost digit
        // numbers[5] is the rightmost digit

        rtc_hal_update();
        if (m_settings.Get(SETTING_24_HOUR_MODE) == 1)
        {
            numbers[0] = rtc_hal_hour() / 10;
            numbers[1] = rtc_hal_hour() % 10;
        }
        else
        {
            numbers[0] = rtc_hal_hourFormat12() / 10;
            numbers[1] = rtc_hal_hourFormat12() % 10;

            if (numbers[0] == 0)
            {
                // disable leading 0 for 12 hour mode
                numbers[0] = Digit::INVALID;
            }
        }

        numbers[2] = rtc_hal_minute() / 10;
        numbers[3] = rtc_hal_minute() % 10;

        numbers[4] = rtc_hal_second() / 10;
        numbers[5] = rtc_hal_second() % 10;

        return numbers;
    }
};
