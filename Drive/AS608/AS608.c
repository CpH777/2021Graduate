#include "AS608.h"
#include "Usart.h"
#include "Delay.h"
#include "Key.h"
#include "Flash.h"
#include <string.h>
#include "flash.h"

extern u16 receivelen;// �����������ݳ���
extern void OLED_ShowCH(u8 x, u8 y,u8 *chs);
extern void OLED_Clear(unsigned dat); 
extern void OLED_ShowNum(u8 x,u8 y,u32 num,u8 len,u8 size2);

u32 AS608Addr = 0XFFFFFFFF; //Ĭ��
u16 idNum[30];
int newIdNum = -1;
u8 judgeRecvi[100];
unsigned char *JudegData = NULL;

static void emptyJudgeRecvi()
{
	int i = 0;
	for(i = 0; i < 100; i++)
		judgeRecvi[i] = 0;	
}


void initIDNum()
{
	int i = 0;
	readIdNum(idNum);
	if(idNum[0] >= 255)
	{
		for(i = 0; i < 30; i++)
			idNum[i] = 0;
		writeIdNum();
	}
	
}


int findIDNum()
{
	int i = 0;
	for(i = 0; i < 30; i++)
	{
		if(idNum[i] == 0)
			return i;
	}
	return -1;
}

void emptyIDNum()
{
	int i = 0;
	for(i = 0; i < 100; i++)
		idNum[i] = 0;	
}

//���Ͱ�ͷ
static void SendHead(void)
{
  Uart_SendOne(0xEF);
  Uart_SendOne(0x01);
}

