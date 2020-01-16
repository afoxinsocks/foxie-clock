#include <vector>

#include "Adafruit_NeoPixel.h"
#include "rtc_hal.hpp"
#include "digit.hpp"
#include "button.hpp"
#include "ble_funcs.hpp"
#include "settings.hpp"

#define DIGIT_TYPE 1 // 1 is edge lit (with acrylics), 2 is pixel display
#define MIN_BRIGHTNESS 4
#define MAX_BRIGHTNESS 255 // 255 max
#define BRIGHTNESS_STEP (MAX_BRIGHTNESS / 8)

#define BLINK 1
#define CYCLE_COLORS 1
#define INITIAL_WHEEL_COLOR 192

void UpdateClock(bool force = false);
void BlinkDigitSeparators();

enum
{
    PIN_LEDS = 2, // pin "A2" on RedBoard Nano, "LEDs" on the PCB
    NUM_LEDS = 122,

    // each number below refers to the top left LED of each column of
    // 20 offset LEDs, starting from the left of the PCB (digit 1)
    // to the right side (digit 6)
    //
    // Note: these numbers are 1 greater than the actual D### LED number
    // markings on the actual PCB, i.e. "D1" on the PCB = LED 0
    DIGIT_1_LED = 0,
    DIGIT_2_LED = 20,
    DIGIT_3_LED = 40,
    DIGIT_4_LED = 60,
    DIGIT_5_LED = 80,
    DIGIT_6_LED = 100,

#if DIGIT_TYPE == 1
    // 
    BLINK_1_LED = 120,
    BLINK_2_LED = 121,
#elif DIGIT_TYPE == 2
    BLINK_1_LED = 25,
    BLINK_2_LED = 65,
#endif

    PIN_BTN_HOUR = 3, // pin "A3" -- "HR" on PCB
    PIN_BTN_MIN = 4, // pin "4" -- "MIN" on PCB
    PIN_BTN_COLOR = 5, // pin "A5" -- "CLR" on PCB
    PIN_BTN_BRIGHTNESS = 6, // pin "6" -- "BRT" on PCB
    NUM_BTNS = 4,

    NUM_DIGITS = 6,
};

size_t g_curBrightness = MIN_BRIGHTNESS;
uint8_t g_wheelColor = INITIAL_WHEEL_COLOR;

struct NumberSet
{
    // stores the value of each physical number, from left to right
    std::vector<uint8_t> number{0,0,0,0,0,0};
    std::vector<uint8_t> prevColumnValue;

    Settings &settings;
    std::vector<Digit*> digits;

    NumberSet(Adafruit_NeoPixel &leds, Settings &s)
    : settings(s)
    {
        digits.push_back(new EdgeLitDigit(leds, DIGIT_1_LED, g_wheelColor));
        digits.push_back(new EdgeLitDigit(leds, DIGIT_2_LED, g_wheelColor));
        digits.push_back(new EdgeLitDigit(leds, DIGIT_3_LED, g_wheelColor));
        digits.push_back(new EdgeLitDigit(leds, DIGIT_4_LED, g_wheelColor));
        digits.push_back(new EdgeLitDigit(leds, DIGIT_5_LED, g_wheelColor));
        digits.push_back(new EdgeLitDigit(leds, DIGIT_6_LED, g_wheelColor));
    }

    void Draw(bool force = false)
    {
        for (size_t i = 0; i < NUM_DIGITS; ++i)
        {
            // TODO: Move this prevColumn logic to inside the Digit class

            // don't update digits that haven't changed
            if (number[i] != prevColumnValue[i] || force)
            {
                prevColumnValue[i] = number[i];
    
                if (settings.Get(SETTING_ANIMATION_TYPE) == ANIM_CYCLE_COLORS)
                {
                    // TODO: This global suuuucks. Fix.
                    g_wheelColor += 4;
                }

                digits[i]->SetColor(ColorWheel(g_wheelColor));
                digits[i]->Draw(number[i]);
            }
        }
    }
};

struct ClockSet : public NumberSet
{
    using NumberSet::NumberSet; // allows use of the NumberSet constructor above

    bool is24{false};
    void Update()
    {
        prevColumnValue = number;
        
        if (!is24)
        {
            number[0] = rtc_hal_hourFormat12() / 10;
            number[1] = rtc_hal_hourFormat12() % 10;
            if (number[0] == 0)
            {
                number[0] = 0xFF; // disable display
            }
        }
        else
        {
            number[0] = rtc_hal_hour() / 10;
            number[1] = rtc_hal_hour() % 10;
        }
        number[2] = rtc_hal_minute() / 10;
        number[3] = rtc_hal_minute() % 10;
        number[4] = rtc_hal_second() / 10;
        number[5] = rtc_hal_second() % 10;
    }
};


Adafruit_NeoPixel *g_leds = nullptr;
ClockSet *g_clock = nullptr;
Button g_btns[NUM_BTNS] = {PIN_BTN_HOUR, PIN_BTN_MIN, PIN_BTN_COLOR, PIN_BTN_BRIGHTNESS};

void setup()
{
    Settings settings;
    //settings.ResetToDefaults();

    g_leds = new Adafruit_NeoPixel(NUM_LEDS, PIN_LEDS, NEO_GRB + NEO_KHZ400);

    g_leds->begin(); // initialize NeoPixel library
    g_leds->setBrightness(g_curBrightness); 

    g_clock = new ClockSet(*g_leds, settings);

    rtc_hal_init();    
    UpdateClock(true);

    BLEInit();
}

void loop()
{
    BLEScheduledProcessing();

    rtc_hal_update();
    static int lastSecs = -1;
    if (lastSecs != rtc_hal_second())
    {
        lastSecs = rtc_hal_second();
        UpdateClock();
    }
    for (int i = 0; i < NUM_BTNS; ++i)
    {
        g_btns[i].Check();
    }

    if (g_btns[0].WasPressed())
    {
        rtc_hal_setTime(rtc_hal_hour() + 1, rtc_hal_minute(), rtc_hal_second());
        UpdateClock(true);
    }

    else if (g_btns[1].WasPressed())
    {
        rtc_hal_setTime(rtc_hal_hour(), rtc_hal_minute() + 1, rtc_hal_second());
        UpdateClock(true);
    }

    else if (g_btns[2].WasPressed())
    {
        g_wheelColor += 8;
        UpdateClock(true);
    }

    else if (g_btns[3].WasPressed())
    {
        g_curBrightness += BRIGHTNESS_STEP;
        if (g_curBrightness > MAX_BRIGHTNESS)
        {
            g_curBrightness = MIN_BRIGHTNESS;
        }
        g_leds->setBrightness(g_curBrightness);
        UpdateLEDs();
    }
}

void UpdateClock(bool force)
{
    g_clock->Update();
    g_clock->Draw(force);
#if BLINK == 1
    BlinkDigitSeparators();
#endif
    UpdateLEDs();
}

void UpdateLEDs()
{
    g_leds->show();
}

void BlinkDigitSeparators()
{
    int blinkColor = ColorWheel(g_wheelColor);
    if (rtc_hal_second() % 2 != 0)
    {
        blinkColor = 0;
    }
    g_leds->setPixelColor(BLINK_1_LED, blinkColor);
    g_leds->setPixelColor(BLINK_2_LED, blinkColor);

#if DIGIT_TYPE == 2
    g_leds->setPixelColor(BLINK_1_LED + 8, blinkColor);
    g_leds->setPixelColor(BLINK_2_LED + 8, blinkColor);
#endif
}
