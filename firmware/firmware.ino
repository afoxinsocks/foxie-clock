#include <Wire.h>
#include <vector>

#include "Adafruit_NeoPixel.h"
#include "ble_funcs.hpp"
#include "button.hpp"
#include "clock.hpp"
#include "cmd_handler.hpp"
#include "digit_manager.hpp"
#include "flippable_led_strip.hpp"
#include "rtc_hal.hpp"
#include "settings.hpp"

Settings *Settings::m_inst = nullptr;
CmdHandler *CmdHandler::m_inst = nullptr;
std::vector<Button *> Button::m_buttons;

enum HardwareConfig_e
{
    PIN_LEDS = 2, // pin "A2" on RedBoard Nano, "LEDs" on the PCB
    NUM_LEDS = 122,

    PIN_BTN_HOUR = 3,       // pin "A3" -- "HR" on PCB
    PIN_BTN_MIN = 4,        // pin "4" -- "MIN" on PCB
    PIN_BTN_COLOR = 5,      // pin "A5" -- "CLR" on PCB
    PIN_BTN_BRIGHTNESS = 6, // pin "6" -- "BRT" on PCB
    NUM_BTNS = 4,

    HOLD_SET_TIME_BTN_DELAY = 1000,
    HELD_BTN_DELAY = 2500,
};

