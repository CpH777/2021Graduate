#include "Rtc.h"
#include "stm32f10x_bkp.h"
#include "stm32f10x_pwr.h"
#include "stm32f10x_rtc.h"
#include "OLED.h"
#include "timepoiling.h"




extern __IO uint32_t TimeDisplay ;

void RTC_IRQHandler(void)
{
	  if (RTC_GetITStatus(RTC_IT_SEC) != RESET)
	  {
	    /* Clear the RTC Second interrupt */
	    RTC_ClearITPendingBit(RTC_IT_SEC);
	
	    /* Enable time update */
	    TimeDisplay = 1;
	
	    /* Wait until last write operation on RTC registers has finished */
	    RTC_WaitForLastTask();
	  }
}
/*
 * ��������NVIC_Configuration
 * ����  ������RTC���жϵ����ж����ȼ�Ϊ1�������ȼ�Ϊ0
 * ����  ����
 * ���  ����
 * ����  ���ⲿ����
 */
void RTC_NVIC_Config(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	
	/* Configure one bit for preemption priority */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
	
	/* Enable the RTC Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}


/*
 * ��������RTC_CheckAndConfig
 * ����  ����鲢����RTC
 * ����  �����ڶ�ȡRTCʱ��Ľṹ��ָ��
 * ���  ����
 * ����  ���ⲿ����
 */
void RTC_CheckAndConfig(struct rtc_time *tm)
{
   	/*������ʱ��鱸�ݼĴ���BKP_DR1��������ݲ���0xA5A5,
	  ������������ʱ�䲢ѯ���û�����ʱ��*/
	if (BKP_ReadBackupRegister(RTC_BKP_DRX) != RTC_BKP_DATA)
	{
		/* ʹ��tm��ʱ������RTC�Ĵ��� */
		Time_Adjust(tm);
		
		/*��BKP_DR1�Ĵ���д���־��˵��RTC��������*/
		BKP_WriteBackupRegister(RTC_BKP_DRX, RTC_BKP_DATA);
	}
	else
	{
		
		/* ʹ�� PWR �� Backup ʱ�� */
	  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
	
		/* ������� Backup ���� */
	  PWR_BackupAccessCmd(ENABLE);
		
	  /*LSE��������������ʱ��*/
		
#ifdef RTC_CLOCK_SOURCE_LSI		
			/* ʹ�� LSI */
			RCC_LSICmd(ENABLE);

			/* �ȴ� LSI ׼���� */
			while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET)
			{}
#endif

		/*����Ƿ��������*/
		if (RCC_GetFlagStatus(RCC_FLAG_PORRST) != RESET)
		{
		}
		/*����Ƿ�Reset��λ*/
		else if (RCC_GetFlagStatus(RCC_FLAG_PINRST) != RESET)
		{
		}
	
		/*�ȴ��Ĵ���ͬ��*/
		RTC_WaitForSynchro();
		
		/*����RTC���ж�*/
		RTC_ITConfig(RTC_IT_SEC, ENABLE);
		
		/*�ȴ��ϴ�RTC�Ĵ���д�������*/
		RTC_WaitForLastTask();
	}
	   /*������ʱ������꣬������У��ʱ�������PC13*/
	#ifdef RTCClockOutput_Enable
	/* ʹ�� PWR �� Backup ʱ�� */
	  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
	
	/* ������� Backup ���� */
	  PWR_BackupAccessCmd(ENABLE);
	
	  /* ��ֹ Tamper ���� */
	  /* Ҫ��� RTCCLK/64 �� Tamper ����,  tamper ���ܱ����ֹ */	
	  BKP_TamperPinCmd(DISABLE); 
	
	  /* ʹ�� RTC ʱ������� Tamper ���� */
	  BKP_RTCOutputConfig(BKP_RTCOutputSource_CalibClock);
	#endif
	
	  /* �����λ��־ flags */
	  RCC_ClearFlag();

}



/*
 * ��������RTC_Configuration
 * ����  ������RTC
 * ����  ����
 * ���  ����
 * ����  ���ⲿ����
 */
