#pragma once
#include <memory>

#include "animator.hpp"
#include "blinkers.hpp"
#include "button.hpp"
#include "digit_manager.hpp"
#include "elapsed_time.hpp"
#include "reversible_neopixels.hpp"
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

    Settings m_settings;
    ReversibleNeopixels m_leds{m_settings, NUM_LEDS, PIN_FOR_LEDS, NEO_GRB + NEO_KHZ400};
    DigitManager m_digitMgr{m_leds, m_settings};
    Blinkers m_blinkers{m_leds, m_settings};
    ClockState_e m_state{STATE_NORMAL};

    Button m_btnSetTime{PIN_BTN_H};
    Button m_btnHour{PIN_BTN_H};
    Button m_btnMinute{PIN_BTN_M};
    Button m_btnAnimationMode{PIN_BTN_M};
    Button m_btnColor{PIN_BTN_C};
    Button m_btnBrightness{PIN_BTN_B};
    Button m_btnToggle24HMode{{PIN_BTN_H, PIN_BTN_M}};
    Button m_btnToggleDisplay{{PIN_BTN_M, PIN_BTN_C}};
    Button m_btnToggleBlinkers{{PIN_BTN_M, PIN_BTN_B}};
    Button m_btnFlipDisplay{{PIN_BTN_C, PIN_BTN_B}};

    std::vector<Button *> m_buttons;

    Numbers_t m_alternateNumbers;
    ElapsedTime m_timeInAltDisplayMode;

  public:
    Clock()
    {
        Serial.begin(115200);
        rtc_hal_init();

        // initialize Adafruit's Neopixel library
        m_leds.begin();
        m_leds.setBrightness(m_settings.Get(SETTING_CUR_BRIGHTNESS));

        m_btnHour.SetEnabled(false);
        m_btnMinute.SetEnabled(false);
        m_btnMinute.config.canRepeat = true;
        m_btnMinute.config.repeatRate = 50;
        m_btnColor.config.canRepeat = true;
        m_btnColor.config.repeatRate = 200;
        m_btnBrightness.config.canRepeat = true;

        // special behavior buttons that require being held to activate
        m_btnSetTime.config.delayBeforePress = DELAY_FOR_COMBINATION_BUTTONS;
        m_btnToggle24HMode.config.delayBeforePress = DELAY_FOR_COMBINATION_BUTTONS;
        m_btnToggleDisplay.config.delayBeforePress = DELAY_FOR_COMBINATION_BUTTONS;
        m_btnToggleBlinkers.config.delayBeforePress = DELAY_FOR_COMBINATION_BUTTONS;
        m_btnFlipDisplay.config.delayBeforePress = DELAY_FOR_COMBINATION_BUTTONS;

        // store all our buttons in a vector so CheckForButtonEvents can
        // use a for loop to check all the buttons
        m_buttons.push_back(&m_btnSetTime);
        m_buttons.push_back(&m_btnHour);
        m_buttons.push_back(&m_btnMinute);
        m_buttons.push_back(&m_btnAnimationMode);
        m_buttons.push_back(&m_btnColor);
        m_buttons.push_back(&m_btnBrightness);
        m_buttons.push_back(&m_btnToggle24HMode);
        m_buttons.push_back(&m_btnToggleDisplay);
        m_buttons.push_back(&m_btnToggleBlinkers);
        m_buttons.push_back(&m_btnFlipDisplay);

        ConfigureButtonHandlers();
    }

    void Loop()
    {
        CheckForButtonEvents();
        DisplayDigits();
        m_blinkers.Update();

        m_leds.show();
    }

    void DisplayTemporarily(const unsigned int value)
    {
        m_state = STATE_ALT_DISPLAY;
        m_digitMgr.UseAnimation(ANIM_ALT_DISPLAY);
        m_timeInAltDisplayMode.Reset();
        m_alternateNumbers = ConvertValueToSeparateNumbers(value);
    }

    Numbers_t ConvertValueToSeparateNumbers(unsigned int value) const
    {
        Numbers_t numbers(NUM_DIGITS, Digit::INVALID);
        int digitNum = NUM_DIGITS - 1;
        do
        {
            numbers[digitNum--] = value % 10;
            value /= 10;
        } while (value && digitNum >= 0);
        return numbers;
    }

    Numbers_t GetNumbersFromRTC() const
    {
        Numbers_t numbers(NUM_DIGITS, Digit::INVALID);

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
            // don't display leading zero in 12H mode
            if (rtc_hal_hourFormat12() >= 10)
            {
                numbers[0] = rtc_hal_hourFormat12() / 10;
            }

            numbers[1] = rtc_hal_hourFormat12() % 10;
        }

        numbers[2] = rtc_hal_minute() / 10;
        numbers[3] = rtc_hal_minute() % 10;

        numbers[4] = rtc_hal_second() / 10;
        numbers[5] = rtc_hal_second() % 10;

        return numbers;
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
            // no break -- fall through to STATE_SET_TIME

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
    }

    void ConfigureButtonHandlers()
    {
        // These C++ lambda are used as callbacks for each button and are
        // called automatically by the Button class when events occur
        // such as pressing a button, releasing it, repeat, etc.

        ///////////////////////////////////////////////////////////////////////
        // Set time button (H button, when held)
        ///////////////////////////////////////////////////////////////////////
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
                    m_btnToggleDisplay.SetEnabled(false);
                    m_btnToggleBlinkers.SetEnabled(false);
                    m_btnToggle24HMode.SetEnabled(false);
                    m_btnFlipDisplay.SetEnabled(false);
                }
                else
                {
                    m_btnHour.SetEnabled(false);
                    m_btnMinute.SetEnabled(false);
                    m_btnAnimationMode.SetEnabled(true);
                    m_btnToggleDisplay.SetEnabled(true);
                    m_btnToggleBlinkers.SetEnabled(true);
                    m_btnToggle24HMode.SetEnabled(true);
                    m_btnFlipDisplay.SetEnabled(true);
                }
            }
        };

        ///////////////////////////////////////////////////////////////////////
        // Hour and minute buttons (only when in STATE_SET_TIME)
        ///////////////////////////////////////////////////////////////////////
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

        ///////////////////////////////////////////////////////////////////////
        // Animation button
        ///////////////////////////////////////////////////////////////////////
        m_btnAnimationMode.config.handlerFunc = [&](const Button::Event_e evt) {
            if (evt == Button::RELEASE)
            {
                m_settings.Set(SETTING_ANIMATION_TYPE, m_settings.Get(SETTING_ANIMATION_TYPE) + 1);
                if (m_settings.Get(SETTING_ANIMATION_TYPE) >= ANIM_USER_ACCESSIBLE_TOTAL)
                {
                    m_settings.Set(SETTING_ANIMATION_TYPE, ANIM_NONE);
                }
                m_settings.Save();

                DisplayTemporarily(m_settings.Get(SETTING_ANIMATION_TYPE));
            }
        };

        ///////////////////////////////////////////////////////////////////////
        // Color button
        ///////////////////////////////////////////////////////////////////////
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

        ///////////////////////////////////////////////////////////////////////
        // Toggle 24H mode on and off by pressing H+M
        ///////////////////////////////////////////////////////////////////////
        m_btnToggle24HMode.config.handlerFunc = [&](const Button::Event_e evt) {
            if (evt == Button::PRESS)
            {
                m_btnSetTime.SetEnabled(false);
                m_btnAnimationMode.SetEnabled(false);

                const auto mode = m_settings.Get(SETTING_24_HOUR_MODE);
                m_settings.Set(SETTING_24_HOUR_MODE, mode == 0 ? 1 : 0);
                m_settings.Save();
                m_digitMgr.CreateDigits();
            }
            else if (evt == Button::RELEASE)
            {
                m_btnSetTime.SetEnabled(true);
                m_btnAnimationMode.SetEnabled(true);
            }
        };

        ///////////////////////////////////////////////////////////////////////
        // Toggle display between PXL and Edge lit using C+B
        ///////////////////////////////////////////////////////////////////////
        m_btnToggleDisplay.config.handlerFunc = [&](const Button::Event_e evt) {
            if (evt == Button::PRESS)
            {
                m_blinkers.TurnOffBlinkers();

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
        };

        ///////////////////////////////////////////////////////////////////////
        // Toggle blinkers on and off by pressing B+C
        ///////////////////////////////////////////////////////////////////////
        m_btnToggleBlinkers.config.handlerFunc = [&](const Button::Event_e evt) {
            if (evt == Button::PRESS)
            {
                m_btnBrightness.SetEnabled(false);
                m_btnAnimationMode.SetEnabled(false);

                const auto mode = m_settings.Get(SETTING_BLINKING_SEPARATORS);
                m_settings.Set(SETTING_BLINKING_SEPARATORS, mode == 0 ? 1 : 0);
                m_settings.Save();
            }
            else if (evt == Button::RELEASE)
            {
                m_btnBrightness.SetEnabled(true);
                m_btnAnimationMode.SetEnabled(true);
            }
        };

        ///////////////////////////////////////////////////////////////////////
        // Brightness button, when held, stop changing when at full brightness
        ///////////////////////////////////////////////////////////////////////
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

        ///////////////////////////////////////////////////////////////////////
        // Flip display by pressing C+B
        ///////////////////////////////////////////////////////////////////////
        m_btnFlipDisplay.config.handlerFunc = [&](const Button::Event_e evt) {
            if (evt == Button::PRESS)
            {
                m_btnColor.SetEnabled(false);
                m_btnBrightness.SetEnabled(false);

                const auto flip = m_settings.Get(SETTING_FLIP_DISPLAY);
                m_settings.Set(SETTING_FLIP_DISPLAY, flip == 0 ? 1 : 0);
                m_settings.Save();
                m_digitMgr.CreateDigits();
            }
            else if (evt == Button::RELEASE)
            {
                m_btnColor.SetEnabled(true);
                m_btnBrightness.SetEnabled(true);
            }
        };
    }
};
