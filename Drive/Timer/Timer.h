#ifndef __TIMER_H
#define __TIMER_H

#include "stm32f10x.h"
#include "GPIOLIKE51.h"

struct rtc_time {
	int tm_sec;
	int tm_min;
	int tm_hour;
	int tm_mday;
	int tm_mon;
	int tm_year;
	int tm_wday;
};


void TIM2_Config(u16 arr,u16 psc);
void TIM4_Config(u16 arr,u16 psc);

#endif