void RTC_Configuration(void)
{
	/* ʹ�� PWR �� Backup ʱ�� */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
	
	/* ������� Backup ���� */
	PWR_BackupAccessCmd(ENABLE);
	
	/* ��λ Backup ���� */
	BKP_DeInit();
	
//ʹ���ⲿʱ�ӻ����ڲ�ʱ�ӣ���bsp_rtc.h�ļ����壩	
//ʹ���ⲿʱ��ʱ������Щ����¾�������
//������Ʒ��ʱ�򣬺����׳����ⲿ����������������̫�ɿ�	
#ifdef 	RTC_CLOCK_SOURCE_LSE
	/* ʹ�� LSE */
	RCC_LSEConfig(RCC_LSE_ON);
	
	/* �ȴ� LSE ׼���� */
	while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)
	{}
	
	/* ѡ�� LSE ��Ϊ RTC ʱ��Դ */
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
	
	/* ʹ�� RTC ʱ�� */
	RCC_RTCCLKCmd(ENABLE);
	
	/* �ȴ� RTC �Ĵ��� ͬ��
	 * ��ΪRTCʱ���ǵ��ٵģ��ڻ�ʱ���Ǹ��ٵģ�����Ҫͬ��
	 */
	RTC_WaitForSynchro();
	
	/* ȷ����һ�� RTC �Ĳ������ */
	RTC_WaitForLastTask();
	
	/* ʹ�� RTC ���ж� */
	RTC_ITConfig(RTC_IT_SEC, ENABLE);
	
	/* ȷ����һ�� RTC �Ĳ������ */
	RTC_WaitForLastTask();
	
	/* ���� RTC ��Ƶ: ʹ RTC ����Ϊ1s  */
	/* RTC period = RTCCLK/RTC_PR = (32.768 KHz)/(32767+1) = 1HZ */
	RTC_SetPrescaler(32767); 
	
	/* ȷ����һ�� RTC �Ĳ������ */
	RTC_WaitForLastTask();
	
#else

	/* ʹ�� LSI */
	RCC_LSICmd(ENABLE);

	/* �ȴ� LSI ׼���� */
	while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET)
	{}
	
	/* ѡ�� LSI ��Ϊ RTC ʱ��Դ */
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);
	
	/* ʹ�� RTC ʱ�� */
	RCC_RTCCLKCmd(ENABLE);
	
	/* �ȴ� RTC �Ĵ��� ͬ��
	 * ��ΪRTCʱ���ǵ��ٵģ��ڻ�ʱ���Ǹ��ٵģ�����Ҫͬ��
	 */
	RTC_WaitForSynchro();
	
	/* ȷ����һ�� RTC �Ĳ������ */
	RTC_WaitForLastTask();
	
	/* ʹ�� RTC ���ж� */
	RTC_ITConfig(RTC_IT_SEC, ENABLE);
	
	/* ȷ����һ�� RTC �Ĳ������ */
	RTC_WaitForLastTask();
	
	/* ���� RTC ��Ƶ: ʹ RTC ����Ϊ1s ,LSIԼΪ40KHz */
	/* RTC period = RTCCLK/RTC_PR = (40 KHz)/(40000-1+1) = 1HZ */	
	RTC_SetPrescaler(40000-1); 
	
	/* ȷ����һ�� RTC �Ĳ������ */
	RTC_WaitForLastTask();
#endif
	
}

/*
 * ��������Time_Adjust
 * ����  ��ʱ�����
 * ����  �����ڶ�ȡRTCʱ��Ľṹ��ָ�루����ʱ�䣩
 * ���  ����
 * ����  ���ⲿ����
 */
void Time_Adjust(struct rtc_time *tm)
{
		/* RTC ���� */
	RTC_Configuration();

	/* �ȴ�ȷ����һ�β������ */
	RTC_WaitForLastTask();
	  
	/* �������� */
	GregorianDay(tm);

	/* �����ڼ���ʱ�����д�뵽RTC�����Ĵ��� */  //��sд��ȥ
	RTC_SetCounter(mktimev(tm)-TIME_ZOOM);

	/* �ȴ�ȷ����һ�β������ */
	RTC_WaitForLastTask();
}




