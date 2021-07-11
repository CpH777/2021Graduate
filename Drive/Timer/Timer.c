#include "timer.h"
#include "stm32f10x.h"
#include "stm32f10x_tim.h"
#include "GPIOLIKE51.h"
#include "OLED.h"
#include "Usart.h"


//设置中断产生间隔为1ms，CLK_INT=72，预分频系数 = 1000
/**********************************************************
** 函数名: TIM2_Int_Init
** 功能描述:  基本定时器配置
** 输入参数: 无
** 输出参数: 无
** 说明：定时时间=(预分频数+1）*（计数值+1) /TIM5时钟(72MHz)，单位(s)
   这里溢出时间t=(7200*10000)/72000000s=1s
***********************************************************/
void TIM2_Config(u16 arr,u16 psc)
{
	TIM_TimeBaseInitTypeDef    TIM_TimeBaseStructure;
	NVIC_InitTypeDef           NVIC_InitStructure;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE); //使能TIM2时钟
	//清除中断标志位
	TIM_ClearITPendingBit(TIM2,TIM_IT_Update);//TIM_ClearFlag(TIM2,TIM_FLAG_Update);//两者作用相同
	TIM_TimeBaseStructure.TIM_Period = arr; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //设置用来作为TIMx时钟频率除数的预分频值
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);  //根据指定的参数初始化TIMx的时间基数单位
	TIM_ITConfig(TIM2,TIM_IT_Update, ENABLE);     //使能指定的TIM2中断,允许更新中断
 
	//中断优先级NVIC设置
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;  //TIM2中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //先占优先级0级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  //从优先级3级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);
	
	TIM_Cmd(TIM2, ENABLE);     //使能定时器2
}
//设置中断产生间隔为1ms，CLK_INT=72，预分频系数 = 1000
/**********************************************************
** 函数名: TIM4_Int_Init
** 功能描述:  基本定时器配置
** 输入参数: 无
** 输出参数: 无
** 说明：定时时间=(预分频数+1）*（计数值+1) /TIM6时钟(72MHz)，单位(s)
   这里溢出时间t=(7200*10000)/72000000s=1s
***********************************************************/
void TIM4_Config(u16 arr,u16 psc)
{
	TIM_TimeBaseInitTypeDef    TIM_TimeBaseStructure;
	NVIC_InitTypeDef           NVIC_InitStructure;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE); //使能TIM4时钟
	//清除中断标志位
	TIM_ClearITPendingBit(TIM4,TIM_IT_Update);//TIM_ClearFlag(TIM2,TIM_FLAG_Update);//两者作用相同
	TIM_TimeBaseStructure.TIM_Period = arr; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //设置用来作为TIMx时钟频率除数的预分频值
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure); //根据指定的参数初始化TIMx的时间基数单位
	TIM_ITConfig(TIM4,TIM_IT_Update, ENABLE);     //使能指定的TIM4中断,允许更新中断
 
	//中断优先级NVIC设置
	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;  //TIM4中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //先占优先级0级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  //从优先级3级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);
	
	TIM_Cmd(TIM4, ENABLE);     //使能定时器4
}
 
/**********************************************************
** 函数名: TIM2_IRQHandler
** 功能描述: 定时器2的更新中断服务程序
** 输入参数: 无
** 输出参数: 无
***********************************************************/
void TIM2_IRQHandler(void)  //s中断
{
	if(TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
	{
		TIM_ClearITPendingBit(TIM2,TIM_IT_Update);//清除更新中断标志位
	}
}
/**********************************************************
** 函数名: TIM4_IRQHandler
** 功能描述: 定时器4的更新中断服务程序
** 输入参数: 无
** 输出参数: 无
***********************************************************/
void TIM4_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)
	{	
		TIM_ClearITPendingBit(TIM4,TIM_IT_Update);//清除更新中断标志位
	}
}
/**************************主函数******************************************/
