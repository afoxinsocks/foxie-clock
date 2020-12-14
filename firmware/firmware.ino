#define FOXIE_ARTEMIS

enum HardwareConfig_e
{
    PIN_FOR_LEDS = 2, // pin "A2" -- "LEDs" on PCB
    NUM_LEDS = 122,
    NUM_DIGITS = 6,
    NUM_BTNS = 4,

    DELAY_FOR_COMBINATION_BUTTONS = 750,
};

enum Pins_e 
{
    PIN_BTN_H = 3, // pin "A3" -- "HR" on PCB
    PIN_BTN_M = 4, // pin "4" -- "MIN" on PCB
    PIN_BTN_C = 5, // pin "A5" -- "CLR" on PCB
    PIN_BTN_B = 6, // pin "6" -- "BRT" on PCB
};

#include "clock.hpp"

void setup()
{
    // The idea in this file is to keep it mostly Arduino-specific,
    // while all "real" clock functionality is contained within the
    // Clock class in a very "clean" way. That's the idea, anyway...

    Clock clock;
    while (true)
    {
        clock.Loop();
    }
}

void loop()
{
    // never used, see Clock::Loop() in clock.hpp instead
}