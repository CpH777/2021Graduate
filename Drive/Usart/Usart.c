#include "Usart.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_dma.h"
#include "stm32f10x_tim.h"
#include "delay.h"	 
#include "stdio.h"	 	 
#include "string.h"	 
#include "Timer.h"
#include <stdarg.h>

u8 sendbuf[1024];
u8 receivebuf[1024]; 

u8 usart2sendbuf[1024];
u8 usart2receivebuf[1024]; 
//���ڽ��ջ����� 	
u8 USART3_RX_BUF[USART3_MAX_RECV_LEN]; 				//���ջ���,���USART3_MAX_RECV_LEN���ֽ�.
u8 USART3_TX_BUF[USART3_MAX_SEND_LEN]; 			  //���ͻ���,���USART3_MAX_SEND_LEN�ֽ�

//ͨ���жϽ�������2���ַ�֮���ʱ������10ms�������ǲ���һ������������.
//���2���ַ����ռ������10ms,����Ϊ����1����������.Ҳ���ǳ���10msû�н��յ�
//�κ�����,���ʾ�˴ν������.
//���յ�������״̬
//[15]:0,û�н��յ�����;1,���յ���һ������.
//[14:0]:���յ������ݳ���
vu16 USART3_RX_STA=0;  
u16 usart2receiveIen = 0;
u16 usart2_RX = 0;
u8 usart2_startRx = 0;

u16 receivelen = 0;// �����������ݳ���
UartEvent event;//����һ���¼�����
u8 USART2_TX_BUF[USART3_MAX_SEND_LEN]; 	


//��������
u16 Uart_Send_Data(void* buffer, u16 size)
{
  if(!size) return 0;// �жϳ����Ƿ���Ч
  while (DMA_GetCurrDataCounter(DMA1_Channel2));// ���DMA����ͨ�����Ƿ�������
  if(buffer) memcpy(sendbuf, buffer,(size > 1024?1024:size));
  //DMA��������-Ҫ�ȹ� ���÷��ͳ��� ����DMA
  DMA_Cmd(DMA1_Channel2, DISABLE);
  DMA1_Channel2->CNDTR = size;// ���÷��ͳ���
  DMA_Cmd(DMA1_Channel2, ENABLE);  // ����DMA����
  return size;
}

void Uart_SendOne(u8 data)
{
	while (DMA_GetCurrDataCounter(DMA1_Channel2));// ���DMA����ͨ�����Ƿ�������
	memcpy(sendbuf, &data,1);
	//DMA��������-Ҫ�ȹ� ���÷��ͳ��� ����DMA
	DMA_Cmd(DMA1_Channel2, DISABLE);
	DMA1_Channel2->CNDTR = 1;// ���÷��ͳ���
	DMA_Cmd(DMA1_Channel2, ENABLE);  // ����DMA����	
}
 
//���DMA ���棬����ֹDMA
void Uart_Dma_Clr(void)
{
    DMA_Cmd(DMA1_Channel2, DISABLE);
    DMA1_Channel2->CNDTR=0;
    DMA_Cmd(DMA1_Channel3, DISABLE);
    DMA1_Channel3->CNDTR=SENDBUFF_SIZE ;
    DMA_Cmd(DMA1_Channel3, ENABLE);
}

// ��ȡһ���¼����¼���Ϊ��������¼��ͽ�������¼������Ը����¼����н��д���
UartEvent Uart_Get_Event(void)
{
  if(!DMA1_Channel3->CNDTR) Uart_Dma_Clr();// �������һ���¼��󣬽�������ͨ���Ѿ�û���˻���ռ䣬�������DMA���
  return event;
}

// �����Ӧ���¼�
void Uart_Clr_Event(UartEvent event_in)
{
    event&=~event_in;
}

void Uart_Set_Event(UartEvent event_in)
{
	event |= event_in;
}

