#include "timer.h"
#include "stm32f10x.h"
#include "stm32f10x_tim.h"
#include "GPIOLIKE51.h"
#include "OLED.h"
#include "Usart.h"


//�����жϲ������Ϊ1ms��CLK_INT=72��Ԥ��Ƶϵ�� = 1000
/**********************************************************
** ������: TIM2_Int_Init
** ��������:  ������ʱ������
** �������: ��
** �������: ��
** ˵������ʱʱ��=(Ԥ��Ƶ��+1��*������ֵ+1) /TIM5ʱ��(72MHz)����λ(s)
   �������ʱ��t=(7200*10000)/72000000s=1s
***********************************************************/
void TIM2_Config(u16 arr,u16 psc)
{
	TIM_TimeBaseInitTypeDef    TIM_TimeBaseStructure;
	NVIC_InitTypeDef           NVIC_InitStructure;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE); //ʹ��TIM2ʱ��
	//����жϱ�־λ
	TIM_ClearITPendingBit(TIM2,TIM_IT_Update);//TIM_ClearFlag(TIM2,TIM_FLAG_Update);//����������ͬ
	TIM_TimeBaseStructure.TIM_Period = arr; //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ	
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);  //����ָ���Ĳ�����ʼ��TIMx��ʱ�������λ
	TIM_ITConfig(TIM2,TIM_IT_Update, ENABLE);     //ʹ��ָ����TIM2�ж�,��������ж�
 
	//�ж����ȼ�NVIC����
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;  //TIM2�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //��ռ���ȼ�0��
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  //�����ȼ�3��
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQͨ����ʹ��
	NVIC_Init(&NVIC_InitStructure);
	
	TIM_Cmd(TIM2, ENABLE);     //ʹ�ܶ�ʱ��2
}
//�����жϲ������Ϊ1ms��CLK_INT=72��Ԥ��Ƶϵ�� = 1000
/**********************************************************
** ������: TIM4_Int_Init
** ��������:  ������ʱ������
** �������: ��
** �������: ��
** ˵������ʱʱ��=(Ԥ��Ƶ��+1��*������ֵ+1) /TIM6ʱ��(72MHz)����λ(s)
   �������ʱ��t=(7200*10000)/72000000s=1s
***********************************************************/
void TIM4_Config(u16 arr,u16 psc)
{
	TIM_TimeBaseInitTypeDef    TIM_TimeBaseStructure;
	NVIC_InitTypeDef           NVIC_InitStructure;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE); //ʹ��TIM4ʱ��
	//����жϱ�־λ
	TIM_ClearITPendingBit(TIM4,TIM_IT_Update);//TIM_ClearFlag(TIM2,TIM_FLAG_Update);//����������ͬ
	TIM_TimeBaseStructure.TIM_Period = arr; //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ	
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure); //����ָ���Ĳ�����ʼ��TIMx��ʱ�������λ
	TIM_ITConfig(TIM4,TIM_IT_Update, ENABLE);     //ʹ��ָ����TIM4�ж�,��������ж�
 
	//�ж����ȼ�NVIC����
	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;  //TIM4�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //��ռ���ȼ�0��
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  //�����ȼ�3��
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQͨ����ʹ��
	NVIC_Init(&NVIC_InitStructure);
	
	TIM_Cmd(TIM4, ENABLE);     //ʹ�ܶ�ʱ��4
}
 
/**********************************************************
** ������: TIM2_IRQHandler
** ��������: ��ʱ��2�ĸ����жϷ������
** �������: ��
** �������: ��
***********************************************************/
void TIM2_IRQHandler(void)  //s�ж�
{
	if(TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
	{
		TIM_ClearITPendingBit(TIM2,TIM_IT_Update);//��������жϱ�־λ
	}
}
/**********************************************************
** ������: TIM4_IRQHandler
** ��������: ��ʱ��4�ĸ����жϷ������
** �������: ��
** �������: ��
***********************************************************/
void TIM4_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)
	{	
		TIM_ClearITPendingBit(TIM4,TIM_IT_Update);//��������жϱ�־λ
	}
}
/**************************������******************************************/
