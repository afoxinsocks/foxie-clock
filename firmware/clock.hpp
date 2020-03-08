#pragma once
#include "Adafruit_NeoPixel.h"
#include "animator.hpp"
#include "digit_manager.hpp"
#include "rtc_hal.hpp"
#include "settings.hpp"
#include <memory>

enum ClockState_e
{
    STATE_NORMAL,
    STATE_SET_TIME,
    STATE_DISPLAY_VALUE,
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
    std::shared_ptr<Animator> m_animator;

    int m_millisSinceDisplayValueEntered{0};

  public:
    Clock(Adafruit_NeoPixel &leds, ClockState_e &state) : m_leds(leds), m_digitMgr(leds), m_state(state)
    {
        UseAnimation((AnimationType_e)Settings::Get(SETTING_ANIMATION_TYPE));
        Draw();
    }

    void UseAnimation(const AnimationType_e type)
    {
        m_animator = AnimatorFactory(m_digitMgr, type);
    }

    void ColorButtonPressed()
    {
        m_animator->ColorButtonPressed();
        Draw();
    }

    void ChangeDigitType()
    {
        TurnOffBlinkers();
        m_digitMgr.CreateDigitDisplay();
    }

    void SetBrightness()
    {
        m_leds.setBrightness(Settings::Get(SETTING_CUR_BRIGHTNESS));
        m_leds.show();
    }

    void DisplayValue(unsigned int value)
    {
        m_state = STATE_DISPLAY_VALUE;
        m_millisSinceDisplayValueEntered = millis();
        UseAnimation(ANIM_NONE);

        for (auto &num : m_digitMgr.numbers)
        {
            num = Digit::INVALID;
        }

        int digitNum = 5;
        do
        {
            m_digitMgr.numbers[digitNum--] = value % 10;
            value /= 10;
        } while (value && digitNum >= 0);

        m_animator->Go();
    }

    void UpdateDigitsFromRTC()
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

        m_animator->Go();

        m_digitMgr.Draw();

        BlinkDigitSeparators();
    }

    void Check()
    {
        bool update = false;

        if (m_state == STATE_DISPLAY_VALUE && ElapsedMsSinceDisplayModeStarted() >= 1000)
        {
            m_state = STATE_NORMAL;
            UseAnimation((AnimationType_e)Settings::Get(SETTING_ANIMATION_TYPE));
        }

        if (m_state == STATE_NORMAL)
        {
            rtc_hal_update();
        }
        else
        {
            // whenever not in normal mode, always update
            update = true;
        }

        if (rtc_hal_second() != m_lastRedrawTime)
        {
            update = true;
        }

        if (update || m_animator->IsFast())
        {
            m_lastRedrawTime = rtc_hal_second();
            Draw();
        }
    }

    void Draw()
    {
        if (m_state != STATE_DISPLAY_VALUE)
        {
            UpdateDigitsFromRTC();
        }
        else
        {
            m_digitMgr.Draw();
        }

        m_leds.show();
    }

    void BlinkDigitSeparators()
    {
        int blinkColor = ColorWheel(Settings::Get(SETTING_COLOR));
        if (rtc_hal_second() % 2 != 0 || Settings::Get(SETTING_BLINKING_SEPARATORS) == 0)
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

    void TurnOffBlinkers()
    {
        m_leds.setPixelColor(BLINK_DIGIT_TYPE_1_LED_1, 0);
        m_leds.setPixelColor(BLINK_DIGIT_TYPE_1_LED_2, 0);
        m_leds.setPixelColor(BLINK_DIGIT_TYPE_2_LED_1, 0);
        m_leds.setPixelColor(BLINK_DIGIT_TYPE_2_LED_2, 0);
        m_leds.setPixelColor(BLINK_DIGIT_TYPE_2_LED_3, 0);
        m_leds.setPixelColor(BLINK_DIGIT_TYPE_2_LED_4, 0);
    }

  private:
    int ElapsedMsSinceDisplayModeStarted()
    {
        return (int)millis() - m_millisSinceDisplayValueEntered;
    }
};