static void SendAddr(void)
{
  Uart_SendOne(AS608Addr >> 24);
  Uart_SendOne(AS608Addr >> 16);
  Uart_SendOne(AS608Addr >> 8);
  Uart_SendOne(AS608Addr);
}
//���Ͱ���ʶ,
static void SendFlag(u8 flag)
{
  Uart_SendOne(flag);
}
//���Ͱ�����
static void SendLength(int length)
{
  Uart_SendOne(length >> 8);
  Uart_SendOne(length);
}
//����ָ����
static void Sendcmd(u8 cmd)
{
  Uart_SendOne(cmd);
}
//����У���
static void SendCheck(u16 check)
{
  Uart_SendOne(check >> 8);
  Uart_SendOne(check);
}
//�ж��жϽ��յ�������û��Ӧ���
//waittimeΪ�ȴ��жϽ������ݵ�ʱ�䣨��λ1ms��
//����ֵ�����ݰ��׵�ַ
//static u8 *JudgeStr(u16 waittime)
//{
//	int i = 0;
//	//char *data;
//	//u8 temp[100];
//	u8 str[8];
//	str[0] = 0xef;
//	str[1] = 0x01;
//	str[2] = AS608Addr >> 24;
//	str[3] = AS608Addr >> 16;
//	str[4] = AS608Addr >> 8;
//	str[5] = AS608Addr;
//	str[6] = 0x07;
//	str[7] = '\0';
//	JudegData = NULL;
//	emptyJudgeRecvi();
//	printf("JudgeStr\n");
//	while(--waittime)
//	{
//		delay_ms(1);
//		if(Uart_Receive_Data(judgeRecvi,&receivelen)) //���յ�һ������
//		{
//			printf("JudgeStr2\n");
//			//JudegData = NULL;
//			JudegData = strstr((const char*)judgeRecvi, (const char*)str);
//			printf("JudgeStr3\n");
//			for(i = 0; i < receivelen; i++)
//			{
//				printf("JudegData[ %2d ] = %#x\n ",i,JudegData[i]);
//			}
//			if(JudegData)
//			{
//				printf("JudegData is Not NULL\n");
//				return (u8*)JudegData;
//				//printf("JudegData is Not NULL\n");
//			}
//				
//		}
//	}
//	return 0;
//}
static u8 *JudgeStr(u16 waittime)
{
	int i = 0;
  char *data;
  u8 str[8];
  str[0] = 0xef;
  str[1] = 0x01;
  str[2] = AS608Addr >> 24;
  str[3] = AS608Addr >> 16;
  str[4] = AS608Addr >> 8;
  str[5] = AS608Addr;
  str[6] = 0x07;
  str[7] = '\0';
  USART3_RX_STA = 0;
  while(--waittime)
  {
    delay_ms(1);
    if(USART3_RX_STA & 0X8000) //���յ�һ������
    {
		if(receivelen<USART3_MAX_RECV_LEN)	//�����Խ�������
		{
			memcpy(USART3_RX_BUF,receivebuf,receivelen);
			Uart_Dma_Clr();	
		}
        USART3_RX_STA = 0;
//				for(i = 0; i < 12; i++)
//			{
//				printf("USART3_RX_BUF[ %2d ] = %#x\n ",i,USART3_RX_BUF[i]);
//			}
      data = strstr((const char*)USART3_RX_BUF, (const char*)str);
			for(i = 0; i < 12; i++)
//			{
//				printf("data[ %2d ] = %#x\n ",i,data[i]);
//			}
      if(data)
        return (u8*)data;
    }
  }
  return 0;
}
//¼��ͼ�� PS_GetImage
//����:̽����ָ��̽�⵽��¼��ָ��ͼ�����ImageBuffer��
//ģ�鷵��ȷ����
u8 PS_GetImage(void)
{
	
//	int i = 0;
	u16 temp;
	u8  ensure;
	u8  *data = NULL;
	SendHead(); //EF 01
	SendAddr(); //FF FF FF FF
	SendFlag(0x01);//�������ʶ
	SendLength(0x03);
	Sendcmd(0x01);
	temp =  0x01 + 0x03 + 0x01;
	SendCheck(temp);
//	printf("PS_GetImage\n");
	data = JudgeStr(1000);
//	printf("PS_GetImage2\n");
//	for(i = 0; i < 12; i++)
//	{
//		if(data[i])
//			printf("data[%d] = %#x\n",i,data[i]);
//	}
	if(data)
		ensure = data[9];
	else
		ensure = 0xff;
	return ensure;
}
//�������� PS_GenChar
//����:��ImageBuffer�е�ԭʼͼ������ָ�������ļ�����CharBuffer1��CharBuffer2
//����:BufferID --> charBuffer1:0x01	charBuffer1:0x02
//ģ�鷵��ȷ����
u8 PS_GenChar(u8 BufferID)
{
	u16 temp;
	u8  ensure;
	u8  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);//�������ʶ
	SendLength(0x04);
	Sendcmd(0x02);
	Uart_SendOne(BufferID);
	temp = 0x01 + 0x04 + 0x02 + BufferID;
	SendCheck(temp);
	data = JudgeStr(2000);
	if(data)
		ensure = data[9];
	else
		ensure = 0xff;
	return ensure;
}

