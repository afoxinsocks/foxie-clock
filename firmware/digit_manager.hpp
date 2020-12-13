#pragma once
#include <memory>
#include <vector>

#include "animator.hpp"
#include "digit.hpp"
#include "settings.hpp"

using Numbers_t = std::vector<uint8_t>; // must always be NUM_DIGITS size

class DigitManager
{
  private:
    enum LEDNumbers_e
    {
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
    };

    Adafruit_NeoPixel &m_leds;
    Settings &m_settings;

    // these are the value of each physical number that can be displayed
    // For a Foxie Clock, a "Digit" can only be 0-9.
    Numbers_t m_numbers;

    // shared pointers used so that destructing automatically deletes the digit
    DigitPtrs_t m_digits;

    std::shared_ptr<Animator> m_animator;

  public:
    DigitManager(Adafruit_NeoPixel &leds, Settings &settings) : m_leds(leds), m_settings(settings)
    {
        m_numbers.resize(NUM_DIGITS, 0);

        CreateDigits();
    }

    void CreateDigits()
    {
        m_digits.clear();

        m_digits.push_back(CreateDigit(DIGIT_1_LED));
        m_digits.push_back(CreateDigit(DIGIT_2_LED));
        m_digits.push_back(CreateDigit(DIGIT_3_LED));
        m_digits.push_back(CreateDigit(DIGIT_4_LED));
        m_digits.push_back(CreateDigit(DIGIT_5_LED));
        m_digits.push_back(CreateDigit(DIGIT_6_LED));

        UseAnimation((AnimationType_e)m_settings.Get(SETTING_ANIMATION_TYPE));
    }

    void UseAnimation(const AnimationType_e type)
    {
        m_animator = AnimatorFactory(m_digits, type, m_settings.Get(SETTING_COLOR));
    }

    void Display(const Numbers_t numbers)
    {
        m_numbers = numbers;

        m_animator->Go();

        for (size_t i = 0; i < NUM_DIGITS; ++i)
        {
            m_digits[i]->Display(m_numbers[i]);
        }
    }

    void ColorButtonPressed(const uint8_t wheelColor)
    {
        m_animator->ColorButtonPressed(wheelColor);
    }

  private:
    DigitPtr_t CreateDigit(const LEDNumbers_e firstLED)
    {
        if (m_settings.Get(SETTING_DIGIT_TYPE) == DT_EDGE_LIT)
        {
            return std::make_shared<EdgeLitDigit>(m_leds, firstLED, ColorWheel(m_settings.Get(SETTING_COLOR)));
        }
        else // DT_PIXELS
        {
            return std::make_shared<DisplayDigit>(m_leds, firstLED, ColorWheel(m_settings.Get(SETTING_COLOR)));
        }
    }
};
