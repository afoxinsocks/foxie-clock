#include <vector>

#include "Adafruit_NeoPixel.h"
#include "rtc_hal.hpp"
#include "digit.hpp"
#include "button.hpp"
#include "ble_funcs.hpp"

#define DIGIT_TYPE 1 // 1 is edge lit (with acrylics), 2 is pixel display
#define MIN_BRIGHTNESS 4
#define MAX_BRIGHTNESS 192 // 255 max
#define BRIGHTNESS_STEP (MAX_BRIGHTNESS / 8)

#define BLINK 1
#define CYCLE_COLORS 1
#define INITIAL_WHEEL_COLOR 192

void updateClock(bool force = false);
void updateDigitSeparators();
uint32_t colorWheel(uint8_t pos);

enum
{
    PIN_LEDS = 2,
    NUM_LEDS = 122,

    DIGIT_1_LED = 0,
    DIGIT_2_LED = 20,
    DIGIT_3_LED = 40,
    DIGIT_4_LED = 60,
    DIGIT_5_LED = 80,
    DIGIT_6_LED = 100,

#if DIGIT_TYPE == 1
    BLINK_1_LED = 120,
    BLINK_2_LED = 121,
#elif DIGIT_TYPE == 2
    BLINK_1_LED = 25,
    BLINK_2_LED = 65,
#endif

    PIN_BTN_HOUR = 3,
    PIN_BTN_MIN = 4,
    PIN_BTN_COLOR = 5,
    PIN_BTN_BRIGHTNESS = 6,
    NUM_BTNS = 4,
};

Adafruit_NeoPixel leds(NUM_LEDS, PIN_LEDS, NEO_GRB + NEO_KHZ400);
size_t g_curBrightness = MIN_BRIGHTNESS;
uint8_t g_wheelColor = INITIAL_WHEEL_COLOR;

#if DIGIT_TYPE == 1
EdgeLitDigit g_digits[6] = {
#elif DIGIT_TYPE == 2
DisplayDigit g_digits[6] = {
#endif
    {leds, DIGIT_1_LED, g_wheelColor},
    {leds, DIGIT_2_LED, g_wheelColor},
    {leds, DIGIT_3_LED, g_wheelColor},
    {leds, DIGIT_4_LED, g_wheelColor},
    {leds, DIGIT_5_LED, g_wheelColor},
    {leds, DIGIT_6_LED, g_wheelColor}
};

struct DigitSet
{
    std::vector<uint8_t> d{0,0,0,0,0,0};
    std::vector<uint8_t> prev;

    void Draw(bool force = false)
    {
        for (size_t i = 0; i < 6; ++i)
        {
            if (d[i] != prev[i] || force)
            {
#if CYCLE_COLORS == 1
                // bump the wheel color every time we change a digit
                g_wheelColor += 4;
#endif
                g_digits[i].SetColor(colorWheel(g_wheelColor));
                g_digits[i].Draw(d[i]);
                prev[i] = d[i];
            }
        }
    }
};

struct Clock : public DigitSet
{
    bool is24{false};
    void Update()
    {
        prev = d;
        
        if (!is24)
        {
            d[0] = rtc_hal_hourFormat12() / 10;
            d[1] = rtc_hal_hourFormat12() % 10;
            if (d[0] == 0)
            {
                d[0] = 0xFF; // disable display
            }
        }
        else
        {
            d[0] = rtc_hal_hour() / 10;
            d[1] = rtc_hal_hour() % 10;
        }
        d[2] = rtc_hal_minute() / 10;
        d[3] = rtc_hal_minute() % 10;
        d[4] = rtc_hal_second() / 10;
        d[5] = rtc_hal_second() % 10;

    }

    void SetFromList(std::vector<uint8_t> list)
    {
        prev = d;
        d = list;
        Draw();
    }
};


Clock g_clock;
Button g_btns[NUM_BTNS] = {PIN_BTN_HOUR, PIN_BTN_MIN, PIN_BTN_COLOR, PIN_BTN_BRIGHTNESS};

void setup()
{
    Serial.begin(115200);
    leds.begin(); // initialize NeoPixel library
    leds.setBrightness(g_curBrightness); 

    rtc_hal_init();    
    updateClock(true);

    for (int i = 0; i < NUM_BTNS; ++i)
    {
        g_btns[i].Init();
    }

    ble_init();
}

void loop()
{
    ble_update_scheduler_timers();

    // if (Serial.available()) {
    //     // TODO: Add handling for commands coming in over the serial connection
    // }

    rtc_hal_update();
    static int lastSecs = -1;
    if (lastSecs != rtc_hal_second())
    {
        lastSecs = rtc_hal_second();
        updateClock();
    }
    for (int i = 0; i < NUM_BTNS; ++i)
    {
        g_btns[i].Check();
    }

    if (g_btns[0].WasPressed())
    {
        rtc_hal_setTime(rtc_hal_hour() + 1, rtc_hal_minute(), rtc_hal_second());
        updateClock(true);
    }

    else if (g_btns[1].WasPressed())
    {
        rtc_hal_setTime(rtc_hal_hour(), rtc_hal_minute() + 1, rtc_hal_second());
        updateClock(true);
    }

    else if (g_btns[2].WasPressed())
    {
        g_wheelColor += 8;
        updateClock(true);
    }

    else if (g_btns[3].WasPressed())
    {
        g_curBrightness += BRIGHTNESS_STEP;
        if (g_curBrightness > MAX_BRIGHTNESS)
        {
            g_curBrightness = MIN_BRIGHTNESS;
        }
        leds.setBrightness(g_curBrightness);
        updateLEDs();
    }
}

void updateClock(bool force)
{
    g_clock.Update();
    g_clock.Draw(force);
#if BLINK == 1
    updateDigitSeparators();
#endif
    updateLEDs();
}

void updateLEDs()
{
    leds.show();
}

void updateDigitSeparators()
{
    int blinkColor = colorWheel(g_wheelColor);
    if (rtc_hal_second() % 2 != 0)
    {
        blinkColor = 0;
    }
    leds.setPixelColor(BLINK_1_LED, blinkColor);
    leds.setPixelColor(BLINK_2_LED, blinkColor);

#if DIGIT_TYPE == 2
    leds.setPixelColor(BLINK_1_LED + 8, blinkColor);
    leds.setPixelColor(BLINK_2_LED + 8, blinkColor);
#endif
}

// returns a color transitioning from r -> g -> b and back to r
uint32_t colorWheel(uint8_t pos) {
    pos = 255 - pos;
    if (pos < 85) 
    {
       return Adafruit_NeoPixel::Color(255 - pos * 3, 0, pos * 3);
    }
    
    if (pos < 170) 
    {
        pos -= 85;
        return Adafruit_NeoPixel::Color(0, pos * 3, 255 - pos * 3);
    }
    
    pos -= 170;
    return Adafruit_NeoPixel::Color(pos * 3, 255 - pos * 3, 0);
}