//��ȷ�ȶ���öָ������ PS_Match
//����:��ȷ�ȶ�CharBuffer1 ��CharBuffer2 �е������ļ�
//ģ�鷵��ȷ����
u8 PS_Match(void)
{
	u16 temp;
	u8  ensure;
	u8  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);//�������ʶ
	SendLength(0x03);
	Sendcmd(0x03);
	temp = 0x01 + 0x03 + 0x03;
	SendCheck(temp);
	data = JudgeStr(2000);
	if(data)
		ensure = data[9];
	else
		ensure = 0xff;
	return ensure;
}
//����ָ�� PS_Search
//����:��CharBuffer1��CharBuffer2�е������ļ����������򲿷�ָ�ƿ�.�����������򷵻�ҳ�롣
//����:  BufferID @ref CharBuffer1	CharBuffer2
//˵��:  ģ�鷵��ȷ���֣�ҳ�루����ָ��ģ�壩
u8 PS_Search(u8 BufferID, u16 StartPage, u16 PageNum, SearchResult *p)
{
	u16 temp;
	u8  ensure;
	u8  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);//�������ʶ
	SendLength(0x08);
	Sendcmd(0x04);
	Uart_SendOne(BufferID);
	Uart_SendOne(StartPage >> 8);
	Uart_SendOne(StartPage);
	Uart_SendOne(PageNum >> 8);
	Uart_SendOne(PageNum);
	temp = 0x01 + 0x08 + 0x04 + BufferID
		 + (StartPage >> 8) + (u8)StartPage
		 + (PageNum >> 8) + (u8)PageNum;
	SendCheck(temp);
	data = JudgeStr(2000);
	if(data)
	{
		ensure = data[9];
		p->pageID   = (data[10] << 8) + data[11];
		p->mathscore = (data[12] << 8) + data[13];
	}
	else
		ensure = 0xff;
	return ensure;
}
//�ϲ�����������ģ�壩PS_RegModel
//����:��CharBuffer1��CharBuffer2�е������ļ��ϲ����� ģ��,�������CharBuffer1��CharBuffer2
//˵��:  ģ�鷵��ȷ����
u8 PS_RegModel(void)
{
	u16 temp;
	u8  ensure;
	u8  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);//�������ʶ
	SendLength(0x03);
	Sendcmd(0x05);
	temp = 0x01 + 0x03 + 0x05;
	SendCheck(temp);
	data = JudgeStr(2000);
	if(data)
		ensure = data[9];
	else
		ensure = 0xff;
	return ensure;
}
//����ģ�� PS_StoreChar
//����:�� CharBuffer1 �� CharBuffer2 �е�ģ���ļ��浽 PageID ��flash���ݿ�λ�á�
//����:  BufferID @ref charBuffer1:0x01	charBuffer1:0x02
//       PageID��ָ�ƿ�λ�úţ�
//˵��:  ģ�鷵��ȷ����
u8 PS_StoreChar(u8 BufferID, u16 PageID)
{
	u16 temp;
	u8  ensure;
	u8  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);//�������ʶ
	SendLength(0x06);
	Sendcmd(0x06);
	Uart_SendOne(BufferID);
	Uart_SendOne(PageID >> 8);
	Uart_SendOne(PageID);
	temp = 0x01 + 0x06 + 0x06 + BufferID
		 + (PageID >> 8) + (u8)PageID;
	SendCheck(temp);
	data = JudgeStr(2000);
	if(data)
		ensure = data[9];
	else
		ensure = 0xff;
	return ensure;
}
//ɾ��ģ�� PS_DeletChar
//����:  ɾ��flash���ݿ���ָ��ID�ſ�ʼ��N��ָ��ģ��
//����:  PageID(ָ�ƿ�ģ���)��Nɾ����ģ�������
//˵��:  ģ�鷵��ȷ����
u8 PS_DeletChar(u16 PageID, u16 N)
{
	u16 temp;
	u8  ensure;
	u8  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);//�������ʶ
	SendLength(0x07);
	Sendcmd(0x0C);
	Uart_SendOne(PageID >> 8);
	Uart_SendOne(PageID);
	Uart_SendOne(N >> 8);
	Uart_SendOne(N);
	temp = 0x01 + 0x07 + 0x0C
		 + (PageID >> 8) + (u8)PageID
		 + (N >> 8) + (u8)N;
	SendCheck(temp);
	data = JudgeStr(2000);
	if(data)
		ensure = data[9];
	else
		ensure = 0xff;
	return ensure;
}
//���ָ�ƿ� PS_Empty
//����:  ɾ��flash���ݿ�������ָ��ģ��
//����:  ��
//˵��:  ģ�鷵��ȷ����
u8 PS_Empty(void)
{
	u16 temp;
	u8  ensure;
	u8  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);//�������ʶ
	SendLength(0x03);
	Sendcmd(0x0D);
	temp = 0x01 + 0x03 + 0x0D;
	SendCheck(temp);
	data = JudgeStr(2000);
	if(data)
		ensure = data[9];
	else
		ensure = 0xff;
	return ensure;
}
//дϵͳ�Ĵ��� PS_WriteReg
//����:  дģ��Ĵ���
//����:  �Ĵ������RegNum:4\5\6
//˵��:  ģ�鷵��ȷ����
u8 PS_WriteReg(u8 RegNum, u8 DATA)
{
	u16 temp;
	u8  ensure;
	u8  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);//�������ʶ
	SendLength(0x05);
	Sendcmd(0x0E);
	Uart_SendOne(RegNum);
	Uart_SendOne(DATA);
	temp = RegNum + DATA + 0x01 + 0x05 + 0x0E;
	SendCheck(temp);
	data = JudgeStr(2000);
	if(data)
		ensure = data[9];
	else
		ensure = 0xff;
	if(ensure == 0)
		//printf("\r\n���ò����ɹ���");
		;
	else
		//printf("\r\n%s", EnsureMessage(ensure));
		;
	return ensure;
}
//��ϵͳ�������� PS_ReadSysPara
//����:  ��ȡģ��Ļ��������������ʣ�����С��)
//����:  ��
//˵��:  ģ�鷵��ȷ���� + ����������16bytes��
u8 PS_ReadSysPara(SysPara *p)
{
  u16 temp;
  u8  ensure;
  u8  *data;
  SendHead();
  SendAddr();
  SendFlag(0x01);//�������ʶ
  SendLength(0x03);
  Sendcmd(0x0F);
  temp = 0x01 + 0x03 + 0x0F;
  SendCheck(temp);
  data = JudgeStr(1000);
  if(data)
  {
    ensure = data[9];
    p->PS_max = (data[14] << 8) + data[15];
    p->PS_level = data[17];
    p->PS_addr = (data[18] << 24) + (data[19] << 16) + (data[20] << 8) + data[21];
    p->PS_size = data[23];
    p->PS_N = data[25];
  }
  else
    ensure = 0xff;
  if(ensure == 0x00)
  {
	  ;
    //printf("\r\nģ�����ָ������=%d", p->PS_max);
    //printf("\r\n�Աȵȼ�=%d", p->PS_level);
    //printf("\r\n��ַ=%x", p->PS_addr);
    //printf("\r\n������=%d", p->PS_N * 9600);
  }
  else
	  ;
    //printf("\r\n%s", EnsureMessage(ensure));
  return ensure;
}
//����ģ���ַ PS_SetAddr
//����:  ����ģ���ַ
//����:  PS_addr
//˵��:  ģ�鷵��ȷ����
u8 PS_SetAddr(u32 PS_addr)
{
  u16 temp;
  u8  ensure;
  u8  *data;
  SendHead();
  SendAddr();
  SendFlag(0x01);//�������ʶ
  SendLength(0x07);
  Sendcmd(0x15);
  Uart_SendOne(PS_addr >> 24);
  Uart_SendOne(PS_addr >> 16);
  Uart_SendOne(PS_addr >> 8);
  Uart_SendOne(PS_addr);
  temp = 0x01 + 0x07 + 0x15
         + (u8)(PS_addr >> 24) + (u8)(PS_addr >> 16)
         + (u8)(PS_addr >> 8) + (u8)PS_addr;
  SendCheck(temp);
  AS608Addr = PS_addr; //������ָ�������ַ
  data = JudgeStr(2000);
  if(data)
    ensure = data[9];
  else
    ensure = 0xff;
  AS608Addr = PS_addr;
  if(ensure == 0x00)
    //printf("\r\n���õ�ַ�ɹ���");
	;
  else
    //printf("\r\n%s", EnsureMessage(ensure));
	;
  return ensure;
}
//���ܣ� ģ���ڲ�Ϊ�û�������256bytes��FLASH�ռ����ڴ��û����±�,
//	�ü��±��߼��ϱ��ֳ� 16 ��ҳ��
//����:  NotePageNum(0~15),Byte32(Ҫд�����ݣ�32���ֽ�)
//˵��:  ģ�鷵��ȷ����
u8 PS_WriteNotepad(u8 NotePageNum, u8 *Byte32)
{
  u16 temp;
  u8  ensure, i;
  u8  *data;
  SendHead();
  SendAddr();
  SendFlag(0x01);//�������ʶ
  SendLength(36);
  Sendcmd(0x18);
  Uart_SendOne(NotePageNum);
  for(i = 0; i < 32; i++)
  {
    Uart_SendOne(Byte32[i]);
    temp += Byte32[i];
  }
  temp = 0x01 + 36 + 0x18 + NotePageNum + temp;
  SendCheck(temp);
  data = JudgeStr(2000);
  if(data)
    ensure = data[9];
  else
    ensure = 0xff;
  return ensure;
}
//������PS_ReadNotepad
//���ܣ�  ��ȡFLASH�û�����128bytes����
//����:  NotePageNum(0~15)
//˵��:  ģ�鷵��ȷ����+�û���Ϣ
u8 PS_ReadNotepad(u8 NotePageNum, u8 *Byte32)
{
  u16 temp;
  u8  ensure, i;
  u8  *data;
  SendHead();
  SendAddr();
  SendFlag(0x01);//�������ʶ
  SendLength(0x04);
  Sendcmd(0x19);
  Uart_SendOne(NotePageNum);
  temp = 0x01 + 0x04 + 0x19 + NotePageNum;
  SendCheck(temp);
  data = JudgeStr(2000);
  if(data)
  {
    ensure = data[9];
    for(i = 0; i < 32; i++)
    {
      Byte32[i] = data[10 + i];
    }
  }
  else
    ensure = 0xff;
  return ensure;
}
//��������PS_HighSpeedSearch
//���ܣ��� CharBuffer1��CharBuffer2�е������ļ��������������򲿷�ָ�ƿ⡣
//		  �����������򷵻�ҳ��,��ָ����ڵ�ȷ������ָ�ƿ��� ���ҵ�¼ʱ����
//		  �ܺõ�ָ�ƣ���ܿ�������������
//����:  BufferID�� StartPage(��ʼҳ)��PageNum��ҳ����
//˵��:  ģ�鷵��ȷ����+ҳ�루����ָ��ģ�壩
u8 PS_HighSpeedSearch(u8 BufferID, u16 StartPage, u16 PageNum, SearchResult *p)
{
  u16 temp;
  u8  ensure;
  u8  *data;
  SendHead();
  SendAddr();
  SendFlag(0x01);//�������ʶ
  SendLength(0x08);
  Sendcmd(0x1b);
  Uart_SendOne(BufferID);
  Uart_SendOne(StartPage >> 8);
  Uart_SendOne(StartPage);
  Uart_SendOne(PageNum >> 8);
  Uart_SendOne(PageNum);
  temp = 0x01 + 0x08 + 0x1b + BufferID
         + (StartPage >> 8) + (u8)StartPage
         + (PageNum >> 8) + (u8)PageNum;
  SendCheck(temp);
  data = JudgeStr(2000);
  if(data)
  {
    ensure = data[9];
    p->pageID 	= (data[10] << 8) + data[11];
    p->mathscore = (data[12] << 8) + data[13];
  }
  else
    ensure = 0xff;
  return ensure;
}
//����Чģ����� PS_ValidTempleteNum
//���ܣ�����Чģ�����
//����: ��
//˵��: ģ�鷵��ȷ����+��Чģ�����ValidN
u8 PS_ValidTempleteNum(u16 *ValidN)
{
  u16 temp;
  u8  ensure;
  u8  *data;
  SendHead();
  SendAddr();
  SendFlag(0x01);//�������ʶ
  SendLength(0x03);
  Sendcmd(0x1d);
  temp = 0x01 + 0x03 + 0x1d;
  SendCheck(temp);
  data = JudgeStr(2000);
  if(data)
  {
    ensure = data[9];
    *ValidN = (data[10] << 8) + data[11];
  }
  else
    ensure = 0xff;

  if(ensure == 0x00)
  {
   //printf("\r\n��Чָ�Ƹ���=%d", (data[10] << 8) + data[11]);
	  ;
  }
  else
	  ;
    //printf("\r\n%s", EnsureMessage(ensure));
  
  return ensure;
}
//��AS608���� PS_HandShake
//����: PS_Addr��ַָ��
//˵��: ģ�鷵�µ�ַ����ȷ��ַ��
u8 PS_HandShake(u32 *PS_Addr)
{
	SendHead();
	SendAddr();
	Uart_SendOne(0X01);
	Uart_SendOne(0X00);
	Uart_SendOne(0X00);
	delay_ms(200);

	if(Uart_Receive_Data(judgeRecvi,&receivelen))
	{
		if(//�ж��ǲ���ģ�鷵�ص�Ӧ���
		  judgeRecvi[0] == 0XEF
		  && judgeRecvi[1] == 0X01
		  && judgeRecvi[6] == 0X07
		)	
		{
		  *PS_Addr = (judgeRecvi[2] << 24) + (judgeRecvi[3] << 16)
					 + (judgeRecvi[4] << 8) + (judgeRecvi[5]);
		  return 0;
		}
	}
	return 1;
}
//ģ��Ӧ���ȷ������Ϣ����
//���ܣ�����ȷ���������Ϣ������Ϣ
//����: ensure
const char *EnsureMessage(u8 ensure)
{
  const char *p;
  switch(ensure)
  {
  case  0x00:
    p = "       OK       ";
    break;
  case  0x01:
    p = " ���ݰ����մ��� ";
    break;
  case  0x02:
    p = "��������û����ָ";
    break;
  case  0x03:
    p = "¼��ָ��ͼ��ʧ��";
    break;
  case  0x04:
    p = " ָ��̫�ɻ�̫�� ";
    break;
  case  0x05:
    p = " ָ��̫ʪ��̫�� ";
    break;
  case  0x06:
    p = "  ָ��ͼ��̫��  ";
    break;
  case  0x07:
    p = " ָ��������̫�� ";
    break;
  case  0x08:
    p = "  ָ�Ʋ�ƥ��    ";
    break;
  case  0x09:
    p = " û��������ָ�� ";
    break;
  case  0x0a:
    p = "   �����ϲ�ʧ�� ";
    break;
  case  0x0b:
    p = "��ַ��ų�����Χ";
  case  0x10:
    p = "  ɾ��ģ��ʧ��  ";
    break;
  case  0x11:
    p = " ���ָ�ƿ�ʧ�� ";
    break;
  case  0x15:
    p = "������������Чͼ";
    break;
  case  0x18:
    p = " ��дFLASH����  ";
    break;
  case  0x19:
    p = "   δ�������   ";
    break;
  case  0x1a:
    p = "  ��Ч�Ĵ�����  ";
    break;
  case  0x1b:
    p = " �Ĵ������ݴ��� ";
    break;
  case  0x1c:
    p = " ���±�ҳ����� ";
    break;
  case  0x1f:
    p = "    ָ�ƿ���    ";
    break;
  case  0x20:
    p = "    ��ַ����    ";
    break;
  default :
    p = " ����ȷ�������� ";
    break;
  }
  return p;
}

