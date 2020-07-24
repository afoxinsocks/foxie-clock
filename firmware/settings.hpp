#pragma once
#include <EEPROM.h>

// Note: If adding new settings, ALWAYS ADD AT THE END OF THE LIST.
// Otherwise, settings will be loaded from the wrong location and have
// the wrong value
enum SettingNames_e
{
    SETTING_DIGIT_TYPE = 0,
    SETTING_CUR_BRIGHTNESS,
    SETTING_MIN_BRIGHTNESS,
    SETTING_MAX_BRIGHTNESS,
    SETTING_BLINKING_SEPARATORS,
    SETTING_COLOR,
    SETTING_ANIMATION_TYPE,
    SETTING_24_HOUR_MODE,
    SETTING_FLIP_DISPLAY,

    // Add new settings here

    // Do not add settings after this line
    TOTAL_SETTINGS,
};

// The purpose of this class is to store user configurable settings
// into EEPROM (including emulated EEPROM stored in flash), only storing
// when all values have been updated to save write cycles when possible.
//
// For simplicity, every value stored will be a uint32_t, restricting us
// to 256 total settings.
class Settings
{
  public:
  private:
    enum
    {
        SETTINGS_SIZE = 256, // 256 4-byte values, 1024 bytes total on Artemis
    };

    uint32_t m_storage[SETTINGS_SIZE] = {0};

    static Settings *m_inst;

  public:
    Settings()
    {
        m_inst = this;
        Load();

        // this should only happen on a fresh board
        if (Get(SETTING_DIGIT_TYPE) > 2)
        {
            ResetToDefaults();
        }
    }
    virtual ~Settings()
    {
        m_inst = nullptr;
    }

    static void ResetToDefaults()
    {
        Set(SETTING_DIGIT_TYPE, 1); // 1 is edge lit (acrylics), 2 is pixel display
        Set(SETTING_CUR_BRIGHTNESS, 64);
        Set(SETTING_MIN_BRIGHTNESS, 4);
        Set(SETTING_MAX_BRIGHTNESS, 192);
        Set(SETTING_BLINKING_SEPARATORS, 1);
        Set(SETTING_COLOR, 192);
        Set(SETTING_ANIMATION_TYPE, 1);
        Set(SETTING_24_HOUR_MODE, 0);
        Set(SETTING_FLIP_DISPLAY, 0);

        Save();
    }

    // only call after all settings have been updated
    static void Save()
    {
        // for Artemis, we're going to write the entire block at once
        // to save write cycles
        const size_t blockSize = SETTINGS_SIZE * sizeof(uint32_t);
        const size_t maxAllowedSize = blockSize;
        writeBlockToEEPROM(0, (const uint8_t *)m_inst->m_storage, blockSize, maxAllowedSize);
    }

    static uint32_t Get(const SettingNames_e name)
    {
        return m_inst->m_storage[(const uint32_t)name];
    }

    static void Set(const SettingNames_e name, const uint32_t value)
    {
        m_inst->m_storage[(const uint32_t)name] = value;
    }

  private:
    void Load()
    {
        for (size_t i = 0; i < TOTAL_SETTINGS; ++i)
        {
            EEPROM.get(i * sizeof(uint32_t), m_storage[i]);
        }
    }
};
