#include "BlueTooth.h"
#include "Usart.h"
#include "Delay.h"
#include "Rtc.h"
#include "OLED.h"
#include <string.h>
#include "AS608.h"
#include "Flash.h"

extern u8 usart2receivebuf[1024]; 
extern u8 usart2receiveIen;
extern struct rtc_time systmtime;
extern u16 idNum[30];

void visioPacket() //组包
{
	;
}

void unPcaket() //拆包
{
	;
}

void test(void)	
{
	if(usart2receivebuf[usart2receiveIen] == 0xff)  //同步时间
	{
		int i = 0;
//		for(i = 0; i > usart2receiveIen; i++)
//		{
//			printf("usart2receivebuf : %4d  ",usart2receivebuf[i]);
//		}
		systmtime.tm_year = usart2receivebuf[usart2receiveIen-6] + 2000;
		systmtime.tm_mon = usart2receivebuf[usart2receiveIen-5] - 4;
		systmtime.tm_mday = usart2receivebuf[usart2receiveIen-4] + 8;
		systmtime.tm_hour = usart2receivebuf[usart2receiveIen-3];
		systmtime.tm_min = usart2receivebuf[usart2receiveIen-2];
		systmtime.tm_sec = usart2receivebuf[usart2receiveIen-1];
		
		Time_Adjust(&systmtime);
		usart2receiveIen = 0;
	}
	else if(usart2receivebuf[usart2receiveIen] == 0xef)   //添加指纹
	{
		int tempId = usart2receivebuf[usart2receiveIen - 1];
//		printf("tempId = %d\n",tempId);
		OLED_Clear(0);
		newIdNum = tempId;
		Add_FR(tempId);
		OLED_Clear(0);
		if(idNum[tempId] == 1)
			Usart_SendByte(USART2,0xff); //结束标志
		usart2receiveIen = 0;
	}
	else if(usart2receivebuf[usart2receiveIen] == 0xdf)  //删除指纹
	{
		int tempId = usart2receivebuf[usart2receiveIen - 1];
		OLED_Clear(0);
		Del_FR(tempId);
		OLED_Clear(0);
		if(idNum[tempId] == 0)
			Usart_SendByte(USART2,0xff); //结束标志
		usart2receiveIen = 0;
	}
	else if(usart2receivebuf[usart2receiveIen] == 0xcf)
	{
		int i = 0,j = 0,index = 0;
		u16 hh[30];
		//发送数据
		u16 temp[30][72] = {0}; 
		
		readIdNum(hh);
		//for(i = 0; i < 30; i++)
		//	printf("hh[%d] = %d\n",i,hh[i]);
		
		for(i = 0; i < 30; i++)
		{
			if(hh[i] == 1)  //id号被注册了
				read_from_flash(i,temp[index++]);
		}
		
//		for(i = 0; i < index; i++)
//			for(j = 0; j < 72; j++)
//				printf("temp[%d][%d] = %d\n",i,j,temp[i][j]);
		
		for(i = 0; i < index; i++)
			for(j = 0; j < 72; j++)
				Usart_SendByte(USART2,temp[i][j]);
		
		Usart_SendByte(USART2,0xcf); //结束标志
		
		for(i = 0; i < 30; i++)
		{
			if(hh[i] == 1)  //id号被注册了
				initstaff(&staff[i]);
		}
		
		usart2receiveIen = 0;
	}
	
}



