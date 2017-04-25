#ifndef _RTC_H
#define _RTC_H

#include "main.h"

void rtc_init(void);
void rtc_set_time(void *p);
void rtc_get_time(void *p);
void rtc_set_sec(int);
int  rtc_get_sec(void);

#endif /* rtc.h */
