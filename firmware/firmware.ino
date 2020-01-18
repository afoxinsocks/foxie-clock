#include <vector>

#include "Adafruit_NeoPixel.h"
#include "rtc_hal.hpp"
#include "digit_manager.hpp"
#include "button.hpp"
#include "ble_funcs.hpp"
#include "settings.hpp"

enum
{
    PIN_LEDS = 2, // pin "A2" on RedBoard Nano, "LEDs" on the PCB
    NUM_LEDS = 122,

    PIN_BTN_HOUR = 3, // pin "A3" -- "HR" on PCB
    PIN_BTN_MIN = 4, // pin "4" -- "MIN" on PCB
    PIN_BTN_COLOR = 5, // pin "A5" -- "CLR" on PCB
    PIN_BTN_BRIGHTNESS = 6, // pin "6" -- "BRT" on PCB
    NUM_BTNS = 4,
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
    Settings &m_settings;
    DigitManager m_digitMgr;
    int m_lastRedrawTime{-1};

public:
    Clock(Adafruit_NeoPixel &leds, Settings &settings)
    : m_leds(leds)
    , m_settings(settings)
    , m_digitMgr(leds, settings)
    {
        RedrawIfNeeded(true);
    }

    void UpdateDigits()
    {
        if (m_settings.Get(SETTING_24_HOUR_MODE) == 1)
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

        // TODO: Move all animation out of this function
        for (int i = 0; i < 6; ++i)
        {
            if (m_settings.Get(SETTING_ANIMATION_TYPE) == ANIM_CYCLE_COLORS)
            {
                m_settings.Set(SETTING_COLOR, m_settings.Get(SETTING_COLOR) + 16);
            }
            m_digitMgr.SetDigitColor(i, ColorWheel(m_settings.Get(SETTING_COLOR)));
        
        }
        m_digitMgr.Draw();
    }


    void RedrawIfNeeded(bool force = false)
    {
        rtc_hal_update();

        if (rtc_hal_second() != m_lastRedrawTime || force)
        {
            m_lastRedrawTime = rtc_hal_second();
            UpdateDigits();

            if (m_settings.Get(SETTING_BLINKING_SEPARATORS))
            {
                BlinkDigitSeparators();
            }

            m_leds.show();
        }
    }

    void BlinkDigitSeparators()
    {
        if (m_settings.Get(SETTING_BLINKING_SEPARATORS) != 1)
        {
            return;
        }

        int blinkColor = ColorWheel(m_settings.Get(SETTING_COLOR));
        if (rtc_hal_second() % 2 != 0)
        {
            blinkColor = 0;
        }

        if (m_settings.Get(SETTING_DIGIT_TYPE) == 1)
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

Clock *g_clock = nullptr;
void setup()
{
    Settings settings;
    //settings.ResetToDefaults();

    Adafruit_NeoPixel leds(NUM_LEDS, PIN_LEDS, NEO_GRB + NEO_KHZ400);

    leds.begin(); // initialize NeoPixel library
    leds.setBrightness(settings.Get(SETTING_CUR_BRIGHTNESS)); 

    g_clock = new Clock(leds, settings);

    BluetoothInit();

    Button btnHour(PIN_BTN_HOUR);
    Button btnMinute(PIN_BTN_MIN);
    Button btnColor(PIN_BTN_COLOR);
    Button btnBrightness(PIN_BTN_BRIGHTNESS);

    btnHour.SetHandlerFunc([&](const Button::Event_e evt) {
        if (btnHour.TimePressed() > 1000)
        {
            rtc_hal_setTime(rtc_hal_hour() + 1, rtc_hal_minute(), rtc_hal_second());
            g_clock->RedrawIfNeeded(true);
        }
    });


    btnMinute.SetHandlerFunc([&](const Button::Event_e evt) {
        if (evt == Button::PRESS)
        {
            rtc_hal_setTime(rtc_hal_hour(), rtc_hal_minute() + 1, rtc_hal_second());
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
    STATE_WAIT,
    STATE_SET_TIME,
};
void handleSetTimeState(uint8_t val);

enum ClockSetState_e
{
    CS_HOUR,
    CS_MINUTE,
    CS_SECOND,
};

AlertState_e g_alertState = STATE_WAIT;
extern "C" void alert(uint8_t val)
{
    switch (g_alertState)
    {
    case STATE_WAIT:
        if (val == CMD_SET_TIME)
        {
            g_alertState = STATE_SET_TIME;
        }
        break;

    case STATE_SET_TIME:
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
        g_alertState = STATE_WAIT;
        g_clock->RedrawIfNeeded(true);
        break;

    default:
        break;
    }
}