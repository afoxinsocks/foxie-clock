#pragma once
#include "digit_manager.hpp"
#include "settings.hpp"

enum AnimationType_e
{
    ANIM_NONE = 0,
    ANIM_CYCLE_COLORS,
    ANIM_GLOW,

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

    virtual bool IsFast()
    {
        return false;
    }
};

class AnimatorCycleAll : public Animator
{
    using Animator::Animator;

  private:
    uint8_t color{0};

  public:
    virtual void Go() override
    {
        for (int i = 0; i < 6; ++i)
        {
            color += 16;
            m_digitMgr.SetDigitColor(i, ColorWheel(color));
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

static inline Animator *AnimatorFactory(DigitManager &digitMgr, const AnimationType_e type)
{
    switch (type)
    {
    case ANIM_CYCLE_COLORS:
        return new AnimatorCycleAll(digitMgr);
    case ANIM_GLOW:
        return new AnimatorGlow(digitMgr);
    case ANIM_SET_TIME:
        return new AnimatorSetTime(digitMgr);
    }

    return new Animator(digitMgr);
}
