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

    ClockState_e m_state{STATE_NORMAL};

    Adafruit_NeoPixel &m_leds;
    Settings &m_settings;
    DigitManager m_digitMgr;

  public:
    Clock(Adafruit_NeoPixel &leds, Settings &settings) : m_leds(leds), m_settings(settings), m_digitMgr(leds, settings)
    {
        m_leds.setBrightness(settings.Get(SETTING_CUR_BRIGHTNESS));
    }

    void Process()
    {
        if (m_state == STATE_NORMAL)
        {
            m_digitMgr.UpdateDigitsFromRTC();
        }
        else
        {
            m_digitMgr.Update();
        }

        m_leds.show();
    }
};
