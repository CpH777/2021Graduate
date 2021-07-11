#ifndef __USART_H
#define __USART_H


#include "GPIOLIKE51.h"
#include "stm32f10x.h"

#define USART3_MAX_RECV_LEN		400					//最大接收缓存字节数
#define USART3_MAX_SEND_LEN		400					//最大发送缓存字节数
#define USART3_RX_EN 			1					//0,不接收;1,接收.

// 串口对应的DMA请求通道
#define  USART_TX_DMA_CHANNEL     DMA1_Channel2
// 外设寄存器地址
#define  USART_DR_ADDRESS        (USART3_BASE+0x04)
// 一次发送的数据量
#define  SENDBUFF_SIZE            5000

extern u8  USART3_RX_BUF[USART3_MAX_RECV_LEN]; 		//接收缓冲,最大USART3_MAX_RECV_LEN字节
extern u8  USART3_TX_BUF[USART3_MAX_SEND_LEN]; 		//发送缓冲,最大USART3_MAX_SEND_LEN字节
extern vu16 USART3_RX_STA;   						//接收数据状态


extern u8 sendbuf[1024];
extern u8 receivebuf[1024]; 

//中断处理
//中断处理相关准备工作
typedef enum _UartEvent_
{
   E_uart_0 = 0,// 没有事件
   E_uart_tc=0x40,//发送完成
   E_uart_idle=0x80,//接收完成
}UartEvent;



void usart_init(u32 bound);				//串口3初始化 
void _uart_dma_configuration(void);
void Usart_SendByte( USART_TypeDef * pUSARTx, uint8_t ch);
u8 Uart_Receive_Data(u8 *recbuf, u16 *revLen);
void Uart_Set_Event(UartEvent event_in);
void Uart_Clr_Event(UartEvent event_in);
UartEvent Uart_Get_Event(void);
void Uart_Dma_Clr(void);
u16 Uart_Send_Data(void* buffer, u16 size);
void Uart_SendOne(u8 data);
void USART2_Config(u32 bound);
void Usart_SendString( USART_TypeDef * pUSARTx, char *str);
void colsebtNVIC(void);
void openbtNVIC(void);
void usart1_init(u32 bound);

#endif

