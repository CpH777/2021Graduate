#include "key.h"
#include "stm32f10x_gpio.h"
#include "delay.h"
#include "OLED.h"
#include "AS608.h"
#include "Usart.h"
#include "Flash.h"

/*********************************************************************
*�����õ�PA8-PA11,PB12-PB15
*PA8-PA11Ϊ�������
*PB12-PB15Ϊ��������
*********************************************************************/
void KEY_Init(void)//������ʼ��
{
    
    GPIO_InitTypeDef  GPIO_InitStructure;
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);    //ʹ��A�˿�ʱ��
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11;     
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;         //�������
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//�ٶ�50MHz
    GPIO_Init(GPIOA, &GPIO_InitStructure);    //��ʼ��GPIOA8-11    
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);    //ʹ��B�˿�ʱ��
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;  
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;        //��������/����ɨ��
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//�ٶ�50MHz
    GPIO_Init(GPIOB, &GPIO_InitStructure);    //��ʼ��GPIOB12-15

}

/*********************************************************************
*����˵���� ����ɨ��
*����ֵ  :  ����ֵ
*����    ��  void
**********************************************************************/
int Key_Scan(void)
{
    int keyValue=0;//����ֵ
   // u16 WriteVal=0;//��PA��д�������
    
    GPIO_Write(GPIOA,((GPIOA->ODR & 0x00ff )| 0x0f00));//��PA8-11����ߵ�ƽ
    
    if((GPIOA->IDR & 0xf000)==0x0000)//��PB12-PB15ȫΪ0����û�а�������
        return -1;
    else
    {
        delay_ms(5);//��ʱ����
        
        if((GPIOA->IDR & 0xf000)==0x0000)//��PB12-PB15ȫΪ0����ո��Ƕ�������
        return -1;  
    
    }
    GPIO_Write(GPIOA,(GPIOA->ODR & 0xf0ff )| 0x0100);//��PA8-11���0001����������
    switch(GPIOB->IDR & 0xf000)
    {
		case 0x1000 : keyValue=4;break;
		case 0x2000 : keyValue=3;break;
		case 0x4000 : keyValue=2;break;
		case 0x8000 : keyValue=1;break;    
	}
    
    GPIO_Write(GPIOA,(GPIOA->ODR & 0xf0ff )| 0x0200);//��PA8-11���0010����������
	switch(GPIOB->IDR & 0xf000)
	{
		case 0x1000 : keyValue=8;break;
		case 0x2000 : keyValue=7;break;
		case 0x4000 : keyValue=6;break;
		case 0x8000 : keyValue=5;break; 
	}
    
    GPIO_Write(GPIOA,(GPIOA->ODR & 0xf0ff )| 0x0400);//��PA8-11���0100�����ڶ���
	switch(GPIOB->IDR & 0xf000)
	{
		case 0x1000 : keyValue=12;break;
		case 0x2000 : keyValue=11;break;
		case 0x4000 : keyValue=10;break;
		case 0x8000 : keyValue=9;break; 
	}
    GPIO_Write(GPIOA,(GPIOA->ODR & 0xf0ff )| 0x0800);//��PA8-11���1000������һ��
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
			OLED_ShowCH(16,0,"S1�����ָ��");
			OLED_ShowCH(16,2,"S2��ɾ��ָ��");
			OLED_ShowCH(16,4,"S3����ָ֤��");
			OLED_ShowCH(16,6,"S4��    ����");
			while(1)
			{
				key_num=Key_Scan();
				if(key_num == 4)//s4
				{
					delay_ms(300);//��ʱ����
					if(key_num == 4)//s4
					{
						OLED_Clear(0);
						break;						
					}
				}
				if(key_num == 3)//s3
				{
					delay_ms(300);//��ʱ����
					OLED_Clear(0);
					OLED_ShowCH(32,2,"�밴��ָ");
					press_FR();
				}
				if(key_num == 2)//s2
				{
					delay_ms(300);//��ʱ����
					OLED_Clear(0);
					Del_FR(newIdNum);
				}
				if(key_num == 1)//s1
				{
					delay_ms(300);//��ʱ����
					if(key_num == 1)
					{
						int temp = findIDNum();	
						newIdNum = temp;
						if(temp != -1)
						{
							OLED_Clear(0);
							Add_FR(temp);
							Usart_SendByte(USART2,0xdd); //������־
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
//		//��������
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
//						if(idNum[i] == 1)  //id�ű�ע����
//							read_from_flash(i,temp[index++]);
//					}
//					for(i = 0; i < index; i++)
//						for(j = 0; j < 72; j++)
//							printf("temp[%d][%d] = %d",i,j,temp[i][j]);
//					
//					//Usart_SendByte(USART2,0xff); //������־
//					
//					for(i = 0; i < 50; i++)
//					{
//						if(idNum[i] == 1)  //id�ű�ע����
//							initstaff(&staff[i]);
//					}
				}
			}			
		}

	}

}

