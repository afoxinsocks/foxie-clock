#pragma once
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

    	NUM_DIGITS = 6,        
    };

    Adafruit_NeoPixel &m_leds;
    std::vector<Digit*> m_digits;

public:
    // these store the value of each physical number display on the PCB
    // and can be updated directly for ease of use
    std::vector<uint8_t> numbers{0, 0, 0, 0, 0, 0};

    DigitManager(Adafruit_NeoPixel &leds)
    : m_leds(leds)
    {
        CreateDigitDisplay();
    }

    void CreateDigitDisplay()
    {
        for (auto &d : m_digits)
        {
            // TODO: Convert to smart pointers so clearing automatically deletes
            delete(d);
        }
        m_digits.clear();

        m_digits.push_back(CreateDigit(DIGIT_1_LED));
        m_digits.push_back(CreateDigit(DIGIT_2_LED));
        m_digits.push_back(CreateDigit(DIGIT_3_LED));
        m_digits.push_back(CreateDigit(DIGIT_4_LED));
        m_digits.push_back(CreateDigit(DIGIT_5_LED));
        m_digits.push_back(CreateDigit(DIGIT_6_LED));
    }

    void Draw()
    {
        for (size_t i = 0; i < NUM_DIGITS; ++i)
        {
            m_digits[i]->Draw(numbers[i]);
        }
    }

    void SetDigitColor(const size_t digitNum, const uint32_t color)
    {
		m_digits[digitNum]->SetColor(color);
    }

private:
    Digit* CreateDigit(const LEDNumbers_e firstLED)
    {
        if (Settings::Get(SETTING_DIGIT_TYPE) == 1)
        {
            return new EdgeLitDigit(m_leds, firstLED, Settings::Get(SETTING_COLOR));
        }
        else
        {
            return new DisplayDigit(m_leds, firstLED, Settings::Get(SETTING_COLOR));
        }
    }
};
