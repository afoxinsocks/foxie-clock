#include <Wire.h>
#include <vector>

#define FOXIE_ARTEMIS

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

#include "Adafruit_NeoPixel.h"
#include "button.hpp"
#include "clock.hpp"
#include "digit_manager.hpp"
#include "rtc_hal.hpp"
#include "settings.hpp"

void setup()
{
    Serial.begin(115200);
    rtc_hal_init();

    Adafruit_NeoPixel leds(NUM_LEDS, PIN_LEDS, NEO_GRB + NEO_KHZ400);
    leds.begin(); // initialize NeoPixel library

    // The idea in this file is to keep it mostly Arduino-specific 
    Settings settings;
    Clock clock(leds, settings);

    while (true)
    {
        clock.Process();
    }
}

void loop()
{
    // will never be used; we use the while(true) loop above instead
}