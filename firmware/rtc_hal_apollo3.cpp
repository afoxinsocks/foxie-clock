#include <RTC.h>
#include "rtc_hal.hpp"

APM3_RTC g_rtc;
void rtc_hal_init()
{
	g_rtc.setTime(0, 0, 0, 0, 1, 1, 20);
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

void rtc_hal_setTime(int h, int m, int s)
{
	// clamp inputs
	h = (h >= 24 ? 0 : h);
	m = (m >= 60 ? 0 : m);
	s = (s >= 60 ? 0 : s);

	g_rtc.setTime(h, m, s, 0, g_rtc.dayOfMonth, g_rtc.month, g_rtc.year);
	rtc_hal_update();
}

void rtc_hal_setDate(int d, int m, int y)
{
	g_rtc.setTime(g_rtc.hour, g_rtc.minute, g_rtc.seconds, 0, d, m, y);
	rtc_hal_update();
}