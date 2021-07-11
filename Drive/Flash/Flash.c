#include "Flash.h"
#include "stm32f10x_flash.h"
#include "string.h"
#include "Timer.h"
#include "AS608.h"
#include "timepoiling.h"

extern struct rtc_time systmtime;
people staff[30];
u16	staff_num = 0;                 /* 人数 */


FLASH_Status Debug;
/* Flash 写函数 以8 bit 方式写入 */
void FLASH_WriteData( uint32_t startAddress, uint16_t *writeData, uint16_t countToWrite )
{
	uint16_t	i;
	uint32_t	offsetAddress;  /* 偏移地址 */
	uint32_t	sectorPosition; /* 扇区位置 */
	uint32_t	sectorStartAddress;
	if ( startAddress < FLASH_BASE || ( (startAddress + countToWrite * 2) >= (FLASH_BASE + 1024 * FLASH_SIZE) ) )
	{
		return;                 /* 非法地址 */
	}
	/* 解锁写保护 */
	FLASH_Unlock();

	/* 计算去掉0X08000000后的实际偏移地址 */
	offsetAddress = startAddress - FLASH_BASE;
	/* 计算扇区地址 */
	sectorPosition = offsetAddress / FLASH_PAGE_SIZE;
	/* 对应扇区的首地址 */
	sectorStartAddress = sectorPosition * FLASH_PAGE_SIZE + FLASH_BASE;

	FLASH_ClearFlag( FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR );

	/* 擦除这个扇区 */
	Debug = FLASH_ErasePage( sectorStartAddress );
#ifdef DEBUG
	if ( Debug != FLASH_COMPLETE )
		//printf( "扇区擦除失败\n\n" );
#endif
	for ( i = 0; i < countToWrite; i++ )
	{
		Debug = FLASH_ProgramHalfWord( startAddress, writeData[i] );
#ifdef DEBUG
		if ( Debug != FLASH_COMPLETE )
			//printf( "扇区写失败\n\n" );
#endif
		startAddress = startAddress + 2;
	}
	FLASH_Lock(); /*上锁写保护 */
}

//从指定地址开始写入多个数据
void FLASH_WriteMoreData(uint32_t startAddress,uint16_t *writeData,uint16_t countToWrite)
{
	uint16_t dataIndex;
	uint32_t	offsetAddress;  /* 偏移地址 */
	uint32_t	sectorPosition; /* 扇区位置 */
	uint32_t	sectorStartAddress;
  if(startAddress<FLASH_BASE||((startAddress+countToWrite*2)>=(FLASH_BASE+1024*FLASH_SIZE)))
  {
    return;//非法地址
  }
  FLASH_Unlock();         //解锁写保护
   offsetAddress=startAddress-FLASH_BASE;               //计算去掉0X08000000后的实际偏移地址
   sectorPosition=offsetAddress/FLASH_PAGE_SIZE;            //计算扇区地址，对于STM32F103VET6为0~255
  
   sectorStartAddress=sectorPosition*FLASH_PAGE_SIZE+FLASH_BASE;    //对应扇区的首地址

  FLASH_ErasePage(sectorStartAddress);//擦除这个扇区
  
  
  for(dataIndex=0;dataIndex<countToWrite;dataIndex++)
  {
    FLASH_ProgramHalfWord(startAddress+dataIndex*2,writeData[dataIndex]);
  }
  
  FLASH_Lock();//上锁写保护
}




uint16_t FLASH_ReadHalfWord(uint32_t address)
{
  return *(__IO uint16_t*)address; 
}


/* 16位数据读取 */
void FLASH_ReadMoreData( u32 ReadAddr, u16 *pBuffer, u16 NumToRead )
{
	u16 i;
	for ( i = 0; i < NumToRead; i++ )
	{
		pBuffer[i]	= FLASH_ReadHalfWord( ReadAddr );    /* 读取2个字节. */
		ReadAddr	+= 2;                                   /* 偏移2个字节. */
	}
}




//读取指定地址的全字(32位数据)
uint32_t FLASH_ReadWord(uint32_t address)
{
  uint32_t temp1,temp2;
  temp1=*(__IO uint16_t*)address; 
  temp2=*(__IO uint16_t*)(address+2); 
  return (temp2<<16)+temp1;
}



