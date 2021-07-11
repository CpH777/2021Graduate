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
//串口接收缓存区 	
u8 USART3_RX_BUF[USART3_MAX_RECV_LEN]; 				//接收缓冲,最大USART3_MAX_RECV_LEN个字节.
u8 USART3_TX_BUF[USART3_MAX_SEND_LEN]; 			  //发送缓冲,最大USART3_MAX_SEND_LEN字节

//通过判断接收连续2个字符之间的时间差不大于10ms来决定是不是一次连续的数据.
//如果2个字符接收间隔超过10ms,则认为不是1次连续数据.也就是超过10ms没有接收到
//任何数据,则表示此次接收完毕.
//接收到的数据状态
//[15]:0,没有接收到数据;1,接收到了一批数据.
//[14:0]:接收到的数据长度
vu16 USART3_RX_STA=0;  
u16 usart2receiveIen = 0;
u16 usart2_RX = 0;
u8 usart2_startRx = 0;

u16 receivelen = 0;// 声明接收数据长度
UartEvent event;//申明一个事件参数
u8 USART2_TX_BUF[USART3_MAX_SEND_LEN]; 	


//发送数据
u16 Uart_Send_Data(void* buffer, u16 size)
{
  if(!size) return 0;// 判断长度是否有效
  while (DMA_GetCurrDataCounter(DMA1_Channel2));// 检查DMA发送通道内是否还有数据
  if(buffer) memcpy(sendbuf, buffer,(size > 1024?1024:size));
  //DMA发送数据-要先关 设置发送长度 开启DMA
  DMA_Cmd(DMA1_Channel2, DISABLE);
  DMA1_Channel2->CNDTR = size;// 设置发送长度
  DMA_Cmd(DMA1_Channel2, ENABLE);  // 启动DMA发送
  return size;
}

void Uart_SendOne(u8 data)
{
	while (DMA_GetCurrDataCounter(DMA1_Channel2));// 检查DMA发送通道内是否还有数据
	memcpy(sendbuf, &data,1);
	//DMA发送数据-要先关 设置发送长度 开启DMA
	DMA_Cmd(DMA1_Channel2, DISABLE);
	DMA1_Channel2->CNDTR = 1;// 设置发送长度
	DMA_Cmd(DMA1_Channel2, ENABLE);  // 启动DMA发送	
}
 
//清除DMA 缓存，并终止DMA
void Uart_Dma_Clr(void)
{
    DMA_Cmd(DMA1_Channel2, DISABLE);
    DMA1_Channel2->CNDTR=0;
    DMA_Cmd(DMA1_Channel3, DISABLE);
    DMA1_Channel3->CNDTR=SENDBUFF_SIZE ;
    DMA_Cmd(DMA1_Channel3, ENABLE);
}

// 获取一个事件，事件分为发送完成事件和接收完成事件，可以根据事件进行进行处理
UartEvent Uart_Get_Event(void)
{
  if(!DMA1_Channel3->CNDTR) Uart_Dma_Clr();// 如果产生一个事件后，接收数据通道已经没有了缓存空间，进行清除DMA清空
  return event;
}

// 清除对应的事件
void Uart_Clr_Event(UartEvent event_in)
{
    event&=~event_in;
}

void Uart_Set_Event(UartEvent event_in)
{
	event |= event_in;
}

