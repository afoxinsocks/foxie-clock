#pragma once
#include <memory>

#include "Adafruit_NeoPixel.h"
#include "animator.hpp"
#include "button.hpp"
#include "digit_manager.hpp"
#include "elapsed_time.hpp"
#include "rtc_hal.hpp"
#include "settings.hpp"

class Clock
{
  private:
    enum ClockState_e
    {
        STATE_NORMAL,
        STATE_SET_TIME,
        STATE_ALT_DISPLAY,
    };

    Adafruit_NeoPixel m_leds{NUM_LEDS, PIN_FOR_LEDS, NEO_GRB + NEO_KHZ400};
    Settings m_settings;
    DigitManager m_digitMgr;
    ClockState_e m_state{STATE_NORMAL};

    Button m_btnSetTime{PIN_BTN_H};
    Button m_btnHour{PIN_BTN_H};
    Button m_btnMinute{PIN_BTN_M};
    Button m_btnAnimationMode{PIN_BTN_M};
    Button m_btnColor{PIN_BTN_C};
    Button m_btnBrightness{PIN_BTN_B};
    Button m_btnToggleDisplay{{PIN_BTN_M, PIN_BTN_C}};
    std::vector<Button *> m_buttons;

    Numbers_t m_alternateNumbers;
    ElapsedTime m_timeInAltDisplayMode;

  public:
    Clock() : m_digitMgr(m_leds, m_settings)
    {
        Serial.begin(115200);
        rtc_hal_init();
        m_leds.begin();
        m_leds.setBrightness(m_settings.Get(SETTING_CUR_BRIGHTNESS));

        m_btnHour.SetEnabled(false);
        m_btnMinute.SetEnabled(false);
        m_btnMinute.config.canRepeat = true;
        m_btnColor.config.canRepeat = true;
        m_btnColor.config.repeatRate = 200;
        m_btnBrightness.config.canRepeat = true;

        // special behavior buttons
        m_btnSetTime.config.delayBeforePress = DELAY_FOR_SET_TIME_MODE;
        m_btnToggleDisplay.config.delayBeforePress = DELAY_FOR_TOGGLE_DISPLAY;

        m_buttons.push_back(&m_btnSetTime);
        m_buttons.push_back(&m_btnHour);
        m_buttons.push_back(&m_btnMinute);
        m_buttons.push_back(&m_btnAnimationMode);
        m_buttons.push_back(&m_btnColor);
        m_buttons.push_back(&m_btnBrightness);
        m_buttons.push_back(&m_btnToggleDisplay);

        ConfigureButtonHandlers();
    }

    void Loop()
    {
        CheckForButtonEvents();
        DisplayDigits();
    }

    void AlternateDisplay(unsigned int value)
    {
        m_state = STATE_ALT_DISPLAY;
        m_timeInAltDisplayMode.Reset();

        m_digitMgr.UseAnimation(ANIM_NONE);

        m_alternateNumbers.resize(NUM_DIGITS, Digit::INVALID);

        int digitNum = 5;
        do
        {
            m_alternateNumbers[digitNum--] = value % 10;
            value /= 10;
        } while (value && digitNum >= 0);
    }

  private:
    void CheckForButtonEvents()
    {
        for (auto &button : m_buttons)
        {
            button->Update();
        }
    }

    void DisplayDigits()
    {
        Numbers_t numbers;

        switch (m_state)
        {
        case STATE_NORMAL:
            rtc_hal_update();
            // no break -- fall through

        case STATE_SET_TIME:
            numbers = GetNumbersFromRTC();
            m_digitMgr.Display(numbers);
            break;

        case STATE_ALT_DISPLAY:
            m_digitMgr.Display(m_alternateNumbers);
            if (m_timeInAltDisplayMode.Ms() > 1000)
            {
                m_state = STATE_NORMAL;
                m_digitMgr.UseAnimation((AnimationType_e)m_settings.Get(SETTING_ANIMATION_TYPE));
            }
            break;
        }

        m_leds.show();
    }

