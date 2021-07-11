#include "timepoiling.h"
#include "OLED.h"
#include "Timer.h"
#include "Rtc.h"


static int month_days[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

#define FEBRUARY		2
#define	STARTOFTIME		1970
#define SECDAY			86400L           /*  一天有多少s */
#define SECYR			(SECDAY * 365)
#define	leapyear(year)		((year) % 4 == 0)
#define	days_in_year(a) 	(leapyear(a) ? 366 : 365)
#define	days_in_month(a) 	(month_days[(a) - 1])

/* 秒中断标志，进入秒中断时置1，当时间被刷新之后清0 */
__IO uint32_t TimeDisplay = 0;

/*星期，生肖用文字ASCII码*/
char const *WEEK_STR[] = {"日", "一", "二", "三", "四", "五", "六"};


int countMonthDay(int month)
{
	int day = days_in_month(month);
	return day;
}

/*
 * This only works for the Gregorian calendar - i.e. after 1752 (in the UK)
 */
 /*计算公历*/
void GregorianDay(struct rtc_time * tm)
{
	int leapsToDate;
	int lastYear;
	int day;
	int MonthOffset[] = { 0,31,59,90,120,151,181,212,243,273,304,334 };

	lastYear=tm->tm_year-1; 

	/*计算从公元元年到计数的前一年之中一共经历了多少个闰年*/
	leapsToDate = lastYear/4 - lastYear/100 + lastYear/400;      

     /*如若计数的这一年为闰年，且计数的月份在2月之后，则日数加1，否则不加1*/
	if((tm->tm_year%4==0) &&
	   ((tm->tm_year%100!=0) || (tm->tm_year%400==0)) &&
	   (tm->tm_mon>2)) {
		/* 
		 * We are past Feb. 29 in a leap year
		 */
		day=1;
	} else {
		day=0;
	}

	day += lastYear*365 + leapsToDate + MonthOffset[tm->tm_mon-1] + tm->tm_mday; /*计算从公元元年元旦到计数日期一共有多少天*/

	tm->tm_wday=day%7;
}

u32 mktimev(struct rtc_time *tm)
{
	if (0 >= (int) (tm->tm_mon -= 2)) {	/* 1..12 -> 11,12,1..10 */
		tm->tm_mon += 12;		/* Puts Feb last since it has leap day */
		tm->tm_year -= 1;
	}

	return (((
		(u32) (tm->tm_year/4 - tm->tm_year/100 + tm->tm_year/400 + 367*tm->tm_mon/12 + tm->tm_mday) +
			tm->tm_year*365 - 719499
	    )*24 + tm->tm_hour /* now have hours */
	  )*60 + tm->tm_min /* now have minutes */
	)*60 + tm->tm_sec; /* finally seconds */	 
}

void to_tm(u32 tim, struct rtc_time * tm)
{
	register u32    i;
	register long   hms, day;

	day = tim / SECDAY;			/* 有多少天 */
	hms = tim % SECDAY;			/* 今天的时间，单位s */

	/* Hours, minutes, seconds are easy */
	tm->tm_hour = hms / 3600;
	tm->tm_min = (hms % 3600) / 60;
	tm->tm_sec = (hms % 3600) % 60;

	/* Number of years in days */ /*算出当前年份，起始的计数年份为1970年*/
	for (i = STARTOFTIME; day >= days_in_year(i); i++) {
		day -= days_in_year(i);
	}
	tm->tm_year = i;

	/* Number of months in days left */ /*计算当前的月份*/
	if (leapyear(tm->tm_year)) {
		days_in_month(FEBRUARY) = 29;
	}
	for (i = 1; day >= days_in_month(i); i++) {
		day -= days_in_month(i);
	}
	days_in_month(FEBRUARY) = 28;
	tm->tm_mon = i;

	/* Days are what is left over (+1) from all that. *//*计算当前日期*/
	tm->tm_mday = day + 1;

	/*
	 * Determine the day of week
	 */
	
}
/*
 * 函数名：Time_Display
 * 描述  ：显示当前时间值
 * 输入  ：-TimeVar RTC计数值，单位为 s
 * 输出  ：无
 * 调用  ：内部调用
 */	
void Time_Display(uint32_t TimeVar,struct rtc_time *tm)
{
	uint32_t BJ_TimeVar; 	
	/*  把标准时间转换为北京时间*/
	BJ_TimeVar =TimeVar + TIME_ZOOM;

	to_tm(BJ_TimeVar, tm);/*把定时器的值转换为北京时间*/	
	//printf("%d\n",tm->tm_mon);

	OLED_ShowTime(tm->tm_hour,tm->tm_min,tm->tm_sec);
	OLED_ShowData(tm->tm_mon,tm->tm_mday,tm->tm_wday);
	OLED_ShowSlogn();
}


/*
 * 函数名：Time_Regulate_Get
 * 描述  ：保存用户使用串口设置的时间，
 *         以便后面转化成时间戳存储到RTC 计数寄存器中。
 * 输入  ：用于读取RTC时间的结构体指针
 * 注意  ：在串口调试助手输入时，输入完数字要加回车
 */
void Time_Regulate_Get(struct rtc_time *tm)
{
	;
}