//中断处理，当所有数据发送完毕，串口产生一个发送完成中断
void USART3_IRQHandler()
{
  u8 tem;
  if(USART_GetITStatus(USART3,USART_IT_IDLE)!= RESET)
  {
	  //printf("USART3_IRQHandler\n");
    tem = USART3->SR;//先读SR，然后读DR才能清除
    tem = USART3->DR;
    tem = tem;
    Uart_Set_Event(E_uart_idle);
    receivelen = SENDBUFF_SIZE - DMA1_Channel3->CNDTR;// 总的buf长度减去剩余buf长度，得到接收到数据的长度
	USART3_RX_STA|=1<<15;				              //强制标记接收完成
    USART_ClearITPendingBit(USART3, USART_IT_IDLE);
  } 
  if(USART_GetITStatus(USART3,USART_IT_TC)!= RESET) // 全部数据发送完成，产生该标记**
  {
	//printf("send over\n");
    USART_ClearITPendingBit(USART3, USART_IT_TC);   // 清除完成标记
    DMA_Cmd(DMA1_Channel2, DISABLE); // 关闭DMA
    DMA1_Channel2->CNDTR=0;          // 清除数据长度
    Uart_Set_Event(E_uart_tc);     //设置发送完成事件
  } 
}

 	
//void USART3_IRQHandler(void)
//{
//	int i = 0;
//	u8 res;	      
//	if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)//接收到数据
//	{	 
//		printf("USART_IT_RXNE\n");
//		USART_ClearITPendingBit(USART3, USART_IT_RXNE);
//		res =USART_ReceiveData(USART3);		 
//		if((USART3_RX_STA&(1<<15))==0)//接收完的一批数据,还没有被处理,则不再接收其他数据
//		{ 
//			if(USART3_RX_STA<USART3_MAX_RECV_LEN)	//还可以接收数据
//			{
//				USART3_RX_BUF[USART3_RX_STA++]=res;	//记录接收到的值	 
//			}else 
//			{
//				USART3_RX_STA|=1<<15;				//强制标记接收完成
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
    if( event & E_uart_idle) // 是否产生空闲中断
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

//初始化IO 串口3
//pclk1:PCLK1时钟频率(Mhz)
//bound:波特率	  
void usart_init(u32 bound)
{  

	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	// GPIOA时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE); //串口2时钟使能

 	USART_DeInit(USART3);  //复位串口3
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; //PA9  TX
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
	GPIO_Init(GPIOB, &GPIO_InitStructure); 
   

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11; // RX
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
	GPIO_Init(GPIOB, &GPIO_InitStructure);  
	
	USART_InitStructure.USART_BaudRate = bound;//波特率设置
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
  
	USART_Init(USART3, &USART_InitStructure); //初始化串口3
  
	//使能接收中断
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);//开启中断   
	USART_ITConfig(USART3, USART_IT_IDLE, ENABLE);//开启串口空闲中断
	
	
	//设置中断优先级
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
	
	USART_Cmd(USART3, ENABLE);//使能串口 
	
	USART3_RX_STA = 0;

}

void usart1_init(u32 bound)
{  

	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	// GPIOA时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE); //串口2时钟使能

 	USART_DeInit(USART1);  
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA9  TX
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
	GPIO_Init(GPIOA, &GPIO_InitStructure); 
   

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; // RX
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
	GPIO_Init(GPIOA, &GPIO_InitStructure);  
	
	USART_InitStructure.USART_BaudRate = bound;//波特率设置
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
  
	USART_Init(USART1, &USART_InitStructure); //初始化串口3
  
	//使能接收中断
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//开启中断   
	
	//设置中断优先级
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
	
	USART_Cmd(USART1, ENABLE);                    //使能串口 

}

