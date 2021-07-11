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


/*时间结构体，默认时间2000-01-01 00:00:00*/


struct rtc_time systmtime=
{
0,0,0,1,1,2000,6
};


extern __IO uint32_t TimeDisplay ;

void SysInit()
{
	delay_init();	    	 //延时函数初始化	  
	usart_init(57600); //指纹
	USART2_Config(115200);  //蓝牙   
	//usart1_init(115200);	
	_uart_dma_configuration();  //DMA初始化
	KEY_Init();   //按键初始化
	OLED_Init();			//初始化OLED	
	RTC_CheckAndConfig(&systmtime);
	initIDNum();    //IDnUM初始化
	RTC_NVIC_Config();  //配置RTC中断
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	//设置NVIC中断分组2:2位抢占优先级，2位响应优先级FD
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
		test(); //蓝牙功能测试
	    if (TimeDisplay == 1)
	    {
	      Time_Display( RTC_GetCounter(),&systmtime); 		  
	      TimeDisplay = 0;
	    }
		keypolling();
		Normal_press_FR();
	}
}

