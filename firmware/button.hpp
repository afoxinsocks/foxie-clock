#pragma once
#include <functional>

// Debouncing button class with press, repeat, release events that
// are sent to callback handler
class Button
{
  public:
    enum Event_e
    {
        PRESS,
        REPEAT,
        RELEASE,
    };

  private:
    enum
    {
        DEBOUNCE_MS = 10,
        DEFAULT_REPEAT_RATE_MS = 250,
    };

    using Func_t = std::function<void(const Event_e evt)>;
    const uint8_t m_pin;

    bool m_enabled{true};

    bool m_currentButtonState{false};
    bool m_futureButtonState{false};
    bool m_checkingDebounce{false};
    int m_millisSinceDebounceStarted{0};

    bool m_pressed{false};
    int m_millisAtPress{0};
    int m_millisSinceRepeat{0};

    bool m_repeatAllowed{true};
    int m_repeatRateMs{DEFAULT_REPEAT_RATE_MS};

    Func_t m_func;

  public:
    Button(const uint8_t pin)
        : m_pin(pin), m_func([](const Event_e evt) {
              // empty function that does nothing by default
          })
    {
        pinMode(m_pin, INPUT_PULLUP);
    }

    void SetHandlerFunc(Func_t &&func)
    {
        m_func = func;
    }

    void SetEnabled(const bool enabled)
    {
        m_enabled = enabled;

        // reset existing state
        m_checkingDebounce = false;
        m_pressed = false;
    }

    void SetAllowRepeat(const bool allow)
    {
        m_repeatAllowed = allow;
    }

    void SetRepeatRate(const int repeatRateMs)
    {
        m_repeatRateMs = repeatRateMs;
    }

    void Check()
    {
        m_currentButtonState = (digitalRead(m_pin) == 0);

        if (!m_checkingDebounce && m_currentButtonState != m_pressed)
        {
            BeginDebouncing();
        }
        else if (m_checkingDebounce)
        {
            CheckDebouncingProgress();
        }
        else if (m_pressed)
        {
            CheckForButtonRepeat();
        }
    }

    bool IsPressed()
    {
        return m_enabled && m_pressed;
    }

    int TimePressed()
    {
        if (!m_enabled || !m_pressed)
        {
            return 0;
        }
        return (int)millis() - m_millisAtPress;
    }

  private:
    void BeginDebouncing()
    {
        m_millisSinceDebounceStarted = millis();
        m_futureButtonState = m_currentButtonState;
        m_checkingDebounce = true;
    }

    void CheckDebouncingProgress()
    {
        if (ElapsedMsSinceStateChange() > DEBOUNCE_MS)
        {
            if (m_currentButtonState == m_futureButtonState)
            {
                if (m_currentButtonState)
                {
                    m_millisAtPress = m_millisSinceRepeat = millis();
                    SendEvent(PRESS);
                }
                else
                {
                    SendEvent(RELEASE);
                }

                m_pressed = m_currentButtonState;
            }

            m_checkingDebounce = false;
        }
    }

    void CheckForButtonRepeat()
    {
        if (ElapsedMsSinceRepeat() > m_repeatRateMs && m_repeatAllowed)
        {
            m_millisSinceRepeat = millis();
            SendEvent(REPEAT);
        }
    }

    int ElapsedMsSinceRepeat()
    {
        return (int)millis() - m_millisSinceRepeat;
    }

    int ElapsedMsSinceStateChange()
    {
        return (int)millis() - m_millisSinceDebounceStarted;
    }

    void SendEvent(const Event_e evt)
    {
        if (m_enabled)
        {
            m_func(evt);
        }
    }
};
