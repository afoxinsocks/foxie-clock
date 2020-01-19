#pragma once
#include "Adafruit_NeoPixel.h"
#include "animator.hpp"
#include "digit_manager.hpp"
#include "rtc_hal.hpp"
#include "settings.hpp"

enum ClockState_e
{
    STATE_NORMAL,
    STATE_SET_TIME,
};

class Clock
{
  private:
    enum DigitSeparatorLEDs_e
    {
        // for edge lit digits, use the two LEDs between rows 2/3 and 4/5
        BLINK_DIGIT_TYPE_1_LED_1 = 120,
        BLINK_DIGIT_TYPE_1_LED_2 = 121,

        // for pixel lit digits, use 4 LEDs in rows 2 and 4
        BLINK_DIGIT_TYPE_2_LED_1 = 25,
        BLINK_DIGIT_TYPE_2_LED_2 = 33,
        BLINK_DIGIT_TYPE_2_LED_3 = 65,
        BLINK_DIGIT_TYPE_2_LED_4 = 73,
    };

    Adafruit_NeoPixel &m_leds;
    DigitManager m_digitMgr;
    int m_lastRedrawTime{-1};
    ClockState_e &m_state;
    Animator *m_animator{nullptr};

  public:
    Clock(Adafruit_NeoPixel &leds, ClockState_e &state) : m_leds(leds), m_digitMgr(leds), m_state(state)
    {
        RedrawIfNeeded(true);
    }

    void UseAnimation(const AnimationType_e type)
    {
        if (m_animator)
        {
            delete m_animator;
        }

        m_animator = AnimatorFactory(m_digitMgr, type);
    }

    void ChangeDigitType()
    {
        m_digitMgr.CreateDigitDisplay();
    }

    void UpdateDigits()
    {
        if (Settings::Get(SETTING_24_HOUR_MODE) == 1)
        {
            m_digitMgr.numbers[0] = rtc_hal_hour() / 10;
            m_digitMgr.numbers[1] = rtc_hal_hour() % 10;
        }
        else
        {
            m_digitMgr.numbers[0] = rtc_hal_hourFormat12() / 10;
            m_digitMgr.numbers[1] = rtc_hal_hourFormat12() % 10;

            if (m_digitMgr.numbers[0] == 0)
            {
                // disable leading 0 for 12 hour mode
                m_digitMgr.numbers[0] = Digit::INVALID;
            }
        }

        m_digitMgr.numbers[2] = rtc_hal_minute() / 10;
        m_digitMgr.numbers[3] = rtc_hal_minute() % 10;

        m_digitMgr.numbers[4] = rtc_hal_second() / 10;
        m_digitMgr.numbers[5] = rtc_hal_second() % 10;

        if (m_animator)
        {
            m_animator->Go();
        }

        m_digitMgr.Draw();
    }

    void RedrawIfNeeded(bool force = false)
    {
        if (m_state == STATE_NORMAL)
        {
            rtc_hal_update();
        }

        if (m_state != STATE_NORMAL || (m_animator && m_animator->IsFast()))
        {
            force = true;
        }

        if (rtc_hal_second() != m_lastRedrawTime || force)
        {
            m_lastRedrawTime = rtc_hal_second();
            UpdateDigits();

            if (Settings::Get(SETTING_BLINKING_SEPARATORS))
            {
                BlinkDigitSeparators();
            }

            m_leds.show();
        }
    }

    void BlinkDigitSeparators()
    {
        if (Settings::Get(SETTING_BLINKING_SEPARATORS) != 1)
        {
            return;
        }

        int blinkColor = ColorWheel(Settings::Get(SETTING_COLOR));
        if (rtc_hal_second() % 2 != 0)
        {
            blinkColor = 0;
        }

        if (Settings::Get(SETTING_DIGIT_TYPE) == 1)
        {
            m_leds.setPixelColor(BLINK_DIGIT_TYPE_1_LED_1, blinkColor);
            m_leds.setPixelColor(BLINK_DIGIT_TYPE_1_LED_2, blinkColor);
        }
        else
        {
            m_leds.setPixelColor(BLINK_DIGIT_TYPE_2_LED_1, blinkColor);
            m_leds.setPixelColor(BLINK_DIGIT_TYPE_2_LED_2, blinkColor);
            m_leds.setPixelColor(BLINK_DIGIT_TYPE_2_LED_3, blinkColor);
            m_leds.setPixelColor(BLINK_DIGIT_TYPE_2_LED_4, blinkColor);
        }
    }
};
