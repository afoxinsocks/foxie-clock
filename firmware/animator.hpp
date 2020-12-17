#pragma once
#include <memory>

#include "digit.hpp"
#include "elapsed_time.hpp"
#include "rtc_hal.hpp"
#include "settings.hpp"

enum AnimationType_e
{
    ANIM_NONE = 0,
    ANIM_ZIPPY,
    ANIM_GLOW,
    ANIM_CYCLE_COLORS,
    ANIM_CYCLE_FLOW_LEFT,
    ANIM_RAINBOW,

    ANIM_USER_ACCESSIBLE_TOTAL,
    ANIM_ALT_DISPLAY,
    ANIM_SET_TIME,
};

class Animator
{
  protected:
    int m_lastSecond{0};
    Settings &m_settings;
    DigitPtrs_t m_digits;
    uint8_t m_wheelColor;
    bool m_isOncePerSecond{false};
    Numbers_t m_lastNumbers;
    Numbers_t m_currentNumbers;
    ElapsedTime m_timeSinceSecondBegan;

  public:
    enum Configuration_e
    {
        // this is the amount of time that we want to spend transitioning
        // between old and new digits, in milliseconds
        TRANSITION_TIME = 400,
    };

    Animator(Settings &settings, DigitPtrs_t digits, const uint8_t wheelColor) : m_settings(settings), m_digits(digits)
    {
        m_wheelColor = wheelColor;
        m_lastSecond = rtc_hal_second();
        m_timeSinceSecondBegan.Set(rtc_hal_millis());

        for (int i = 0; i < NUM_DIGITS; ++i)
        {
            m_digits[i]->AllOff();
        }

        m_lastNumbers = {Digit::INVALID, Digit::INVALID, Digit::INVALID,
                         Digit::INVALID, Digit::INVALID, Digit::INVALID};
    }

    virtual ~Animator()
    {
    }

