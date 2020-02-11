#pragma once
#include "digit_manager.hpp"
#include "settings.hpp"
#include <memory>

enum AnimationType_e
{
    ANIM_NONE = 0,
    ANIM_GLOW,
    ANIM_CYCLE_COLORS,
    ANIM_CYCLE_FLOW_LEFT,

    // Add new types above here
    ANIM_SET_TIME,
};

class Animator
{
  protected:
    DigitManager &m_digitMgr;

  public:
    Animator(DigitManager &digitMgr) : m_digitMgr(digitMgr)
    {
    }

    virtual void Go()
    {
        // does nothing except update the digit colors to the current setting
        for (int i = 0; i < 6; ++i)
        {
            m_digitMgr.SetDigitColor(i, ColorWheel(Settings::Get(SETTING_COLOR)));
        }
    };

    // Fast animations occur as often as possible (like AnimatorGlow below),
    // while non-fast animations occur once a second.
    virtual bool IsFast()
    {
        return false;
    }

    // an animator can choose to do something special when the color button
    // is pressed
    virtual void ColorButtonPressed()
    {
    }
};

class AnimatorCycleAll : public Animator
{
    using Animator::Animator;

  private:
    uint8_t m_color{0};

  public:
    virtual void Go() override
    {
        for (int i = 0; i < 6; ++i)
        {
            m_color += 16;
            m_digitMgr.SetDigitColor(i, ColorWheel(m_color));
        }
    }
};

class AnimatorGlow : public Animator
{
    using Animator::Animator;

  private:
    float m_brightness{1.0f};
    float m_incrementer{0.1f};
    int m_millis{0};

  public:
    virtual void Go() override
    {
        int scaledColor = ScaleBrightness(ColorWheel(Settings::Get(SETTING_COLOR)), m_brightness);
        for (int i = 0; i < 6; ++i)
        {
            m_digitMgr.SetDigitColor(i, scaledColor);
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

    virtual bool IsFast()
    {
        return true;
    }
};

// Changes one digit color at a time, flowing to the left.
class AnimatorCycleFlowLeft : public Animator
{
  private:
    std::vector<uint8_t> prevNumbers;

  public:
    AnimatorCycleFlowLeft(DigitManager &digitMgr) : Animator(digitMgr)
    {
        prevNumbers = digitMgr.numbers;
        Animator::Go();
    }

    virtual void Go() override
    {
        for (int i = 0; i < 6; ++i)
        {
            if (prevNumbers[i] != m_digitMgr.numbers[i])
            {
                CycleDigitColor(i);
            }
        }

        prevNumbers = m_digitMgr.numbers;
    }

    virtual void ColorButtonPressed() override
    {
        // invalidate previous numbers
        for (auto &prev : prevNumbers)
        {
            prev = Digit::INVALID;
        }
    }

  private:
    void CycleDigitColor(const int digitNum)
    {
        int newColor;
        if (digitNum == 5)
        {
            Settings::Set(SETTING_COLOR, Settings::Get(SETTING_COLOR) + 6);
            newColor = ColorWheel(Settings::Get(SETTING_COLOR));
        }
        else
        {
            // get the color of the digit to the right
            newColor = m_digitMgr.GetDigitColor(digitNum + 1);
        }
        m_digitMgr.SetDigitColor(digitNum, newColor);
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
            m_digitMgr.SetDigitColor(i, ColorWheel((uint8_t)(Settings::Get(SETTING_COLOR) + 128)));
        }
    }
};

static inline std::shared_ptr<Animator> AnimatorFactory(DigitManager &digitMgr, const AnimationType_e type)
{
    switch (type)
    {
    case ANIM_GLOW:
        return std::make_shared<AnimatorGlow>(digitMgr);
    case ANIM_CYCLE_COLORS:
        return std::make_shared<AnimatorCycleAll>(digitMgr);
    case ANIM_CYCLE_FLOW_LEFT:
        return std::make_shared<AnimatorCycleFlowLeft>(digitMgr);
    case ANIM_SET_TIME:
        return std::make_shared<AnimatorSetTime>(digitMgr);
    }

    return std::make_shared<Animator>(digitMgr);
}