void setup()
{
    Serial.begin(115200);
    Wire.begin();

    Settings settings;
    if (false) // change to 'true' to allow the below code to change the settings on boot
    {

        // NOTE: These settings are stored while the system is off. The mobile app
        // can control any of these settings and this section will be removed once
        // the mobile app launches.

        Settings::Set(SETTING_DIGIT_TYPE, DT_EDGE_LIT);
        Settings::Set(SETTING_CUR_BRIGHTNESS, 128);
        Settings::Set(SETTING_MIN_BRIGHTNESS, 4);
        Settings::Set(SETTING_MAX_BRIGHTNESS, 255);
        Settings::Set(SETTING_BLINKING_SEPARATORS, 1);
        Settings::Set(SETTING_COLOR, 192);

        Settings::Set(SETTING_ANIMATION_TYPE, ANIM_NONE);
        // Settings::Set(SETTING_ANIMATION_TYPE, ANIM_GLOW);
        // Settings::Set(SETTING_ANIMATION_TYPE, ANIM_CYCLE_COLORS);
        // Settings::Set(SETTING_ANIMATION_TYPE, ANIM_CYCLE_FLOW_LEFT);

        // Settings::Set(SETTING_24_HOUR_MODE, 1);

        // Settings::Set(SETTING_FLIP_DISPLAY, 1);
        // Settings::Set(SETTING_TRANSITION_TYPE, 1);

        // Settings::Save(); // UNCOMMENT to save these settings into flash
    }

    // temporarily force fade transitions always on (set to 0 to disable)
    Settings::Set(SETTING_TRANSITION_TYPE, 1);

    rtc_hal_init();

    FlippableLEDStrip leds(NUM_LEDS, PIN_LEDS, NEO_GRB + NEO_KHZ400);

    leds.begin(); // initialize NeoPixel library
    leds.setBrightness(settings.Get(SETTING_CUR_BRIGHTNESS));

    ClockState_e clockState{STATE_NORMAL};
    Clock clock(leds, clockState);

    CmdHandler cmdHandler(clock);

    Button btnSetTime(PIN_BTN_HOUR);
    btnSetTime.SetRepeatRate(HOLD_SET_TIME_BTN_DELAY);

    Button btnHour(PIN_BTN_HOUR);
    btnHour.SetAllowRepeat(false);
    btnHour.SetEnabled(false);

    Button btnMinute(PIN_BTN_MIN);
    btnMinute.SetAllowRepeat(true);
    btnMinute.SetRepeatRate(100);
    btnMinute.SetEnabled(false);

    Button btnSetAnimationMode(PIN_BTN_MIN); // M... for Mode! ... :)
    btnSetAnimationMode.SetAllowRepeat(false);

    Button btnColor(PIN_BTN_COLOR);
    Button btnBrightness(PIN_BTN_BRIGHTNESS);

    bool sleepMode = false;

    btnSetTime.SetHandlerFunc([&](const Button::Event_e evt) {
        if (evt == Button::REPEAT)
        {
            btnSetTime.SetAllowRepeat(false);
            if (clockState == STATE_NORMAL)
            {
                clockState = STATE_SET_TIME;
                clock.UseAnimation(ANIM_SET_TIME);
            }
            else
            {
                clock.UseAnimation((AnimationType_e)Settings::Get(SETTING_ANIMATION_TYPE));
                clockState = STATE_NORMAL;

                rtc_hal_setTime(rtc_hal_hour(), rtc_hal_minute(), rtc_hal_second());
                clock.Draw();
            }
        }
        else if (evt == Button::RELEASE && btnSetTime.TimePressed() > HOLD_SET_TIME_BTN_DELAY)
        {
            btnSetTime.SetAllowRepeat(true);
            btnHour.SetEnabled(clockState == STATE_SET_TIME);
            btnMinute.SetEnabled(clockState == STATE_SET_TIME);

            Settings::Set(SETTING_TRANSITION_TYPE, (clockState == STATE_NORMAL));

            btnSetAnimationMode.SetEnabled(clockState == STATE_NORMAL);
        }

        clock.Draw();
    });

    btnHour.SetHandlerFunc([&](const Button::Event_e evt) {
        if (evt == Button::RELEASE)
        {
            rtc_hal_setTime(rtc_hal_hour() + 1, rtc_hal_minute(), rtc_hal_second());
            clock.UpdateDigitsFromRTC();
        }
    });

    btnMinute.SetHandlerFunc([&](const Button::Event_e evt) {
        if (evt == Button::RELEASE || evt == Button::REPEAT)
        {
            rtc_hal_setTime(rtc_hal_hour(), rtc_hal_minute() + 1, rtc_hal_second());
            clock.UpdateDigitsFromRTC();
        }
    });

    btnSetAnimationMode.SetHandlerFunc([&](const Button::Event_e evt) {
        if (evt == Button::RELEASE)
        {
            settings.Set(SETTING_ANIMATION_TYPE, settings.Get(SETTING_ANIMATION_TYPE) + 1);
            if (settings.Get(SETTING_ANIMATION_TYPE) >= ANIM_TOTAL)
            {
                settings.Set(SETTING_ANIMATION_TYPE, ANIM_NONE);
            }
            settings.Save();
            clock.UseAnimation((AnimationType_e)settings.Get(SETTING_ANIMATION_TYPE));

            clock.DisplayValue(settings.Get(SETTING_ANIMATION_TYPE) * 1);
        }
        else if (evt == Button::HELD && btnSetAnimationMode.TimePressed() > HELD_BTN_DELAY)
        {
            btnSetAnimationMode.DisableEventsUntilRelease();

            if (Settings::Get(SETTING_DIGIT_TYPE) == DT_EDGE_LIT)
            {
                Settings::Set(SETTING_DIGIT_TYPE, DT_PIXELS);
            }
            else
            {
                Settings::Set(SETTING_DIGIT_TYPE, DT_EDGE_LIT);
            }

            clock.ChangeDigitType();
        }
    });

    btnColor.SetHandlerFunc([&](const Button::Event_e evt) {
        if (evt == Button::PRESS || evt == Button::REPEAT)
        {
            settings.Set(SETTING_COLOR, (settings.Get(SETTING_COLOR) + 8) & 0xFF);
            clock.ColorButtonPressed();
        }
        else if (evt == Button::RELEASE)
        {
            settings.Save();
        }
    });

    btnBrightness.SetHandlerFunc([&](const Button::Event_e evt) {
        if (evt == Button::PRESS || evt == Button::REPEAT)
        {
            if (btnSetAnimationMode.IsPressed())
            {
                btnSetAnimationMode.DisableEventsUntilRelease();
                return;
            }

            const uint8_t step = settings.Get(SETTING_MAX_BRIGHTNESS) / 8;
            settings.Set(SETTING_CUR_BRIGHTNESS, settings.Get(SETTING_CUR_BRIGHTNESS) + step);
            if (settings.Get(SETTING_CUR_BRIGHTNESS) > settings.Get(SETTING_MAX_BRIGHTNESS))
            {
                settings.Set(SETTING_CUR_BRIGHTNESS, settings.Get(SETTING_MIN_BRIGHTNESS));
            }
            clock.SetBrightness();
        }
        else if (evt == Button::RELEASE)
        {
            if (btnSetAnimationMode.IsPressed())
            {
                leds.FadeToOff();
                sleepMode = true;
                Button::WaitForAllButtonsToBeReleased();
            }
            else
            {

                settings.Save();
            }
        }
    });

    // this loop is used instead of loop(), so we can use our local variables
    bool initializedBluetooth = false;
    while (true)
    {
        if (!sleepMode)
        {
            if (!initializedBluetooth && millis() > 500)
            {
                BluetoothInit();
                initializedBluetooth = true;
            }

            if (initializedBluetooth)
            {
                BluetoothProcessing();
            }

            clock.Check();

            Button::CheckAll();
        }
        else if (sleepMode && Button::AreAnyButtonsPressed())
        {
            leds.SetToCurrentBrightness();
            sleepMode = false;
            Button::WaitForAllButtonsToBeReleased();
        }
    }
}

void loop()
{
    // will never be used, while(true) loop above used instead
}