    void Go(const Numbers_t &numbers)
    {
        m_currentNumbers = numbers;

        // this is to catch the case when we've stopped receiving updates from the RTC while we're in Set Time mode. If
        // we pass some a sufficient value above 1000, we can be pretty sure rtc_hal_second() is malfunctioning. TODO: I
        // should move this into rtc_hal_millis()
        bool oneSecondHasPassed = (m_timeSinceSecondBegan.Ms() > 1057);

        if (rtc_hal_second() != m_lastSecond || oneSecondHasPassed)
        {
            m_timeSinceSecondBegan.Reset();
            m_lastSecond = rtc_hal_second();
            DoOncePerSecond();
        }

        DoColorChanges();
        DoBrightnessAndDisplay();

        if (!IsTransitioning() || oneSecondHasPassed)
        {
            m_lastNumbers = numbers;
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

  protected:
    // subclasses can do different things if they override these functions
    virtual void DoColorChanges()
    {
        const int color = ColorWheel(m_wheelColor);
        for (int i = 0; i < NUM_DIGITS; ++i)
        {
            m_digits[i]->SetColor(color);
        }
    }

    virtual void DoOncePerSecond()
    {
    }

    virtual void DoBrightnessAndDisplay()
    {
        const bool isTransitioning = IsTransitioning();
        for (int i = 0; i < NUM_DIGITS; ++i)
        {
            if (m_lastNumbers[i] != m_currentNumbers[i] && isTransitioning)
            {
                float progress = TransitionProgress();

                // display previous number at diminishing brightness
                m_digits[i]->SetBrightness(1.0f - progress);
                m_digits[i]->Display(m_lastNumbers[i]);

                // display new number at increasing brightness, except
                // in PXL mode where it doesn't look good to fade in
                if (m_settings.Get(SETTING_DIGIT_TYPE) == DT_EDGE_LIT)
                {
                    m_digits[i]->SetBrightness(progress);
                }
                else
                {
                    m_digits[i]->SetBrightness(1.0f);
                }
            }
            else
            {
                m_digits[i]->Display(Digit::INVALID);
                m_digits[i]->SetBrightness(1.0f);
            }
            m_digits[i]->Display(m_currentNumbers[i]);
        }
    }

  private:
    virtual bool IsTransitioning()
    {
        return m_timeSinceSecondBegan.Ms() < TRANSITION_TIME;
    }

    float TransitionProgress()
    {
        return (float)m_timeSinceSecondBegan.Ms() / TRANSITION_TIME;
    }
};
using AnimatorPtr_t = std::shared_ptr<Animator>;

class AnimatorCycleColors : public Animator
{
    using Animator::Animator;

  protected:
    virtual void DoColorChanges()
    {
    }

    virtual void DoOncePerSecond() override
    {
        for (int i = 0; i < NUM_DIGITS; ++i)
        {

            m_wheelColor += 16;
            m_digits[i]->SetColor(ColorWheel(m_wheelColor));
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
    virtual void DoColorChanges() override
    {
        int scaledColor = ScaleBrightness(ColorWheel(m_wheelColor), m_brightness);
        for (int i = 0; i < NUM_DIGITS; ++i)
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
            else if (m_brightness < 0.4f)
            {
                m_incrementer = 0.025f;
                m_brightness = 0.4f;
            }
        }
    }
};

// Changes one digit color at a time, flowing to the left.
class AnimatorCycleFlowLeft : public Animator
{
    using Animator::Animator;

  public:
    virtual void DoOncePerSecond() override
    {
        CycleDigitColors(false);
    }

    virtual void ColorButtonPressed(uint8_t wheelColor) override
    {
        CycleDigitColors(true);
    }

    virtual void DoColorChanges()
    {
    }

  private:
    void CycleDigitColors(bool forceRotate = false)
    {
        SetWheelColor(m_wheelColor + 6);
        m_digits[5]->SetColor(ColorWheel(m_wheelColor));
        if (m_currentNumbers[5] == 0 || forceRotate)
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
    virtual void DoColorChanges() override
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

class AnimatorZippy : public Animator
{
    using Animator::Animator;

  private:
    enum
    {
        ZIPPY_TIME = 250,
    };

    float m_zippy{0};

  public:
    virtual void DoColorChanges() override
    {
        for (int i = 0; i < NUM_DIGITS; ++i)
        {
            m_digits[i]->SetColor(ColorWheel(m_wheelColor));
        }
    }

    virtual void DoBrightnessAndDisplay()
    {
        const float ms = m_timeSinceSecondBegan.Ms();

        bool onceEvery10Seconds = m_currentNumbers[5] == 0;
        float zippyTime = ZIPPY_TIME;

        bool isZippy = ms < zippyTime;
        for (int i = 0; i < NUM_DIGITS; ++i)
        {
            if (m_lastNumbers[i] != m_currentNumbers[i] && isZippy || onceEvery10Seconds && isZippy)
            {
                int zippy = m_lastNumbers[i];
                zippy += (ms / zippyTime) * 10;

                if (zippy > 9)
                {
                    zippy -= 10;
                }

                m_digits[i]->AllOff();
                m_digits[i]->Display(zippy);
            }
            else
            {
                m_digits[i]->AllOff();
                m_digits[i]->Display(m_currentNumbers[i]);
            }
        }
    }
};

class AnimatorAltDisplay : public Animator
{
    using Animator::Animator;

  public:
    virtual void DoBrightnessAndDisplay() override
    {
        for (int i = 0; i < NUM_DIGITS; ++i)
        {
            m_digits[i]->AllOff();
            m_digits[i]->SetBrightness(1.0f);
            m_digits[i]->SetColor(ColorWheel(m_wheelColor + 128));
            m_digits[i]->Display(m_currentNumbers[i]);
        }
    };
};

class AnimatorSetTime : public Animator
{
    using Animator::Animator;

  public:
    virtual void DoBrightnessAndDisplay() override
    {
        for (int i = 0; i < NUM_DIGITS; ++i)
        {
            m_digits[i]->AllOff();

            m_digits[i]->SetBrightness(1.0f);
            if (i < DIGIT_5)
            {
                if (m_timeSinceSecondBegan.Ms() > 500)
                {
                    m_digits[i]->SetBrightness(0.8f);
                }

                m_digits[i]->SetColor(ColorWheel(m_wheelColor + 128));
            }
            else
            {
                m_digits[i]->SetColor(ColorWheel(m_wheelColor));
            }

            m_digits[i]->Display(m_currentNumbers[i]);
        }
    };
};

static inline std::shared_ptr<Animator> AnimatorFactory(Settings &settings, DigitPtrs_t digits,
                                                        const AnimationType_e type, uint8_t wheelColor)
{
    switch (type)
    {
    case ANIM_GLOW:
        return std::make_shared<AnimatorGlow>(settings, digits, wheelColor);
    case ANIM_CYCLE_COLORS:
        return std::make_shared<AnimatorCycleColors>(settings, digits, wheelColor);
    case ANIM_CYCLE_FLOW_LEFT:
        return std::make_shared<AnimatorCycleFlowLeft>(settings, digits, wheelColor);
    case ANIM_RAINBOW:
        return std::make_shared<AnimatorRainbow>(settings, digits, wheelColor);
    case ANIM_ZIPPY:
        return std::make_shared<AnimatorZippy>(settings, digits, wheelColor);
    case ANIM_ALT_DISPLAY:
        return std::make_shared<AnimatorAltDisplay>(settings, digits, wheelColor);
    case ANIM_SET_TIME:
        return std::make_shared<AnimatorSetTime>(settings, digits, wheelColor);
    }

    return std::make_shared<Animator>(settings, digits, wheelColor);
}
