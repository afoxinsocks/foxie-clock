#pragma once
#include <memory>
#include <vector>

#include "animator.hpp"
#include "digit.hpp"
#include "settings.hpp"

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

    DigitPtrs_t m_digits;
    AnimatorPtr_t m_animator;
    DigitValues m_values;

  public:
    DigitManager(Adafruit_NeoPixel &leds, Settings &settings) : m_leds(leds), m_settings(settings)
    {
        CreateDigits();
    }

    void CreateDigits()
    {
        m_values.digits.clear();

        m_values.digits.push_back(CreateDigit(DIGIT_1_LED));
        m_values.digits.push_back(CreateDigit(DIGIT_2_LED));
        m_values.digits.push_back(CreateDigit(DIGIT_3_LED));
        m_values.digits.push_back(CreateDigit(DIGIT_4_LED));
        m_values.digits.push_back(CreateDigit(DIGIT_5_LED));
        m_values.digits.push_back(CreateDigit(DIGIT_6_LED));

        UseAnimation((AnimationType_e)m_settings.Get(SETTING_ANIMATION_TYPE));
    }

    void UseAnimation(const AnimationType_e type)
    {
        m_animator = AnimatorFactory(m_settings, m_values, type, m_settings.Get(SETTING_COLOR));
    }

    void Display(const Numbers_t numbers)
    {
        m_values.Set(numbers);
        m_animator->Go(numbers);
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
            return std::make_shared<PXLDigit>(m_leds, firstLED, ColorWheel(m_settings.Get(SETTING_COLOR)));
        }
    }
};