    Numbers_t GetNumbersFromRTC() const
    {
        Numbers_t numbers(NUM_DIGITS, 0);

        // on a Foxie Clock, the numbers are laid out this way:
        // numbers[0] is the leftmost digit
        // numbers[5] is the rightmost digit
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

    void ConfigureButtonHandlers()
    {
        // TODO: Add a bit about what I'm doing here with these lambda functions

        m_btnSetTime.config.handlerFunc = [&](const Button::Event_e evt) {
            if (evt == Button::PRESS)
            {
                if (m_state == STATE_NORMAL)
                {
                    m_state = STATE_SET_TIME;
                    m_digitMgr.UseAnimation(ANIM_SET_TIME);
                }
                else
                {
                    m_state = STATE_NORMAL;
                    m_digitMgr.UseAnimation((AnimationType_e)m_settings.Get(SETTING_ANIMATION_TYPE));
                    rtc_hal_setTime(rtc_hal_hour(), rtc_hal_minute(), rtc_hal_second());
                }
            }
            else if (evt == Button::RELEASE)
            {
                if (m_state == STATE_SET_TIME)
                {
                    m_btnHour.SetEnabled(true);
                    m_btnMinute.SetEnabled(true);
                    m_btnAnimationMode.SetEnabled(false);
                }
                else
                {
                    m_btnHour.SetEnabled(false);
                    m_btnMinute.SetEnabled(false);
                    m_btnAnimationMode.SetEnabled(true);
                }
            }
        };

        m_btnHour.config.handlerFunc = [&](const Button::Event_e evt) {
            if (evt == Button::RELEASE)
            {
                rtc_hal_setTime(rtc_hal_hour() + 1, rtc_hal_minute(), rtc_hal_second());
            }
        };

        m_btnMinute.config.handlerFunc = [&](const Button::Event_e evt) {
            if (evt == Button::REPEAT || evt == Button::RELEASE)
            {
                rtc_hal_setTime(rtc_hal_hour(), rtc_hal_minute() + 1, rtc_hal_second());
            }
        };

        m_btnAnimationMode.config.handlerFunc = [&](const Button::Event_e evt) {
            if (evt == Button::RELEASE)
            {
                m_settings.Set(SETTING_ANIMATION_TYPE, m_settings.Get(SETTING_ANIMATION_TYPE) + 1);
                if (m_settings.Get(SETTING_ANIMATION_TYPE) >= ANIM_TOTAL)
                {
                    m_settings.Set(SETTING_ANIMATION_TYPE, ANIM_NONE);
                }
                m_settings.Save();
                m_digitMgr.UseAnimation((AnimationType_e)m_settings.Get(SETTING_ANIMATION_TYPE));
                AlternateDisplay(m_settings.Get(SETTING_ANIMATION_TYPE));
            }
        };

        m_btnColor.config.handlerFunc = [&](const Button::Event_e evt) {
            if (evt == Button::REPEAT || evt == Button::RELEASE)
            {
                m_settings.Set(SETTING_COLOR, (m_settings.Get(SETTING_COLOR) + 8) & 0xFF);
                m_digitMgr.ColorButtonPressed(m_settings.Get(SETTING_COLOR));
            }

            if (evt == Button::RELEASE)
            {
                m_settings.Save();
            }
        };

        m_btnToggleDisplay.config.handlerFunc = [&](const Button::Event_e evt) {
            if (evt == Button::PRESS)
            {
                m_btnAnimationMode.SetEnabled(false);
                m_btnColor.SetEnabled(false);
                if (m_settings.Get(SETTING_DIGIT_TYPE) == DT_EDGE_LIT)
                {
                    m_settings.Set(SETTING_DIGIT_TYPE, DT_PIXELS);
                }
                else
                {
                    m_settings.Set(SETTING_DIGIT_TYPE, DT_EDGE_LIT);
                }

                m_settings.Save();
                m_digitMgr.CreateDigits();
            }
            else
            {
                m_btnAnimationMode.SetEnabled(true);
                m_btnColor.SetEnabled(true);
            }
        };

        m_btnBrightness.config.handlerFunc = [&](const Button::Event_e evt) {
            if (evt == Button::PRESS || evt == Button::REPEAT)
            {
                const uint8_t step = m_settings.Get(SETTING_MAX_BRIGHTNESS) / 8;
                m_settings.Set(SETTING_CUR_BRIGHTNESS, m_settings.Get(SETTING_CUR_BRIGHTNESS) + step);

                if (m_settings.Get(SETTING_CUR_BRIGHTNESS) > m_settings.Get(SETTING_MAX_BRIGHTNESS))
                {
                    if (evt == Button::PRESS)
                    {
                        m_settings.Set(SETTING_CUR_BRIGHTNESS, m_settings.Get(SETTING_MIN_BRIGHTNESS));
                    }
                    else
                    {
                        m_settings.Set(SETTING_CUR_BRIGHTNESS, m_settings.Get(SETTING_MAX_BRIGHTNESS));
                    }
                }

                m_leds.setBrightness(m_settings.Get(SETTING_CUR_BRIGHTNESS));
            }
        };
    }
};
