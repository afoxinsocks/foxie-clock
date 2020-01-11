#pragma once
#include "Adafruit_NeoPixel.h"

class DigitBase
{
protected:
    enum
    {
        LEDS_PER_DIGIT = 20,
        OFF_COLOR = 0x000000,
    };

    Adafruit_NeoPixel &leds;
    int first;
    int color;

public:
    DigitBase(Adafruit_NeoPixel &strip, const int firstLED, const int onColor)
    : leds(strip), first(firstLED), color(onColor)
    {
    }

    virtual void Draw(const int num) = 0;

    void AllOff()
    {
        for (int i = 0; i < LEDS_PER_DIGIT; ++i)
        {
            leds.setPixelColor(first + i, OFF_COLOR);
        }
    }

    void SetColor(const int newColor)
    {
        color = newColor;
    }
};

// lights two LEDs at a time under each numeral
class EdgeLitDigit : public DigitBase
{
private:
    using DigitBase::DigitBase;

public:
    virtual void Draw(const int num)
    {
        AllOff();
        if (num >= 0 && num <= 9)
        {
            const int row = 10 - num;
            leds.setPixelColor(first + (row * 2) - 2, color);
            leds.setPixelColor(first + (row * 2) - 1, color);
        }
    }
};

// draws an entire number on the display using the available LEDs
class DisplayDigit : public DigitBase
{
private:
    using DigitBase::DigitBase;

public:
    virtual void Draw(const int num)
    {
        if (num >= 0 && num <= 9)
        {
            const uint8_t *data = NUMBERS + (num * LEDS_PER_DIGIT);
            for (int i = 0; i < LEDS_PER_DIGIT; ++i)
            {
                const int col = (data[i] == 0 ? OFF_COLOR : color);
                leds.setPixelColor(first + i, col);
            }
        }
        else
        {
            AllOff();
        }
    }

private:
        const uint8_t NUMBERS[10 * LEDS_PER_DIGIT] = {
        // 0
            1,      0,
        1,      1,
            0,      0,
        1,      1,
            0,      0,
        1,      1,
            0,      0,
        1,      1,
            1,      0,
        0,      0,

        // 1
            1,      0,
        1,      0,
            1,      0,
        0,      0,
            1,      0,
        0,      0,
            1,      0,
        0,      0,
            1,      0,
        0,      0,

        // 2
            1,      0,
        1,      1,
            0,      0,
        0,      1,
            1,      0,
        1,      0,
            0,      0,
        1,      1,
            0,      0,
        0,      0,

        // 3
            1,      0,
        1,      1,
            0,      0,
        0,      1,
            1,      0,
        0,      1,
            0,      0,
        1,      1,
            1,      0,
        0,      0,

        // 4
            0,      0,
        1,      1,
            0,      0,
        1,      1,
            1,      0,
        0,      1,
            0,      0,
        0,      1,
            0,      0,
        0,      1,

        // 5
            0,      0,
        1,      1,
            0,      0,
        1,      0,
            1,      0,
        0,      1,
            0,      0,
        1,      1,
            1,      0,
        0,      0,

        // 6
            1,      0,
        1,      0,
            0,      0,
        1,      0,
            1,      0,
        1,      1,
            0,      0,
        1,      1,
            1,      0,
        0,      0,

        // 7
            1,      0,
        1,      1,
            0,      0,
        0,      1,
            0,      0,
        0,      1,
            0,      0,
        0,      1,
            0,      0,
        0,      1,

        // 8
            1,      0,
        1,      1,
            0,      0,
        1,      1,
            1,      0,
        1,      1,
            0,      0,
        1,      1,
            1,      0,
        0,      0,

        // 9
            1,      0,
        1,      1,
            0,      0,
        1,      1,
            1,      0,
        0,      1,
            0,      0,
        0,      1,
            1,      0,
        0,      0,          
    };
};
