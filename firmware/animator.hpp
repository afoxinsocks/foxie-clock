#pragma once
#include "digit_manager.hpp"
#include "settings.hpp"

class Animator
{
protected:
    DigitManager &m_digitMgr;

public:
    Animator(DigitManager &digitMgr)
    : m_digitMgr(digitMgr)
    {
    }

    virtual void Go() 
    {
        for (int i = 0; i < 6; ++i)
        {
            m_digitMgr.SetDigitColor(i, ColorWheel(Settings::Get(SETTING_COLOR)));
        }
    };
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

static inline Animator *AnimatorFactory(DigitManager &digitMgr, const AnimationType_e type)
{
    switch (type)
    {
        case ANIM_CYCLE_COLORS: return new AnimatorCycleAll(digitMgr);
    }
    
    return new Animator(digitMgr);
}