//��ʾȷ���������Ϣ
void ShowErrMessage(u8 ensure)
{
  OLED_ShowCH(0,2,(u8*)EnsureMessage(ensure));
}

//¼ָ��
void Add_FR(u16 ID_NUM)
{
	int  key_num = 0;
	u8 i, ensure, processnum = 0;
	while(1)
	{
		switch (processnum)
		{	
			case 0:
				i++;
				OLED_ShowCH(0, 2, "	    �밴��ָ");
				ensure = PS_GetImage();
				if(ensure == 0x00)
				{
					//printf("����ڶ���");
					ensure = PS_GenChar(CharBuffer1); //��������
					//printf("���������");
					if(ensure == 0x00)
					{
						OLED_ShowCH(0, 2, "	    ָ������");
						i = 0;
						processnum = 1; //�����ڶ���
					}
					else ShowErrMessage(ensure);
				}
				else ShowErrMessage(ensure);
				break;

			case 1:
				i++;
				OLED_ShowCH(0, 2, "	   ���ٰ�һ��");
				delay_ms(1500);
				ensure = PS_GetImage();
				if(ensure == 0x00)
				{
					ensure = PS_GenChar(CharBuffer2); //��������
					if(ensure == 0x00)
					{
						OLED_ShowCH(0, 2, "	    ָ������  ");
						i = 0;
						processnum = 2; //����������
					}
					else ShowErrMessage(ensure);
				}
				else ShowErrMessage(ensure);
				break;

			case 2:
				OLED_ShowCH(0, 2, "	  �Ա�����ָ��");
				delay_ms(300);
				ensure = PS_Match();
				if(ensure == 0x00)
				{
					OLED_ShowCH(0, 2, "	     �Աȳɹ�  ");
					processnum = 3; //�������Ĳ�
				}
				else
				{
					OLED_ShowCH(0, 2, "	     �Ա�ʧ��  ");
					ShowErrMessage(ensure);
					i = 0;
					processnum = 0; //���ص�һ��
				}
				delay_ms(300);
				break;
			case 3:
				OLED_ShowCH(0, 2, "	  ����ָ��ģ��");
				delay_ms(300);
				ensure = PS_RegModel();
				if(ensure == 0x00)
				{
					OLED_ShowCH(0, 2, "	����ָ��ģ��ɹ�");
					processnum = 4; //�������岽
				}
				else
				{
					processnum = 0;
					ShowErrMessage(ensure);
				}
				delay_ms(1000);
				break;
			case 4:
				OLED_ShowCH(0, 0, " ��S13��,��S15�� ");
				OLED_ShowCH(0, 2, "    ��S14����    ");
				OLED_ShowCH(0, 4, "  0 =< ID <= 99  ");
				while(key_num != 14)
				{
					OLED_ShowCH(40, 6, "ID=");
					OLED_ShowNum(65, 6, ID_NUM, 2, 16);
					key_num = Key_Scan();
					if(key_num == 15)
					{
						delay_ms(300);
						if(key_num == 15)
						{
							key_num = 0;
							if(ID_NUM > 0)
								ID_NUM--;				
						}
					}
					if(key_num == 13)
					{
						delay_ms(300);
						if(key_num == 13)
						{		
							key_num = 0;
							if(ID_NUM < 99)
								ID_NUM++;				
						}
					}
				}
				key_num = 0;
				ensure = PS_StoreChar(CharBuffer2, ID_NUM); //����ģ��
				if(ensure == 0x00)
				{
					idNum[ID_NUM] = 1;
					writeIdNum();
					OLED_Clear(0);
					OLED_ShowCH(0, 2, "	  ¼��ָ�Ƴɹ�  ");
					delay_ms(1500);
					OLED_Clear(0);
					OLED_ShowCH(16,0,"S1�����ָ��");
					OLED_ShowCH(16,2,"S2��ɾ��ָ��");
					OLED_ShowCH(16,4,"S3����ָ֤��");
					OLED_ShowCH(16,6,"S4��    ����");
					staff_num += 1;  //Ա����Ŀ+1
					return ;
				}
				else
				{
					OLED_Clear(0);
					processnum = 0;
					ShowErrMessage(ensure);
				}
				break;
		}
		delay_ms(400);
		if(i == 10)//����5��û�а���ָ���˳�
		{
			break;
		}
	}
}