//�жϴ������������ݷ�����ϣ����ڲ���һ����������ж�
void USART3_IRQHandler()
{
  u8 tem;
  if(USART_GetITStatus(USART3,USART_IT_IDLE)!= RESET)
  {
	  //printf("USART3_IRQHandler\n");
    tem = USART3->SR;//�ȶ�SR��Ȼ���DR�������
    tem = USART3->DR;
    tem = tem;
    Uart_Set_Event(E_uart_idle);
    receivelen = SENDBUFF_SIZE - DMA1_Channel3->CNDTR;// �ܵ�buf���ȼ�ȥʣ��buf���ȣ��õ����յ����ݵĳ���
	USART3_RX_STA|=1<<15;				              //ǿ�Ʊ�ǽ������
    USART_ClearITPendingBit(USART3, USART_IT_IDLE);
  } 
  if(USART_GetITStatus(USART3,USART_IT_TC)!= RESET) // ȫ�����ݷ�����ɣ������ñ��**
  {
	//printf("send over\n");
    USART_ClearITPendingBit(USART3, USART_IT_TC);   // �����ɱ��
    DMA_Cmd(DMA1_Channel2, DISABLE); // �ر�DMA
    DMA1_Channel2->CNDTR=0;          // ������ݳ���
    Uart_Set_Event(E_uart_tc);     //���÷�������¼�
  } 
}

 	
//void USART3_IRQHandler(void)
//{
//	int i = 0;
//	u8 res;	      
//	if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)//���յ�����
//	{	 
//		printf("USART_IT_RXNE\n");
//		USART_ClearITPendingBit(USART3, USART_IT_RXNE);
//		res =USART_ReceiveData(USART3);		 
//		if((USART3_RX_STA&(1<<15))==0)//�������һ������,��û�б�����,���ٽ�����������
//		{ 
//			if(USART3_RX_STA<USART3_MAX_RECV_LEN)	//�����Խ�������
//			{
//				USART3_RX_BUF[USART3_RX_STA++]=res;	//��¼���յ���ֵ	 
//			}else 
//			{
//				USART3_RX_STA|=1<<15;				//ǿ�Ʊ�ǽ������
//			} 
//		}
//	} 
//	if(USART_GetITStatus(USART3,USART_IT_IDLE)!= RESET)
//	{
//		printf("USART_IT_IDLE\n");
//		for(i = 0; i < 12; i++)
//			printf("USART3_RX_BUF[ %2d ] = %#x",i,USART3_RX_BUF[i]);
//		USART_ClearITPendingBit(USART3, USART_IT_IDLE);
//		USART3_RX_STA|=1<<15;
//	}
//}  

void USART2_IRQHandler()
{
	if(USART_GetITStatus(USART2,USART_IT_RXNE)!= RESET)
	{
		USART_ClearITPendingBit(USART2, USART_IT_RXNE);
		if(USART_ReceiveData(USART2) == 0x0f)
		{
			usart2_startRx = 1;
			return;
		}
		if(usart2_startRx)
		{
			if(usart2_RX >= 1024)
				return;
			usart2receivebuf[usart2_RX] = USART_ReceiveData(USART2);
			if(usart2receivebuf[usart2_RX] == 0xff || usart2receivebuf[usart2_RX] == 0xef || usart2receivebuf[usart2_RX] == 0xdf || usart2receivebuf[usart2_RX] == 0xcf)	
			{
				usart2receiveIen = usart2_RX;
				usart2_startRx = 0;
				usart2_RX = 0;
				return;
			}
			usart2_RX++;
		}
	}
}

void USART1_IRQHandler()
{
	uint8_t ucTemp;
	if(USART_GetITStatus(USART1,USART_IT_RXNE)!=RESET)
	{		
		ucTemp = USART_ReceiveData(USART1);
		USART_SendData(USART1,ucTemp);    
	}	 
}

