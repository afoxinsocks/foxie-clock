#pragma once
#include "digit.hpp"
#include "settings.hpp"
#include <memory>
#include <vector>

using Numbers_t = std::vector<uint8_t>; // must always be NUM_DIGITS size
using DigitPtr_t = std::shared_ptr<Digit>;
using DigitPtrs_t = std::vector<DigitPtr_t>;

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
    }

    void Display(const Numbers_t numbers)
    {
        m_numbers = numbers;

        for (size_t i = 0; i < NUM_DIGITS; ++i)
        {
            m_digits[i]->Display(m_numbers[i]);
        }
    }

  private:
    DigitPtr_t CreateDigit(const LEDNumbers_e firstLED)
    {
        if (m_settings.Get(SETTING_DIGIT_TYPE) == DT_EDGE_LIT)
        {
            return std::make_shared<EdgeLitDigit>(m_leds, firstLED, m_settings.Get(SETTING_COLOR));
        }
        else // DT_PIXELS
        {
            return std::make_shared<DisplayDigit>(m_leds, firstLED, m_settings.Get(SETTING_COLOR));
        }
    }
};
