#pragma once
#include "Adafruit_NeoPixel.h"
#include "button.hpp"
#include "digit_manager.hpp"
#include "rtc_hal.hpp"
#include "settings.hpp"
#include <memory>

class Clock
{
  private:
    enum ClockState_e
    {
        STATE_NORMAL,
        STATE_SET_TIME,
        STATE_DISPLAY_VALUE,
    };

    Adafruit_NeoPixel &m_leds;
    Settings &m_settings;
    DigitManager m_digitMgr;
    ClockState_e m_state{STATE_NORMAL};

  public:
    Clock(Adafruit_NeoPixel &leds, Settings &settings) : m_leds(leds), m_settings(settings), m_digitMgr(leds, settings)
    {
        m_leds.setBrightness(m_settings.Get(SETTING_CUR_BRIGHTNESS));
    }

    void Process()
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

  private:
    Numbers_t GetNumbersFromRTC()
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