u8 Uart_Receive_Data(u8 *recbuf, u16 *revLen)
{
	//printf("Uart_Receive_Data\n");
	if(*revLen >= 100)
	{
		*revLen = 0;
		return 0;
	}
    if( event & E_uart_idle) // �Ƿ���������ж�
    {
		//printf("Uart_Receive_Data2\n");
        memcpy(recbuf,receivebuf,*revLen);
		//printf("Uart_Receive_Data3\n");
        Uart_Clr_Event(E_uart_idle);
		//printf("Uart_Receive_Data4\n");
        Uart_Dma_Clr();
		//printf("Uart_Receive_Data5\n");
        return 1;
    }
    else
    {
        *revLen = 0;
        return 0;
    }
}

//��ʼ��IO ����3
//pclk1:PCLK1ʱ��Ƶ��(Mhz)
//bound:������	  
void usart_init(u32 bound)
{  

	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	// GPIOAʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE); //����2ʱ��ʹ��

 	USART_DeInit(USART3);  //��λ����3
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; //PA9  TX
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
	GPIO_Init(GPIOB, &GPIO_InitStructure); 
   

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11; // RX
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
	GPIO_Init(GPIOB, &GPIO_InitStructure);  
	
	USART_InitStructure.USART_BaudRate = bound;//����������
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ
  
	USART_Init(USART3, &USART_InitStructure); //��ʼ������3
  
	//ʹ�ܽ����ж�
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);//�����ж�   
	USART_ITConfig(USART3, USART_IT_IDLE, ENABLE);//�������ڿ����ж�
	
	
	//�����ж����ȼ�
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
	
	USART_Cmd(USART3, ENABLE);//ʹ�ܴ��� 
	
	USART3_RX_STA = 0;

}

void usart1_init(u32 bound)
{  

	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	// GPIOAʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE); //����2ʱ��ʹ��

 	USART_DeInit(USART1);  
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA9  TX
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
	GPIO_Init(GPIOA, &GPIO_InitStructure); 
   

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; // RX
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
	GPIO_Init(GPIOA, &GPIO_InitStructure);  
	
	USART_InitStructure.USART_BaudRate = bound;//����������
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ
  
	USART_Init(USART1, &USART_InitStructure); //��ʼ������3
  
	//ʹ�ܽ����ж�
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//�����ж�   
	
	//�����ж����ȼ�
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
	
	USART_Cmd(USART1, ENABLE);                    //ʹ�ܴ��� 

}

void _uart_dma_configuration()
{
  DMA_InitTypeDef DMA_InitStructure;
 
  /* DMA1 Channel6 (triggered by USART1 Rx event) Config */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1 ,
                        ENABLE);
 
  /* DMA1 Channel5 (triggered by USART1 Rx event) Config */
  DMA_DeInit(DMA1_Channel3);
  DMA_InitStructure.DMA_PeripheralBaseAddr = USART_DR_ADDRESS;// ��ʼ�������ַ���൱�ڡ��ļҿ�ݡ�  
  DMA_InitStructure.DMA_MemoryBaseAddr =(u32)receivebuf;// �ڴ��ַ���൱�ڼ��Ź�
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;//������Ϊ������Դ����Ϊ�տ��
  DMA_InitStructure.DMA_BufferSize = SENDBUFF_SIZE ;// ���������������Ӵ�С
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable; // �����ַ�������������Ӷ�Ӧ�Ŀ�ݲ���
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;// �ڴ����
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; //�����ֽڿ�ȣ��������������С�������������㣬���ǰ�����㣩 
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;// �ڴ��ֽڿ�ȣ���������װ��ݵĶ���(�����������ǰ����ʽ��з�װ)
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;// ����ģʽ�������˾Ͳ��ڽ����ˣ�������ѭ���洢
  DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;// ���ȼ��ܸߣ���Ӧ��ݾ��ǼӼ�
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable; // �ڴ�������ͨ�ţ������ڴ浽�ڴ� 
  DMA_Init(DMA1_Channel3, &DMA_InitStructure);// �Ѳ�����ʼ������������ݹ�˾��Э��
 
  DMA_Cmd(DMA1_Channel3, ENABLE);// ����DMA�������ݹ�˾ǩ����ͬ����ʽ��Ч
 
  /* DMA1 Channel4 (triggered by USART1 Tx event) Config */
  DMA_DeInit(DMA1_Channel2);
  DMA_InitStructure.DMA_PeripheralBaseAddr = USART_DR_ADDRESS;  // �����ַ������1�� �������Ŀ��
  DMA_InitStructure.DMA_MemoryBaseAddr =(u32)sendbuf;// �����ڴ��ַ
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;// ����Ϊ��������Ŀ�ĵأ����������ݣ�������Ƿ���
  DMA_InitStructure.DMA_BufferSize = 0;  //���ͳ���Ϊ0����δ�п����Ҫ����
  DMA_Init(DMA1_Channel2, &DMA_InitStructure);//��ʼ��
 
  USART_ITConfig(USART3, USART_IT_TC, ENABLE);// ʹ�ܴ��ڷ�������ж�
  USART_DMACmd(USART3, USART_DMAReq_Tx|USART_DMAReq_Rx, ENABLE);// ʹ��DMA���ڷ��ͺͽ�������
  
}

 


