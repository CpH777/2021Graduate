#include "timepoiling.h"
#include "OLED.h"
#include "Timer.h"
#include "Rtc.h"


static int month_days[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

#define FEBRUARY		2
#define	STARTOFTIME		1970
#define SECDAY			86400L           /*  һ���ж���s */
#define SECYR			(SECDAY * 365)
#define	leapyear(year)		((year) % 4 == 0)
#define	days_in_year(a) 	(leapyear(a) ? 366 : 365)
#define	days_in_month(a) 	(month_days[(a) - 1])

/* ���жϱ�־���������ж�ʱ��1����ʱ�䱻ˢ��֮����0 */
__IO uint32_t TimeDisplay = 0;

/*���ڣ���Ф������ASCII��*/
char const *WEEK_STR[] = {"��", "һ", "��", "��", "��", "��", "��"};


int countMonthDay(int month)
{
	int day = days_in_month(month);
	return day;
}

/*
 * This only works for the Gregorian calendar - i.e. after 1752 (in the UK)
 */
 /*���㹫��*/
void GregorianDay(struct rtc_time * tm)
{
	int leapsToDate;
	int lastYear;
	int day;
	int MonthOffset[] = { 0,31,59,90,120,151,181,212,243,273,304,334 };

	lastYear=tm->tm_year-1; 

	/*����ӹ�ԪԪ�굽������ǰһ��֮��һ�������˶��ٸ�����*/
	leapsToDate = lastYear/4 - lastYear/100 + lastYear/400;      

     /*������������һ��Ϊ���꣬�Ҽ������·���2��֮����������1�����򲻼�1*/
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

	day += lastYear*365 + leapsToDate + MonthOffset[tm->tm_mon-1] + tm->tm_mday; /*����ӹ�ԪԪ��Ԫ������������һ���ж�����*/

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

	day = tim / SECDAY;			/* �ж����� */
	hms = tim % SECDAY;			/* �����ʱ�䣬��λs */

	/* Hours, minutes, seconds are easy */
	tm->tm_hour = hms / 3600;
	tm->tm_min = (hms % 3600) / 60;
	tm->tm_sec = (hms % 3600) % 60;

	/* Number of years in days */ /*�����ǰ��ݣ���ʼ�ļ������Ϊ1970��*/
	for (i = STARTOFTIME; day >= days_in_year(i); i++) {
		day -= days_in_year(i);
	}
	tm->tm_year = i;

	/* Number of months in days left */ /*���㵱ǰ���·�*/
	if (leapyear(tm->tm_year)) {
		days_in_month(FEBRUARY) = 29;
	}
	for (i = 1; day >= days_in_month(i); i++) {
		day -= days_in_month(i);
	}
	days_in_month(FEBRUARY) = 28;
	tm->tm_mon = i;

	/* Days are what is left over (+1) from all that. *//*���㵱ǰ����*/
	tm->tm_mday = day + 1;

	/*
	 * Determine the day of week
	 */
	
}
/*
 * ��������Time_Display
 * ����  ����ʾ��ǰʱ��ֵ
 * ����  ��-TimeVar RTC����ֵ����λΪ s
 * ���  ����
 * ����  ���ڲ�����
 */	
void Time_Display(uint32_t TimeVar,struct rtc_time *tm)
{
	uint32_t BJ_TimeVar; 	
	/*  �ѱ�׼ʱ��ת��Ϊ����ʱ��*/
	BJ_TimeVar =TimeVar + TIME_ZOOM;

	to_tm(BJ_TimeVar, tm);/*�Ѷ�ʱ����ֵת��Ϊ����ʱ��*/	
	//printf("%d\n",tm->tm_mon);

	OLED_ShowTime(tm->tm_hour,tm->tm_min,tm->tm_sec);
	OLED_ShowData(tm->tm_mon,tm->tm_mday,tm->tm_wday);
	OLED_ShowSlogn();
}


/*
 * ��������Time_Regulate_Get
 * ����  �������û�ʹ�ô������õ�ʱ�䣬
 *         �Ա����ת����ʱ����洢��RTC �����Ĵ����С�
 * ����  �����ڶ�ȡRTCʱ��Ľṹ��ָ��
 * ע��  ���ڴ��ڵ�����������ʱ������������Ҫ�ӻس�
 */
void Time_Regulate_Get(struct rtc_time *tm)
{
	;
}