void writeTime(u16 IdNum)
{
	u16 temp[72] = {0};
	int index = 0,tempIndex = 2,i,j;
	if(staff[IdNum].index == 0)
	{
		staff[IdNum].index = systmtime.tm_mday;
	}
	if(systmtime.tm_mday > staff[IdNum].index)
	{
		index = systmtime.tm_mday - staff[IdNum].index;
	}
	else if(systmtime.tm_mday < staff[IdNum].index)
	{
		index = (countMonthDay(systmtime.tm_mon - 1) + systmtime.tm_mday) - staff[IdNum].index;
	}
	if(index >= 7)
	{
		staff[IdNum].index = 0;
		return;
	}
	staff[IdNum].IDNUM = IdNum;
	if(systmtime.tm_hour > 7 && systmtime.tm_hour < 16)
	{
		staff[IdNum].upTime[index][0] = systmtime.tm_year-2000;
		staff[IdNum].upTime[index][1] = systmtime.tm_mon;
		staff[IdNum].upTime[index][2] = systmtime.tm_mday;
		staff[IdNum].upTime[index][3] = systmtime.tm_hour;
		staff[IdNum].upTime[index][4] = systmtime.tm_min;
	}
	else
	{
		staff[IdNum].offTime[index][0] = systmtime.tm_year-2000;
		staff[IdNum].offTime[index][1] = systmtime.tm_mon;
		staff[IdNum].offTime[index][2] = systmtime.tm_mday;
		staff[IdNum].offTime[index][3] = systmtime.tm_hour;
		staff[IdNum].offTime[index][4] = systmtime.tm_min;
	}
	temp[0] = staff[IdNum].IDNUM;
	temp[1] = staff[IdNum].index;
	for(i = 0; i < 7; i++)
		for(j = 0; j < 5; j++)
			temp[tempIndex++] = staff[IdNum].upTime[i][j];
	
	for(i = 0; i < 7; i++)
		for(j = 0; j < 5; j++)
			temp[tempIndex++] = staff[IdNum].offTime[i][j];
	
//	printf("write\n");
//	for(i = 0; i < 72; i++)
//		printf("temp[%d] = %d\n",i,temp[i]);
	
	FLASH_WriteMoreData(WRITE_START_ADDR + (IdNum+23) * FLASH_PAGE_SIZE,temp,72);
	
//	FLASH_ReadMoreData(WRITE_START_ADDR + (IdNum+23) * FLASH_PAGE_SIZE, temp, 72);
//	
//	printf("read\n");
//	for(i = 0; i < 72; i++)
//		printf("temp[%d] = %d\n",i,temp[i]);
}

void writeIdNum()
{
	u16 temp[30];
	int i = 0;
//	printf("Save\n");
//	printf("WRITE_START_ADDR + 54 * FLASH_PAGE_SIZE = %#x\n",WRITE_START_ADDR + 54 * FLASH_PAGE_SIZE);
	for(i = 0; i < 30; i++)
		temp[i] = idNum[i];
	
//	for(i = 0; i < 30; i++)
//		printf("temp[%d] = %d\n",i,temp[i]);
	
	FLASH_WriteMoreData(WRITE_START_ADDR + 54 * FLASH_PAGE_SIZE,temp,30);  //规定60页存放idNum
	
//	readIdNum(temp);
	
//	for(i = 0; i < 30; i++)
//		printf("temp[%d] = %d\n",i,temp[i]);
}

void readIdNum(u16 *temp)
{
	int i;
//	printf("Read\n");
//	printf("WRITE_START_ADDR + 54 * FLASH_PAGE_SIZE = %#x\n",WRITE_START_ADDR + 54 * FLASH_PAGE_SIZE);
	FLASH_ReadMoreData( WRITE_START_ADDR + 54 * FLASH_PAGE_SIZE, temp, 30);
	for(i = 0; i < 30; i++)
		idNum[i] = temp[i];	
//	for(i = 0; i < 30; i++)
//		printf("idNum[%d] = %d\n",i,idNum[i]);
}


void read_from_flash(u16 IdNum,u16 *temp)
{
	FLASH_ReadMoreData( WRITE_START_ADDR + (IdNum+23) * FLASH_PAGE_SIZE, temp, 72);
	//发送
}

void initstaff(people *staff)
{
	int i = 0,j = 0;
	for(i = 0; i < 7; i++)
		for(j = 0; j < 5; j++)
		{
			staff->upTime[i][j] = 0;
			staff->offTime[i][j] = 0;
		}
	staff->index = 0;
}





