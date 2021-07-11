#include "stm32f10x.h"
#include "stm32f10x_rtc.h"
#include "GPIOLIKE51.h"
#include "OLED.h"
#include "Delay.h"
#include "Timer.h"
#include "timepoiling.h"
#include "Usart.h"
#include "AS608.h"
#include "key.h"
#include "Rtc.h"
#include "BlueTooth.h"
#include "Flash.h"


/*ʱ��ṹ�壬Ĭ��ʱ��2000-01-01 00:00:00*/


struct rtc_time systmtime=
{
0,0,0,1,1,2000,6
};


extern __IO uint32_t TimeDisplay ;

void SysInit()
{
	delay_init();	    	 //��ʱ������ʼ��	  
	usart_init(57600); //ָ��
	USART2_Config(115200);  //����   
	//usart1_init(115200);	
	_uart_dma_configuration();  //DMA��ʼ��
	KEY_Init();   //������ʼ��
	OLED_Init();			//��ʼ��OLED	
	RTC_CheckAndConfig(&systmtime);
	initIDNum();    //IDnUM��ʼ��
	RTC_NVIC_Config();  //����RTC�ж�
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	//����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�FD
}

int main()
{
	SysInit();
	OLED_Clear(0); 
	OLED_ShowTime(systmtime.tm_hour,systmtime.tm_min,systmtime.tm_sec);
	OLED_ShowData(systmtime.tm_mon,systmtime.tm_mday,systmtime.tm_wday);	 
	OLED_ShowSlogn();
	while(1)
	{
		test(); //�������ܲ���
	    if (TimeDisplay == 1)
	    {
	      Time_Display( RTC_GetCounter(),&systmtime); 		  
	      TimeDisplay = 0;
	    }
		keypolling();
		Normal_press_FR();
	}
}

