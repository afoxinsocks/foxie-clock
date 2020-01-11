#pragma once

void rtc_hal_init();
void rtc_hal_update();

int rtc_hal_hour();
int rtc_hal_hourFormat12();
int rtc_hal_minute();
int rtc_hal_second();

void rtc_hal_setTime(int h, int m, int s);
void rtc_hal_setDate(int d, int m, int y);
