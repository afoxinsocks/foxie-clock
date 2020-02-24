#include <vector>

#include "Adafruit_NeoPixel.h"
#include "ble_funcs.hpp"
#include "button.hpp"
#include "clock.hpp"
#include "cmd_handler.hpp"
#include "digit_manager.hpp"
#include "rtc_hal.hpp"
#include "settings.hpp"

Settings *Settings::m_inst = nullptr;
CmdHandler *CmdHandler::m_inst = nullptr;

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
};

void setup()
{
    Serial.begin(115200);

    Settings settings;
    if (false) // change to 'true' to allow the below code to change the settings on boot
    {

        // NOTE: These settings are stored while the system is off. The mobile app
        // can control any of these settings and this section will be removed once
        // the mobile app launches.

        Settings::Set(SETTING_DIGIT_TYPE, 1); // 1 is edge lit (acrylics), 2 is pixel display
        Settings::Set(SETTING_CUR_BRIGHTNESS, 128);
        Settings::Set(SETTING_MIN_BRIGHTNESS, 4);
        Settings::Set(SETTING_MAX_BRIGHTNESS, 255);
        Settings::Set(SETTING_BLINKING_SEPARATORS, 1);
        Settings::Set(SETTING_COLOR, 192);

        Settings::Set(SETTING_ANIMATION_TYPE, ANIM_NONE);
        // Settings::Set(SETTING_ANIMATION_TYPE, ANIM_GLOW);
        // Settings::Set(SETTING_ANIMATION_TYPE, ANIM_CYCLE_COLORS);
        // Settings::Set(SETTING_ANIMATION_TYPE, ANIM_CYCLE_FLOW_LEFT);

        Settings::Set(SETTING_24_HOUR_MODE, 0);

        // Settings::Save(); // UNCOMMENT to save these settings into flash
    }

    rtc_hal_init();

    Adafruit_NeoPixel leds(NUM_LEDS, PIN_LEDS, NEO_GRB + NEO_KHZ400);
    leds.begin(); // initialize NeoPixel library
    leds.setBrightness(settings.Get(SETTING_CUR_BRIGHTNESS));

    BluetoothInit();

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

            // disable the animation mode button
            btnSetAnimationMode.SetEnabled(clockState == STATE_NORMAL);
        }

        clock.Draw();
    });

    btnHour.SetHandlerFunc([&](const Button::Event_e evt) {
        if (evt == Button::RELEASE)
        {
            rtc_hal_setTime(rtc_hal_hour() + 1, rtc_hal_minute(), rtc_hal_second());
            clock.Draw();
        }
    });

    btnMinute.SetHandlerFunc([&](const Button::Event_e evt) {
        if (evt == Button::RELEASE || evt == Button::REPEAT)
        {
            rtc_hal_setTime(rtc_hal_hour(), rtc_hal_minute() + 1, rtc_hal_second());
            clock.Draw();
        }
    });

    btnSetAnimationMode.SetHandlerFunc([&](const Button::Event_e evt) {
        if (evt == Button::RELEASE)
        {
            settings.Set(SETTING_ANIMATION_TYPE, settings.Get(SETTING_ANIMATION_TYPE) + 1);
            if (settings.Get(SETTING_ANIMATION_TYPE) == ANIM_TOTAL)
            {
                settings.Set(SETTING_ANIMATION_TYPE, ANIM_NONE);
            }
            settings.Save();
            clock.UseAnimation((AnimationType_e)settings.Get(SETTING_ANIMATION_TYPE));

            clock.DisplayValue(settings.Get(SETTING_ANIMATION_TYPE) * 1);
        }
    });

    btnColor.SetHandlerFunc([&](const Button::Event_e evt) {
        if (evt == Button::PRESS || evt == Button::REPEAT)
        {
            settings.Set(SETTING_COLOR, settings.Get(SETTING_COLOR) + 8);
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
            const uint8_t step = settings.Get(SETTING_MAX_BRIGHTNESS) / 8;
            settings.Set(SETTING_CUR_BRIGHTNESS, settings.Get(SETTING_CUR_BRIGHTNESS) + step);
            if (settings.Get(SETTING_CUR_BRIGHTNESS) > settings.Get(SETTING_MAX_BRIGHTNESS))
            {
                settings.Set(SETTING_CUR_BRIGHTNESS, settings.Get(SETTING_MIN_BRIGHTNESS));
            }
            leds.setBrightness(settings.Get(SETTING_CUR_BRIGHTNESS));
            leds.show();
        }
        else if (evt == Button::RELEASE)
        {
            settings.Save();
        }
    });

    // this loop is used instead of loop(), so we can use our local variables
    while (true)
    {
        BluetoothProcessing();

        clock.Check();

        btnSetTime.Check();
        btnHour.Check();
        btnMinute.Check();
        btnSetAnimationMode.Check();
        btnColor.Check();
        btnBrightness.Check();
    }
}

void loop()
{
    // will never be used, while(true) loop above used instead
}
