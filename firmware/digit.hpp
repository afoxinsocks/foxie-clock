#pragma once
#include "Adafruit_NeoPixel.h"
#include "rtc_hal.hpp"

// returns a color transitioning from r -> g -> b and back to r
// lightly modified from Adafruit NeoPixel strand test

enum DigitTypes_e
{
    DT_EDGE_LIT = 1,
    DT_PIXELS = 2,
};

static inline uint32_t ColorWheel(uint8_t pos)
{
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

static inline int ScaleBrightness(const int color, const float brightness)
{
    const float r = ((color & 0xFF0000) >> 16) * brightness;
    const float g = ((color & 0x00FF00) >> 8) * brightness;
    const float b = (color & 0x0000FF) * brightness;
    return Adafruit_NeoPixel::Color(r, g, b);
}

class Digit
{
  public:
    enum
    {
        LEDS_PER_DIGIT = 20,
        OFF_COLOR = 0x000000,
        INVALID = 0xFF,
    };

  protected:
    Adafruit_NeoPixel &m_leds;
    int m_first;
    int m_color;
    float m_brightness{1.0f};

  public:
    Digit(Adafruit_NeoPixel &strip, const int firstLED, const int onColor)
        : m_leds(strip), m_first(firstLED), m_color(onColor)
    {
    }

    virtual void Display(const int num, const bool skipErase = false) = 0;

    void AllOff()
    {
        for (int i = 0; i < LEDS_PER_DIGIT; ++i)
        {
            SetPixel(m_first + i, OFF_COLOR);
        }
    }

    void SetColor(const int newColor)
    {
        m_color = newColor;
    }

    void SetBrightness(const float brightness)
    {
        if (brightness > 1.0f)
        {
            return;
        }

        m_brightness = brightness;
    }

    int GetColor()
    {
        return m_color;
    }

    void SetPixel(const int pixelNum, const int color)
    {
        m_leds.setPixelColor(pixelNum, ScaleBrightness(color, m_brightness));
    }
};

// lights two LEDs at a time under each numeral
class EdgeLitDigit : public Digit
{
  private:
    using Digit::Digit;

  public:
    virtual void Display(const int num, const bool skipErase)
    {
        if (!skipErase)
        {
            AllOff();
        }

        if (num >= 0 && num <= 9)
        {
            const int row = 10 - num;
            SetPixel(m_first + (row * 2) - 2, m_color);
            SetPixel(m_first + (row * 2) - 1, m_color);
        }
    }
};

// draws an entire number on the display using the available LEDs
class DisplayDigit : public Digit
{
  private:
    using Digit::Digit;

  public:
    virtual void Display(const int num, const bool skipErase)
    {
        if (num >= 0 && num <= 9)
        {
            const uint8_t *data = NUMBERS + (num * LEDS_PER_DIGIT);
            for (int i = 0; i < LEDS_PER_DIGIT; ++i)
            {
                const int col = (data[i] == 0 ? OFF_COLOR : m_color);
                if (col != OFF_COLOR || !skipErase)
                {
                    SetPixel(m_first + i, col);
                }
            }
        }
        else
        {
            AllOff();
        }
    }

  private:
    const uint8_t NUMBERS[10 * LEDS_PER_DIGIT] = {
        // clang-format off

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
        0,      0,

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
        0,      0,

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

        // clang-format on
    };
};