SysPara AS608Para;//ָ��ģ��AS608����
//ˢָ��
void press_FR(void)
{
	int  key_num = 0;
	SearchResult seach;
	u8 ensure;
	char str[20];
	while(key_num != 1)
	{
		key_num = Key_Scan();
		ensure = PS_GetImage();
		if(ensure == 0x00) //��ȡͼ��ɹ�
		{
		  ensure = PS_GenChar(CharBuffer1);
		  if(ensure == 0x00) //���������ɹ�
		  {
			ensure = PS_HighSpeedSearch(CharBuffer1, 0, 99, &seach);
			if(ensure == 0x00) //�����ɹ�
			{
			  OLED_ShowCH(32, 2, "��֤�ɹ�");
			  sprintf(str, " ID:%d �÷�:%d ", seach.pageID, seach.mathscore);
			  OLED_ShowCH(20, 5, (u8*)str);
			  delay_ms(1500);
			  delay_ms(1500);
			}
			else
			{
			  OLED_ShowCH(32, 2, "��֤ʧ��");
			  delay_ms(1500);
			}
		  }
		  else
				{};
		  OLED_Clear(0);
		  OLED_ShowCH(32, 2, "�밴��ָ");
		}
	}
	OLED_Clear(0);
	OLED_ShowCH(16,0,"S1�����ָ��");
	OLED_ShowCH(16,2,"S2��ɾ��ָ��");
	OLED_ShowCH(16,4,"S3����ָ֤��");
	OLED_ShowCH(16,6,"S4��    ����");
}