void _uart_dma_configuration()
{
  DMA_InitTypeDef DMA_InitStructure;
 
  /* DMA1 Channel6 (triggered by USART1 Rx event) Config */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1 ,
                        ENABLE);
 
  /* DMA1 Channel5 (triggered by USART1 Rx event) Config */
  DMA_DeInit(DMA1_Channel3);
  DMA_InitStructure.DMA_PeripheralBaseAddr = USART_DR_ADDRESS;// 初始化外设地址，相当于“哪家快递”  
  DMA_InitStructure.DMA_MemoryBaseAddr =(u32)receivebuf;// 内存地址，相当于几号柜
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;//外设作为数据来源，即为收快递
  DMA_InitStructure.DMA_BufferSize = SENDBUFF_SIZE ;// 缓存容量，即柜子大小
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable; // 外设地址不递增，即柜子对应的快递不变
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;// 内存递增
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; //外设字节宽度，即快递运输快件大小度量（按重量算，还是按体积算） 
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;// 内存字节宽度，即店主封装快递的度量(按重量，还是按体质进行封装)
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;// 正常模式，即满了就不在接收了，而不是循环存储
  DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;// 优先级很高，对应快递就是加急
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable; // 内存与外设通信，而非内存到内存 
  DMA_Init(DMA1_Channel3, &DMA_InitStructure);// 把参数初始化，即拟好与快递公司的协议
 
  DMA_Cmd(DMA1_Channel3, ENABLE);// 启动DMA，即与快递公司签订合同，正式生效
 
  /* DMA1 Channel4 (triggered by USART1 Tx event) Config */
  DMA_DeInit(DMA1_Channel2);
  DMA_InitStructure.DMA_PeripheralBaseAddr = USART_DR_ADDRESS;  // 外设地址，串口1， 即发件的快递
  DMA_InitStructure.DMA_MemoryBaseAddr =(u32)sendbuf;// 发送内存地址
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;// 外设为传送数据目的地，即发送数据，即快递是发件
  DMA_InitStructure.DMA_BufferSize = 0;  //发送长度为0，即未有快递需要发送
  DMA_Init(DMA1_Channel2, &DMA_InitStructure);//初始化
 
  USART_ITConfig(USART3, USART_IT_TC, ENABLE);// 使能串口发送完成中断
  USART_DMACmd(USART3, USART_DMAReq_Tx|USART_DMAReq_Rx, ENABLE);// 使能DMA串口发送和接受请求
  
}

 


/*****************  发送一个字节 **********************/
void Usart_SendByte( USART_TypeDef * pUSARTx, uint8_t ch)
{
	/* 发送一个字节数据到USART */
	USART_SendData(pUSARTx,ch);
		
	/* 等待发送数据寄存器为空 */
	while (USART_GetFlagStatus(pUSARTx, USART_FLAG_TXE) == RESET);	
}

//初始化IO 串口2
//pclk1:PCLK1时钟频率(Mhz)
//bound:波特率	  
void USART2_Config(u32 bound)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	// 打开串口GPIO的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	// 打开串口外设的时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

	// 将USART Tx的GPIO配置为推挽复用模式
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

  // 将USART Rx的GPIO配置为浮空输入模式
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	// 配置串口的工作参数
	// 配置波特率
	USART_InitStructure.USART_BaudRate = bound;
	// 配置 针数据字长
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	// 配置停止位
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	// 配置校验位
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	// 配置硬件流控制
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	// 配置工作模式，收发一起
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	// 完成串口的初始化配置
	USART_Init(USART2, &USART_InitStructure);
	

	/* 嵌套向量中断控制器组选择 */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

	/* 配置USART为中断源 */
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	/* 抢断优先级*/
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	/* 子优先级 */
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	/* 使能中断 */
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	/* 初始化配置NVIC */
	NVIC_Init(&NVIC_InitStructure);
	
	// 使能串口接收中断
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);	
	
	// 使能串口
	USART_Cmd(USART2, ENABLE);		

  // 清除发送完成标志
	//USART_ClearFlag(USART1, USART_FLAG_TC);     
}



//串口2,printf 函数
//确保一次发送数据不超过USART3_MAX_SEND_LEN字节
///重定向c库函数printf到串口，重定向后可使用printf函数
//int fputc(int ch, FILE *f)
//{
//		/* 发送一个字节数据到串口 */
//	USART_SendData(USART1, (uint8_t) ch);
//	
//	/* 等待发送完毕 */
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
  
  /* 等待发送完成 */
  while(USART_GetFlagStatus(pUSARTx,USART_FLAG_TC)==RESET)
  {}
}

 



















