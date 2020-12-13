#pragma once
#include <memory>

#include "Adafruit_NeoPixel.h"
#include "button.hpp"
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
        m_btnMinute.config.repeatRate = 100;

        m_btnSetTime.config.delayBeforePress = DELAY_FOR_SET_TIME_MODE;
        m_btnToggleDisplay.config.delayBeforePress = DELAY_FOR_DISPLAY_TOGGLE;

        ConfigureButtonHandlers();

        m_buttons.push_back(&m_btnSetTime);
        m_buttons.push_back(&m_btnHour);
        m_buttons.push_back(&m_btnMinute);
        m_buttons.push_back(&m_btnAnimationMode);
        m_buttons.push_back(&m_btnColor);
        m_buttons.push_back(&m_btnBrightness);
        m_buttons.push_back(&m_btnToggleDisplay);
    }

    void Loop()
    {
        CheckForButtonEvents();

        DisplayDigits();
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
        if (m_state == STATE_NORMAL)
        {
            rtc_hal_update();
        }

        const auto numbers = GetNumbersFromRTC();
        m_digitMgr.Display(numbers);

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
        m_btnSetTime.config.handlerFunc = [&](const Button::Event_e evt) {
            if (evt == Button::PRESS)
            {
                if (m_state == STATE_NORMAL)
                {
                    m_state = STATE_SET_TIME;
                    m_settings.Set(SETTING_COLOR, 0x00FFFF);
                }
                else
                {
                    m_state = STATE_NORMAL;
                    m_settings.Set(SETTING_COLOR, 0x0000FF);
                    rtc_hal_setTime(rtc_hal_hour(), rtc_hal_minute(), rtc_hal_second());
                }

                // TODO: tell the digit manager to use a different animator instead of changing the colors above
                m_digitMgr.CreateDigits();
            }
            else if (evt == Button::RELEASE)
            {
                if (m_state == STATE_SET_TIME)
                {
                    m_btnHour.SetEnabled(true);
                    m_btnMinute.SetEnabled(true);
                }
                else
                {
                    m_btnHour.SetEnabled(false);
                    m_btnMinute.SetEnabled(false);
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
    }
};