/*****************  ����һ���ֽ� **********************/
void Usart_SendByte( USART_TypeDef * pUSARTx, uint8_t ch)
{
	/* ����һ���ֽ����ݵ�USART */
	USART_SendData(pUSARTx,ch);
		
	/* �ȴ��������ݼĴ���Ϊ�� */
	while (USART_GetFlagStatus(pUSARTx, USART_FLAG_TXE) == RESET);	
}

//��ʼ��IO ����2
//pclk1:PCLK1ʱ��Ƶ��(Mhz)
//bound:������	  
void USART2_Config(u32 bound)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	// �򿪴���GPIO��ʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	// �򿪴��������ʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

	// ��USART Tx��GPIO����Ϊ���츴��ģʽ
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

  // ��USART Rx��GPIO����Ϊ��������ģʽ
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	// ���ô��ڵĹ�������
	// ���ò�����
	USART_InitStructure.USART_BaudRate = bound;
	// ���� �������ֳ�
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	// ����ֹͣλ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	// ����У��λ
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	// ����Ӳ��������
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	// ���ù���ģʽ���շ�һ��
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	// ��ɴ��ڵĳ�ʼ������
	USART_Init(USART2, &USART_InitStructure);
	

	/* Ƕ�������жϿ�������ѡ�� */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

	/* ����USARTΪ�ж�Դ */
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	/* �������ȼ�*/
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	/* �����ȼ� */
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	/* ʹ���ж� */
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	/* ��ʼ������NVIC */
	NVIC_Init(&NVIC_InitStructure);
	
	// ʹ�ܴ��ڽ����ж�
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);	
	
	// ʹ�ܴ���
	USART_Cmd(USART2, ENABLE);		

  // ���������ɱ�־
	//USART_ClearFlag(USART1, USART_FLAG_TC);     
}



//����2,printf ����
//ȷ��һ�η������ݲ�����USART3_MAX_SEND_LEN�ֽ�
///�ض���c�⺯��printf�����ڣ��ض�����ʹ��printf����
//int fputc(int ch, FILE *f)
//{
//		/* ����һ���ֽ����ݵ����� */
//	USART_SendData(USART1, (uint8_t) ch);
//	
//	/* �ȴ�������� */
//	while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);		

//	return (ch);
//}

void Usart_SendString( USART_TypeDef * pUSARTx, char *str)
{
	unsigned int k=0;
  do 
  {
      Usart_SendByte( pUSARTx, *(str + k) );
      k++;
  } while(*(str + k)!='\0');
  
  /* �ȴ�������� */
  while(USART_GetFlagStatus(pUSARTx,USART_FLAG_TC)==RESET)
  {}
}

 



