void Normal_press_FR(void)
{
	SearchResult seach;
	u8 ensure;
	ensure = PS_GetImage();
	if(ensure == 0x00) //��ȡͼ��ɹ�
	{
	  ensure = PS_GenChar(CharBuffer1);
	  if(ensure == 0x00) //���������ɹ�
	  {
		ensure = PS_HighSpeedSearch(CharBuffer1, 0, 99, &seach);
		OLED_Clear(0);
		if(ensure == 0x00) //�����ɹ�
		{
		  OLED_ShowCH(0, 2, "	    ǩ���ɹ�");
			//printf("seach.pageID : %d",seach.pageID);
		  writeTime(seach.pageID);  //��¼����
		  delay_ms(1500);
		}
		else
		{
		  OLED_ShowCH(0, 2, "	    ǩ��ʧ��");
		  delay_ms(1500);
		}
		OLED_Clear(0);
	  }
	  else
			{};
	}
	//OLED_Clear(0);
}

//ɾ��ָ��
void Del_FR(u16 ID_NUM )
{
  int key_num = 0;
  u8  ensure;
  OLED_ShowCH(0, 0, "S5�� S6�� S7ȷ��");
  OLED_ShowCH(0, 2, "  S8���ָ�ƿ�  ");
  OLED_ShowCH(0, 4, "	     S4����      ");
  while(key_num != 7)
  {
    key_num = Key_Scan();
    if(key_num == 6)
    {
		delay_ms(300);
		if(key_num == 6)
		{
		  key_num = 0;
		  if(ID_NUM > 0)
			ID_NUM--;		
		}
    }
    if(key_num == 5)
    {
		delay_ms(300);
		if(key_num == 5)
		{
		  key_num = 0;
		  if(ID_NUM < 99)
			ID_NUM++;			
		}
    }
    if(key_num == 4)
      goto MENU ; //������ҳ��
    if(key_num == 8)
    {
      key_num = 0;
      ensure = PS_Empty(); //���ָ�ƿ�
      if(ensure == 0)
      {
        OLED_Clear(0);
        OLED_ShowCH(0, 2, " ���ָ�ƿ�ɹ� ");
		  staff_num = 0;  //Ա����ĿΪ0
		  emptyIDNum();
      }
      else
        ShowErrMessage(ensure);
      delay_ms(1500);
      goto MENU ; //������ҳ��
    }
    OLED_ShowCH(40, 6, "ID=");
    OLED_ShowNum(65, 6, ID_NUM, 2, 16);
  }
  ensure = PS_DeletChar(ID_NUM, 1); //ɾ������ָ��
  if(ensure == 0)
  {
    OLED_Clear(0);
	idNum[ID_NUM] = 0;
	  writeIdNum();
    OLED_ShowCH(0, 2, "  ɾ��ָ�Ƴɹ�  ");
	  staff_num -= 1;  //Ա����Ŀ-1
  }
  else
    ShowErrMessage(ensure);
  delay_ms(1500);
MENU:
	OLED_Clear(0);
	OLED_ShowCH(16,0,"S1�����ָ��");
	OLED_ShowCH(16,2,"S2��ɾ��ָ��");
	OLED_ShowCH(16,4,"S3����ָ֤��");
	OLED_ShowCH(16,6,"S4��    ����");
	key_num = 0;
}


