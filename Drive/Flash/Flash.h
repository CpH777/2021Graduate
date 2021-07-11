#ifndef __FLASH_H
#define __FLASH_H

#include "stm32f10x.h"

#define DEBUG
#define FLASH_SIZE 64                   /* ��ѡMCU��FLASH������С(��λΪK) */

/* STM32��������Ʒÿҳ��С2KByte���С�С������Ʒÿҳ��С1KByte */
#if defined (STM32F10X_HD) || defined (STM32F10X_HD_VL) || defined (STM32F10X_CL) || defined (STM32F10X_XL)
  #define FLASH_PAGE_SIZE    ((uint16_t)0x800)	//2048
#else
  #define FLASH_PAGE_SIZE    ((uint16_t)0x400)	//1024
#endif

//д�����ʼ��ַ�������ַ
#define WRITE_START_ADDR  ((uint32_t)0x08000000)
#define WRITE_END_ADDR    ((uint32_t)0x0800FFFF)

typedef struct people
{
	u8 upTime[7][5];
	u8 offTime[7][5];
	u8 IDNUM;
	u8 index;
}people;


typedef enum 
{
	FAILED = 0, 
  PASSED = !FAILED
} TestStatus;

extern people staff[30];
extern u16	staff_num;                 /* ���� */

void FLASH_WriteData( uint32_t startAddress, uint16_t *writeData, uint16_t countToWrite );
u16 STMFLASH_ReadHalfWord( u32 faddr );
void FLASH_ReadMoreData( u32 ReadAddr, u16 *pBuffer, u16 NumToRead );
void writeTime(u16 IdNum);
void read_from_flash(u16 IdNum,u16 *temp);
void initstaff(people *staff);
void writeIdNum(void);
void readIdNum(u16 *temp);


#endif

