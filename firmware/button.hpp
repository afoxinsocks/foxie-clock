 #pragma once

class Button
{
private:
    const uint8_t m_pin;
    int m_start{0};
    bool m_held{false};
    bool m_released{false};
    
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

    void Check()
    {
        bool pressed = digitalRead(m_pin) == 0;
        if (pressed)
        {
            if (m_start == 0)
            {
                m_start = millis();
            }
        }
        else
        {
            m_start = 0;
            if (m_held)
            {
                m_held = false;
                m_released = true;
            }
        }

        if (pressed && millis() - m_start > DEBOUNCE_MS)
        {
            m_held = true;
        }
    }

    bool WasPressed()
    {
        const int duration = millis() - m_start;
        bool released = m_released;
        m_released = false;

        if (m_held && duration >= REPEAT_MS)
        {
            released = true; // simulate button press release
        }

        if (released)
        {
            m_start = millis();
        }
        return released;
    }
};
