#include "rtc_hal.hpp"
#include <RTC.h>

// Uncomment if using the backup clock modification
// #define UseDS3232

#ifdef UseDS3232
#include <DS3231.h>
#include <Wire.h>
DS3231 BackupClock;
#endif

APM3_RTC g_rtc;
void rtc_hal_init()
{
    // force a specific time on boot if desired
    // rtc_hal_setTime() ...

#ifdef UseDS3232
    bool h12, PM;
    rtc_hal_setTime(BackupClock.getHour(h12, PM), BackupClock.getMinute(), BackupClock.getSecond());
#endif

    rtc_hal_update();
}

void rtc_hal_update()
{
    g_rtc.getTime();
}

int rtc_hal_hour()
{
    return g_rtc.hour;
}

int rtc_hal_hourFormat12()
{
    const int hour = g_rtc.hour == 0 ? 12 : g_rtc.hour;
    return hour > 12 ? hour - 12 : hour;
}

int rtc_hal_minute()
{
    return g_rtc.minute;
}

int rtc_hal_second()
{
    return g_rtc.seconds;
}

int rtc_hal_hundredths()
{
    return g_rtc.hundredths;
}

int rtc_hal_millis()
{
    return g_rtc.hundredths * 10;
}

void rtc_hal_setTime(int h, int m, int s)
{
    // clamp inputs
    h = (h >= 24 ? 0 : h);
    m = (m >= 60 ? 0 : m);
    s = (s >= 60 ? 0 : s);

    g_rtc.setTime(h, m, s, 0, g_rtc.dayOfMonth, g_rtc.month, g_rtc.year);

    rtc_hal_update();

#ifdef UseDS3232
    // Set the DS3231 to the current displayed time
    BackupClock.setHour(h);
    BackupClock.setMinute(m);
    BackupClock.setSecond(s);
#endif
}

void rtc_hal_setDate(int d, int m, int y)
{
    g_rtc.setTime(g_rtc.hour, g_rtc.minute, g_rtc.seconds, 0, d, m, y);
    rtc_hal_update();
}
