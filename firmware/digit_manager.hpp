#pragma once
#include "digit.hpp"
#include "settings.hpp"
#include <memory>
#include <vector>

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

    enum
    {
        FADE_TIME_MS = 600,
    };

    Adafruit_NeoPixel &m_leds;
    // shared pointers used so that destructing automatically deletes the digit
    std::vector<std::shared_ptr<Digit>> m_digits;

  public:
    // these store the value of each physical number display on the PCB
    // and can be updated directly for ease of use
    std::vector<uint8_t> numbers{0, 0, 0, 0, 0, 0}, prevNumbers;
    bool scalingNumbers[6] = {false};

    DigitManager(Adafruit_NeoPixel &leds) : m_leds(leds)
    {
        CreateDigitDisplay();
        prevNumbers = numbers;
    }

    void CreateDigitDisplay()
    {
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
            if (prevNumbers[i] != numbers[i])
            {
                scalingNumbers[i] = true;
            }

            if (IsFadeWindow() && scalingNumbers[i] && Settings::Get(SETTING_TRANSITION_TYPE) == 1)
            {
                const float halfFadeTime = FADE_TIME_MS / 2;

                // first half of fade, just before the second has flipped over
                float positionThroughFade = ((rtc_hal_millis() - (1000.0f - halfFadeTime)) / halfFadeTime) / 2;

                if (rtc_hal_millis() < halfFadeTime)
                {
                    // halfway through, just after the second has flipped over
                    positionThroughFade = 0.5f + ((rtc_hal_millis() / halfFadeTime) / 2);
                }

                m_digits[i]->SetBrightness(1.0f - positionThroughFade);
                m_digits[i]->Draw(prevNumbers[i]);

                m_digits[i]->SetBrightness(1.0f);
                m_digits[i]->Draw(numbers[i], true);
            }
            else
            {
                m_digits[i]->SetBrightness(1.0f);
                m_digits[i]->Draw(numbers[i]);
                scalingNumbers[i] = false;
            }
        }

        if (!IsFadeWindow())
        {
            prevNumbers = numbers;
        }
    }

    void SetDigitColor(const size_t digitNum, const uint32_t color)
    {
        m_digits[digitNum]->SetColor(color);
    }

    int GetDigitColor(const size_t digitNum)
    {
        return m_digits[digitNum]->GetColor();
    }

  private:
    bool IsFadeWindow()
    {
        return rtc_hal_millis() < (FADE_TIME_MS / 2) || rtc_hal_millis() > (1000 - (FADE_TIME_MS / 2));
    }

    bool IsLastSecondOfMinute()
    {
        return rtc_hal_second() == 59;
    }

    bool IsLastSecondOfHour()
    {
        return rtc_hal_minute() == 59 && rtc_hal_second() == 59;
    }

    std::shared_ptr<Digit> CreateDigit(const LEDNumbers_e firstLED)
    {
        if (Settings::Get(SETTING_DIGIT_TYPE) == 1)
        {
            return std::make_shared<EdgeLitDigit>(m_leds, firstLED, Settings::Get(SETTING_COLOR));
        }
        else
        {
            return std::make_shared<DisplayDigit>(m_leds, firstLED, Settings::Get(SETTING_COLOR));
        }
    }
};
