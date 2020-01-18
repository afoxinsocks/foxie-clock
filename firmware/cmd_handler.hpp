#pragma once
#include <vector>
#include "clock.hpp"

class CmdHandler
{
private:
    static CmdHandler *m_inst;
    std::vector<uint8_t> m_rx;

    enum State_e
    {
        STATE_WAIT,
        STATE_RX,
    };

    enum Command_e
    {
        CMD_SET_TIME = 0x10,
    };

    Clock &m_clock;
    State_e m_state{STATE_WAIT};
    uint8_t m_expectingBytes{0};

public:
    CmdHandler(Clock& clock)
    : m_clock(clock)
    {
        m_inst = this;
    }

    void Process()
    {
        if (m_state == STATE_WAIT)
        {
            // size of payload received
            m_expectingBytes = m_rx[0];
            m_state = STATE_RX;
            m_rx.clear();
        }
        else if (m_state == STATE_RX && m_rx.size() == m_expectingBytes)
        {
            // all data received, do something with it
            switch (m_rx[0])
            {
                case CMD_SET_TIME:
                {
                    int h = m_rx[1];
                    int m = m_rx[2];
                    int s = m_rx[3];
                    rtc_hal_setTime(h, m, s + 1);
                    m_clock.RedrawIfNeeded(true);
                }
                break;
            }
            m_state = STATE_WAIT;
            m_rx.clear();
        }
    }

    static void ReceiveByte(const uint8_t val)
    {
        m_inst->m_rx.push_back(val);
        m_inst->Process();
    }
};
