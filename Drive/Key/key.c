#include "key.h"
#include "stm32f10x_gpio.h"
#include "delay.h"
#include "OLED.h"
#include "AS608.h"
#include "Usart.h"
#include "Flash.h"

/*********************************************************************
*按键用的PA8-PA11,PB12-PB15
*PA8-PA11为推挽输出
*PB12-PB15为下拉输入
*********************************************************************/
void KEY_Init(void)//按键初始化
{
    
    GPIO_InitTypeDef  GPIO_InitStructure;
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);    //使能A端口时钟
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11;     
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;         //推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//速度50MHz
    GPIO_Init(GPIOA, &GPIO_InitStructure);    //初始化GPIOA8-11    
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);    //使能B端口时钟
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;  
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;        //下拉输入/逐行扫描
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//速度50MHz
    GPIO_Init(GPIOB, &GPIO_InitStructure);    //初始化GPIOB12-15

}

/*********************************************************************
*函数说明： 按键扫描
*返回值  :  按键值
*参数    ：  void
**********************************************************************/
int Key_Scan(void)
{
    int keyValue=0;//按键值
   // u16 WriteVal=0;//给PA口写入的数据
    
    GPIO_Write(GPIOA,((GPIOA->ODR & 0x00ff )| 0x0f00));//让PA8-11输出高电平
    
    if((GPIOA->IDR & 0xf000)==0x0000)//若PB12-PB15全为0，则没有按键按下
        return -1;
    else
    {
        delay_ms(5);//延时消抖
        
        if((GPIOA->IDR & 0xf000)==0x0000)//若PB12-PB15全为0，则刚刚是抖动产生
        return -1;  
    
    }
    GPIO_Write(GPIOA,(GPIOA->ODR & 0xf0ff )| 0x0100);//让PA8-11输出0001，检测第四行
    switch(GPIOB->IDR & 0xf000)
    {
		case 0x1000 : keyValue=4;break;
		case 0x2000 : keyValue=3;break;
		case 0x4000 : keyValue=2;break;
		case 0x8000 : keyValue=1;break;    
	}
    
    GPIO_Write(GPIOA,(GPIOA->ODR & 0xf0ff )| 0x0200);//让PA8-11输出0010，检测第三行
	switch(GPIOB->IDR & 0xf000)
	{
		case 0x1000 : keyValue=8;break;
		case 0x2000 : keyValue=7;break;
		case 0x4000 : keyValue=6;break;
		case 0x8000 : keyValue=5;break; 
	}
    
    GPIO_Write(GPIOA,(GPIOA->ODR & 0xf0ff )| 0x0400);//让PA8-11输出0100，检测第二行
	switch(GPIOB->IDR & 0xf000)
	{
		case 0x1000 : keyValue=12;break;
		case 0x2000 : keyValue=11;break;
		case 0x4000 : keyValue=10;break;
		case 0x8000 : keyValue=9;break; 
	}
    GPIO_Write(GPIOA,(GPIOA->ODR & 0xf0ff )| 0x0800);//让PA8-11输出1000，检测第一行
	switch(GPIOB->IDR & 0xf000)
	{
		case 0x1000 : keyValue=16;break;
		case 0x2000 : keyValue=15;break;
		case 0x4000 : keyValue=14;break;
		case 0x8000 : keyValue=13;break; 
	} 
    return keyValue;
}


void keypolling(void)
{
	u16 key_num = 0;
	key_num = Key_Scan();
	if(key_num == 4)
	{
		delay_ms(300);
		if(key_num == 4)
		{
			OLED_Clear(0);
			OLED_ShowCH(16,0,"S1键添加指纹");
			OLED_ShowCH(16,2,"S2键删除指纹");
			OLED_ShowCH(16,4,"S3键验证指纹");
			OLED_ShowCH(16,6,"S4键    返回");
			while(1)
			{
				key_num=Key_Scan();
				if(key_num == 4)//s4
				{
					delay_ms(300);//延时消抖
					if(key_num == 4)//s4
					{
						OLED_Clear(0);
						break;						
					}
				}
				if(key_num == 3)//s3
				{
					delay_ms(300);//延时消抖
					OLED_Clear(0);
					OLED_ShowCH(32,2,"请按手指");
					press_FR();
				}
				if(key_num == 2)//s2
				{
					delay_ms(300);//延时消抖
					OLED_Clear(0);
					Del_FR(newIdNum);
				}
				if(key_num == 1)//s1
				{
					delay_ms(300);//延时消抖
					if(key_num == 1)
					{
						int temp = findIDNum();	
						newIdNum = temp;
						if(temp != -1)
						{
							OLED_Clear(0);
							Add_FR(temp);
							Usart_SendByte(USART2,0xdd); //结束标志
						}
						else
						{
							;
						}					
					}

				}
				if(key_num == 11)//s1
				{
//					int i = 0,j = 0,index = 0;
//					u16 hh[100];
//		//发送数据
//					u16 temp[50][72] = {0};
//					
//					readIdNum(hh);
//					
//					for(i = 0; i < 100; i++)
//						printf("hh[%d] = %d\n",i,hh[i]);
//					
//					for(i = 0; i < 100; i++)
//						printf("idNum[%d] = %d\n",i,idNum[i]);
//					
//					for(i = 0; i < 50; i++)
//					{
//						if(idNum[i] == 1)  //id号被注册了
//							read_from_flash(i,temp[index++]);
//					}
//					for(i = 0; i < index; i++)
//						for(j = 0; j < 72; j++)
//							printf("temp[%d][%d] = %d",i,j,temp[i][j]);
//					
//					//Usart_SendByte(USART2,0xff); //结束标志
//					
//					for(i = 0; i < 50; i++)
//					{
//						if(idNum[i] == 1)  //id号被注册了
//							initstaff(&staff[i]);
//					}
				}
			}			
		}

	}

}

