#pragma once
#include <memory>

#include "digit.hpp"
#include "rtc_hal.hpp"

enum AnimationType_e
{
    ANIM_NONE = 0,
    ANIM_GLOW,
    ANIM_CYCLE_COLORS,
    ANIM_CYCLE_FLOW_LEFT,
    ANIM_RAINBOW,

    // Add new types above here
    ANIM_TOTAL,
    ANIM_SET_TIME,
};

class Animator
{
  protected:
    DigitPtrs_t m_digits;
    int m_lastSecond;
    uint8_t m_wheelColor;

  public:
    Animator(DigitPtrs_t digits, const uint8_t wheelColor) : m_digits(digits)
    {
        m_wheelColor = wheelColor;
        m_lastSecond = rtc_hal_second();
        Go();
    }
    virtual ~Animator()
    {
    }

    virtual void Go()
    {
        // does nothing except update the digit colors to the current setting
        for (int i = 0; i < 6; ++i)
        {
            m_digits[i]->SetColor(ColorWheel(m_wheelColor));
        }
    };

    virtual void SetWheelColor(const uint8_t wheelColor)
    {
        m_wheelColor = wheelColor;
    }

    // an animator can choose to do something special when the color button
    // is pressed
    virtual void ColorButtonPressed(uint8_t wheelColor)
    {
        SetWheelColor(wheelColor);
    }
};

class AnimatorCycleAll : public Animator
{
    using Animator::Animator;

  public:
    virtual void Go() override
    {
        if (m_lastSecond != rtc_hal_second())
        {
            for (int i = 0; i < 6; ++i)
            {

                m_wheelColor += 16;
                m_digits[i]->SetColor(ColorWheel(m_wheelColor));
            }
            m_lastSecond = rtc_hal_second();
        }
    }
};

class AnimatorGlow : public Animator
{
    using Animator::Animator;

    float m_brightness{1.0f};
    float m_incrementer{0.1f};
    int m_millis{0};

  public:
    virtual void Go() override
    {
        int scaledColor = ScaleBrightness(ColorWheel(m_wheelColor), m_brightness);
        for (int i = 0; i < 6; ++i)
        {
            m_digits[i]->SetColor(scaledColor);
        }

        if ((int)millis() - m_millis >= 25)
        {
            m_millis = millis();

            m_brightness += m_incrementer;
            if (m_brightness > 1.0f)
            {
                m_incrementer = -0.025f;
                m_brightness = 1.0f;
            }
            else if (m_brightness < 0.2f)
            {
                m_incrementer = 0.025f;
                m_brightness = 0.2f;
            }
        }
    }
};

// Changes one digit color at a time, flowing to the left.
class AnimatorCycleFlowLeft : public Animator
{
    using Animator::Animator;

  public:
    virtual void Go() override
    {
        if (m_lastSecond != rtc_hal_second())
        {
            CycleDigitColors(false);
            m_lastSecond = rtc_hal_second();
        }
    }

    virtual void ColorButtonPressed(uint8_t wheelColor) override
    {
        CycleDigitColors(true);
    }

  private:
    void CycleDigitColors(bool forceRotate = false)
    {
        SetWheelColor(m_wheelColor + 6);
        m_digits[5]->SetColor(ColorWheel(m_wheelColor));
        if (m_digits[5]->GetNumDisplayed() == 9 || forceRotate)
        {
            for (int i = 0; i < 5; ++i)
            {
                // get the color of the digit to the right
                int newColor = m_digits[i + 1]->GetColor();
                m_digits[i]->SetColor(newColor);
            }
        }
    }
};

class AnimatorRainbow : public Animator
{
    using Animator::Animator;

  private:
    bool m_paused{false};
    float m_colors[6] = {0, 12, 24, 36, 48, 60};

  public:
    virtual void Go() override
    {
        if (m_paused)
        {
            return;
        }

        for (int i = 5; i >= 0; --i)
        {
            m_colors[i] += 0.1f;

            if (m_colors[i] > 255.0f)
            {
                m_colors[i] -= 255.0f;
            }

            m_digits[i]->SetColor(ColorWheel(m_colors[i]));
        }
    }

    virtual void ColorButtonPressed(uint8_t wheelColor) override
    {
        m_paused = !m_paused;
    }
};

class AnimatorSetTime : public Animator
{
    using Animator::Animator;

  public:
    virtual void Go() override
    {
        for (int i = 0; i < 6; ++i)
        {
            m_digits[i]->SetColor(ColorWheel(m_wheelColor + 128));
        }
    };
};

static inline std::shared_ptr<Animator> AnimatorFactory(DigitPtrs_t digits, const AnimationType_e type,
                                                        uint8_t wheelColor)
{
    switch (type)
    {
    case ANIM_GLOW:
        return std::make_shared<AnimatorGlow>(digits, wheelColor);
    case ANIM_CYCLE_COLORS:
        return std::make_shared<AnimatorCycleAll>(digits, wheelColor);
    case ANIM_CYCLE_FLOW_LEFT:
        return std::make_shared<AnimatorCycleFlowLeft>(digits, wheelColor);
    case ANIM_RAINBOW:
        return std::make_shared<AnimatorRainbow>(digits, wheelColor);
    case ANIM_SET_TIME:
        return std::make_shared<AnimatorSetTime>(digits, wheelColor);
    }

    return std::make_shared<Animator>(digits, wheelColor);
}
