#pragma once

class ElapsedTime
{
  private:
    int m_millis{0};

  public:
    ElapsedTime()
    {
        Reset();
    }

    void Reset()
    {
        m_millis = millis();
    }

    int Ms()
    {
        return ((int)millis()) - m_millis;
    }
};
