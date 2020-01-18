#include <vector>

#include "Adafruit_NeoPixel.h"
#include "rtc_hal.hpp"
#include "digit_manager.hpp"
#include "button.hpp"
#include "ble_funcs.hpp"
#include "clock.hpp"
#include "settings.hpp"

enum HardwareConfig_e
{
    PIN_LEDS = 2, // pin "A2" on RedBoard Nano, "LEDs" on the PCB
    NUM_LEDS = 122,

    PIN_BTN_HOUR = 3, // pin "A3" -- "HR" on PCB
    PIN_BTN_MIN = 4, // pin "4" -- "MIN" on PCB
    PIN_BTN_COLOR = 5, // pin "A5" -- "CLR" on PCB
    PIN_BTN_BRIGHTNESS = 6, // pin "6" -- "BRT" on PCB
    NUM_BTNS = 4,

    HOLD_SET_TIME_BTN_DELAY = 1000,
};

Clock *g_clock = nullptr;
void setup()
{
    // TODO: Change this to a globally accessible class
    Settings settings;
    //settings.ResetToDefaults();

    Adafruit_NeoPixel leds(NUM_LEDS, PIN_LEDS, NEO_GRB + NEO_KHZ400);
    leds.begin(); // initialize NeoPixel library
    leds.setBrightness(settings.Get(SETTING_CUR_BRIGHTNESS));

    BluetoothInit();

    ClockState_e clockState{STATE_NORMAL};
    g_clock = new Clock(leds, clockState);
    g_clock->UseAnimation(ANIM_CYCLE_COLORS);

    Button btnSetTime(PIN_BTN_HOUR);
    btnSetTime.SetRepeatRate(HOLD_SET_TIME_BTN_DELAY);

    Button btnHour(PIN_BTN_HOUR);
    btnHour.SetAllowRepeat(false);
    btnHour.SetEnabled(false);

    Button btnMinute(PIN_BTN_MIN);
    btnMinute.SetAllowRepeat(false);
    btnMinute.SetEnabled(false);

    Button btnColor(PIN_BTN_COLOR);
    Button btnBrightness(PIN_BTN_BRIGHTNESS);


    btnSetTime.SetHandlerFunc([&](const Button::Event_e evt) {
        if (evt == Button::REPEAT)
        {
            btnSetTime.SetAllowRepeat(false);
            if (clockState == STATE_NORMAL)
            {
                clockState = STATE_SET_TIME;
            }
            else
            {
                clockState = STATE_NORMAL;
            }
        }
        else if (evt == Button::RELEASE && btnSetTime.TimePressed() > HOLD_SET_TIME_BTN_DELAY)
        {
            btnSetTime.SetAllowRepeat(true);
            btnHour.SetEnabled(clockState == STATE_SET_TIME);
            btnMinute.SetEnabled(clockState == STATE_SET_TIME);
        }

        g_clock->RedrawIfNeeded(true);
    });


    btnHour.SetHandlerFunc([&](const Button::Event_e evt) {
        if (evt == Button::RELEASE)
        {
            rtc_hal_setTime(rtc_hal_hour() + 1, 
                rtc_hal_minute(), rtc_hal_second());
            g_clock->RedrawIfNeeded(true);
        }
    });

    btnMinute.SetHandlerFunc([&](const Button::Event_e evt) {
        if (evt == Button::RELEASE || evt == Button::REPEAT)
        {
            rtc_hal_setTime(rtc_hal_hour(), 
                rtc_hal_minute() + 1, rtc_hal_second());
            g_clock->RedrawIfNeeded(true);
        }
    });

    btnColor.SetHandlerFunc([&](const Button::Event_e evt) {
        if (evt == Button::PRESS || evt == Button::REPEAT)
        {
            settings.Set(SETTING_COLOR, settings.Get(SETTING_COLOR) + 8);
            g_clock->RedrawIfNeeded(true);
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

        g_clock->RedrawIfNeeded();

        btnSetTime.Check();
        btnHour.Check();
        btnMinute.Check();

        btnColor.Check();
        btnBrightness.Check();
    }
}

void loop()
{
    // will never be used, while(true) loop above used instead
}


enum AlertCommands_e
{
    CMD_SET_TIME = 0x10,
};

enum AlertState_e
{
    AL_STATE_WAIT,
    AL_STATE_SET_TIME,
};
void handleSetTimeState(uint8_t val);

enum ClockSetState_e
{
    CS_HOUR,
    CS_MINUTE,
    CS_SECOND,
};

AlertState_e g_alertState = AL_STATE_WAIT;
extern "C" void alert(uint8_t val)
{
    switch (g_alertState)
    {
    case AL_STATE_WAIT:
        if (val == CMD_SET_TIME)
        {
            g_alertState = AL_STATE_SET_TIME;
        }
        break;

    case AL_STATE_SET_TIME:
        handleSetTimeState(val);

    default:
        break;
    }

    // TODO: Implement a timeout for this
}

void handleSetTimeState(uint8_t val)
{
    static ClockSetState_e state = CS_HOUR;

    static int h, m;

    switch (state)
    {
    case CS_HOUR:
        h = val;
        state = CS_MINUTE;
        break;

    case CS_MINUTE:
        m = val;
        state = CS_SECOND;
        break;

    case CS_SECOND:
        rtc_hal_setTime(h, m, val + 1);
        state = CS_HOUR;
        g_alertState = AL_STATE_WAIT;
        g_clock->RedrawIfNeeded(true);
        break;

    default:
        break;
    }
}