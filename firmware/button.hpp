#pragma once
#include <functional>

class Button
{
private:
    using Func_t = std::function<void()>;
    const uint8_t m_pin;

    bool m_newState{false};
    bool m_changingState{false};
    int m_millisAtStateChange{0};

    bool m_pressed{false};
    int m_millisAtPress{0};

    Func_t m_pressFunc;
    Func_t m_releaseFunc;
    
public:
    enum
    {
        DEBOUNCE_MS = 10,
        REPEAT_MS = 200,
    };

    Button(const uint8_t pin)
    : m_pin(pin)
    {
        pinMode(m_pin, INPUT_PULLUP);
    }

    void SetPressFunc(Func_t &&func)
    {
        m_pressFunc = func;
    }

    void SetReleaseFunc(Func_t &&func)
    {
        m_releaseFunc = func;
    }

    void Check()
    {
        const bool pressed = digitalRead(m_pin) == 0;

        if (m_changingState == false && pressed != m_pressed)
        {
            m_millisAtStateChange = millis();
            m_newState = pressed;
            m_changingState = true;
        }
        else if (m_changingState)
        {
            if ((int)millis() - m_millisAtStateChange > DEBOUNCE_MS)
            {
                if (pressed == m_newState)
                {
                    // state change occurred, do something
                    m_pressed = m_newState;

                    if (m_pressed && m_pressFunc)
                    {
                        m_millisAtPress = millis();
                        m_pressFunc();
                    }
                    else if (!m_pressed && m_releaseFunc)
                    {
                        m_releaseFunc();
                    }

                }
                m_changingState = false;
            }
        }

        // handle repeat
        if (m_pressed && (int)millis() - m_millisAtPress > REPEAT_MS)
        {
            m_millisAtPress = millis();
            if (m_pressFunc)
            {
                m_pressFunc();
            }
        }
    }
};
