#pragma once
#include <algorithm>
#include <functional>
#include <vector>

#include "elapsed_time.hpp"

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
    std::vector<Pins_e> m_pins; // a Button can have multiple input pins
    bool m_currentPinState{false};

    bool m_enabled{true};
    bool m_isPressed{false};
    bool m_wasPressed{false};

    ElapsedTime m_timeInState;
    ElapsedTime m_timeSinceRepeat;

  public:
    using HandlerFunc_t = std::function<void(const Event_e evt)>;
    struct Config
    {
        bool canRepeat{false};

        // values below are in milliseconds
        int repeatRate{100};
        int delayBeforeRepeat{400};
        int delayBeforePress{0}; // used for button combinations
        int debounceTime{20};
        HandlerFunc_t handlerFunc;
    } config;

    Button(Pins_e pin) : Button({pin})
    {
    }

    Button(std::initializer_list<Pins_e> pins)
    {
        m_pins = pins;
        for (auto pin : m_pins)
        {
            pinMode(pin, INPUT_PULLUP);
        }
    }

    void Update() // Called repeatedly by owner
    {
        if (m_enabled && !AreOtherButtonsPressed())
        {
            CheckForPinStateChange();
            CheckForEventsToSend();
        }
    }

    void SetEnabled(const bool enabled)
    {
        m_enabled = enabled;
        Reset();
    }

    bool IsPressed()
    {
        return m_enabled && m_isPressed;
    }

    void Reset()
    {
        m_timeInState.Reset();
        m_isPressed = false;
        m_wasPressed = false;
    }

  private:
    void CheckForPinStateChange()
    {
        if (m_timeInState.Ms() < config.debounceTime)
        {
            // do nothing while we're debouncing because this is
            // when the button can physically bounce.

            return;
        }

        bool lastPinState = m_currentPinState;
        m_currentPinState = GetCombinedPinState();
        if (lastPinState != m_currentPinState && m_currentPinState != m_isPressed)
        {
            m_timeInState.Reset();
            m_isPressed = m_currentPinState;
        }
    }

    void CheckForEventsToSend()
    {
        if (!config.handlerFunc)
        {
            return;
        }

        if (m_isPressed != m_wasPressed)
        {
            if (m_isPressed && m_timeInState.Ms() < config.delayBeforePress)
            {
                return;
            }

            m_wasPressed = m_isPressed;
            m_timeSinceRepeat.Reset();

            config.handlerFunc(m_isPressed ? PRESS : RELEASE);
        }
        else if (m_isPressed && (m_timeInState.Ms() >= config.delayBeforeRepeat) &&
                 (m_timeSinceRepeat.Ms() >= config.repeatRate))
        {
            m_timeSinceRepeat.Reset();
            if (config.canRepeat)
            {
                config.handlerFunc(REPEAT);
            }
        }
    }

    bool GetCombinedPinState()
    {
        bool state = true;
        for (auto pin : m_pins)
        {
            state = state && (digitalRead(pin) == 0);
        }
        return state;
    }

    bool AreOtherButtonsPressed()
    {
        std::vector<Pins_e> pins = {PIN_BTN_H, PIN_BTN_M, PIN_BTN_C, PIN_BTN_H};
        for (auto pin : pins)
        {
            if (std::find(m_pins.begin(), m_pins.end(), pin) == m_pins.end() && digitalRead(pin) == 0)
            {
                Reset();
                return true;
            }
        }
        return false;
    }
};
