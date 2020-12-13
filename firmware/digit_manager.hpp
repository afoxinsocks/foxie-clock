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

    Adafruit_NeoPixel &m_leds;
    Settings &m_settings;

    // these store the value of each physical number display on the PCB
    std::vector<uint8_t> m_numbers{0, 0, 0, 0, 0, 0};
    std::vector<uint8_t> m_prevNumbers{0, 0, 0, 0, 0, 0};

    // shared pointers used so that destructing automatically deletes the digit
    std::vector<std::shared_ptr<Digit>> m_digits;

  public:
    DigitManager(Adafruit_NeoPixel &leds, Settings &settings) : m_leds(leds), m_settings(settings)
    {
        CreateDigitDisplay();
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

    void UpdateDigitsFromRTC()
    {
        rtc_hal_update();

        if (m_settings.Get(SETTING_24_HOUR_MODE) == 1)
        {
            m_numbers[0] = rtc_hal_hour() / 10;
            m_numbers[1] = rtc_hal_hour() % 10;
        }
        else
        {
            m_numbers[0] = rtc_hal_hourFormat12() / 10;
            m_numbers[1] = rtc_hal_hourFormat12() % 10;

            if (m_numbers[0] == 0)
            {
                // disable leading 0 for 12 hour mode
                m_numbers[0] = Digit::INVALID;
            }
        }

        m_numbers[2] = rtc_hal_minute() / 10;
        m_numbers[3] = rtc_hal_minute() % 10;

        m_numbers[4] = rtc_hal_second() / 10;
        m_numbers[5] = rtc_hal_second() % 10;

        Update();
    }

    void Update()
    {
        for (size_t i = 0; i < NUM_DIGITS; ++i)
        {
            // m_digits[i]->SetBrightness(1.0f);
            m_digits[i]->Display(m_numbers[i]);
        }

        m_prevNumbers = m_numbers;
    }

  private:
    // bool IsFadeWindow()
    // {
    //     return rtc_hal_millis() < (FADE_TIME_MS / 2) || rtc_hal_millis() > (1000 - (FADE_TIME_MS / 2));
    // }

    std::shared_ptr<Digit> CreateDigit(const LEDNumbers_e firstLED)
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
