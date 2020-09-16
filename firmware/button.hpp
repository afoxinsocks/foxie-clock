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
        HELD,
        REPEAT,
        RELEASE,
    };

  private:
    enum
    {
        DEBOUNCE_MS = 10,
        DEFAULT_REPEAT_RATE_MS = 250,
        WAIT_BEFORE_REPEAT = 350,
    };

    using HandlerFunc_t = std::function<void(const Event_e evt)>;

    static std::vector<Button *> m_buttons;

    const uint8_t m_pin;

    bool m_enabled{true};

    bool m_currentButtonState{false};
    bool m_futureButtonState{false};
    bool m_checkingDebounce{false};
    int m_millisSinceDebounceStarted{0};

    bool m_pressed{false};
    bool m_isRepeating{false};
    bool m_disableEvents{false};

    int m_millisAtPress{0};
    int m_millisSinceRepeat{0};

    bool m_repeatAllowed{true};
    int m_repeatRateMs{DEFAULT_REPEAT_RATE_MS};

    HandlerFunc_t m_handlerFunc;

  public:
    Button(const uint8_t pin)
        : m_pin(pin), m_handlerFunc([](const Event_e evt) {
              // empty function that does nothing by default
          })
    {
        pinMode(m_pin, INPUT_PULLUP);

        // store this new button so it can be checked by Process() later
        m_buttons.push_back(this);
    }

    static void CheckAll()
    {
        for (auto button : m_buttons)
        {
            button->Check();
        }
    }

    void SetHandlerFunc(HandlerFunc_t &&func)
    {
        m_handlerFunc = func;
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

    void DisableEventsUntilRelease()
    {
        m_disableEvents = true;
    }

  private:
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
            SendEvent(HELD);
            CheckForButtonRepeat();
        }
    }

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
                    m_disableEvents = false;
                }

                m_pressed = m_currentButtonState;
            }

            m_checkingDebounce = false;
        }
    }

    void CheckForButtonRepeat()
    {
        if (m_repeatAllowed && TimePressed() > WAIT_BEFORE_REPEAT && ElapsedMsSinceRepeat() > m_repeatRateMs)
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
        if (m_enabled && !m_disableEvents)
        {
            m_handlerFunc(evt);
        }
    }
};
