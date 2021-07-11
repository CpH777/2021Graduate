#ifndef __TIMEPOILLING_H
#define __TIMEPOILLING_H

#include "stm32f10x.h"
#include "timer.h"


void timepoiling(void);
void Time_Regulate_Get(struct rtc_time *tm);
void Time_Display(uint32_t TimeVar,struct rtc_time *tm);
void to_tm(u32 tim, struct rtc_time * tm);
u32 mktimev(struct rtc_time *tm);
void GregorianDay(struct rtc_time * tm);
int countMonthDay(int month);


#endif
