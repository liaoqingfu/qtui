/********************************************************************************************************* 
    Copyright (C) 2012, Changsha Spon Electronic Co., Ltd
	created:	2014/08/06
	filename: 	RC522.c
	author:		leilei <drylei@163.com>
	
    Description  : RC522配置
******************************************************************************************************** 
* $Id:$ 
******************************************************************************************************** 
*/
#include "Types.h"
#include "I2C_STC11Fxx.h"
#include "Version.h"
#include "RC522.h"

//----------------- 1. 宏及结构本声明 -----------------
#define RC522_ADDR             0x50

//----------------- 2. 变量定义 -----------------------
unsigned char ucCardData[4];
unsigned char ucCardTmpData[4];

//----------------- 3. 函数声明 -----------------------

//----------------- 4. 函数定义 -----------------------
// 连续写寄存器数据
static BOOL RC522_WriteBuf(BYTE ucAddr, BYTE *pBuf, BYTE ucSize)
{
    BYTE    i;
    int     nAck = 1;
	
    I2CStart();
    I2CSendByte(RC522_ADDR);
    nAck = I2CSendByte(ucAddr);
    for (i = 0; nAck != 0 && i < ucSize; i++)
        nAck = I2CSendByte(pBuf[i]);
	I2CStop();
    return ((i == ucSize) ? TRUE : FALSE);
}

// 读寄存器数据
static BYTE RC522_Read(BYTE ucAddr)
{
    BYTE    ucDevice, ucData;
	
	ucAddr = ucAddr & 0x7F;
    ucDevice = RC522_ADDR;
    I2CStart();
    I2CSendByte(ucDevice);
    I2CSendByte(ucAddr);
    ucDevice = RC522_ADDR | 1;
    I2CStart();
    I2CSendByte(ucDevice);
    I2CRecvByte(&ucData, FALSE);
    I2CStop();    
	return ucData;
}

// 读取寄存器值
static BYTE RC522_ReadRawRC(BYTE ucAddress)
{
	BYTE ucRet;
	
	ucRet = RC522_Read(ucAddress);
	return ucRet;
}

// 写寄存器值
static void RC522_WriteRawRC(BYTE ucAddress, BYTE ucValue)
{
	RC522_WriteBuf(ucAddress, &ucValue, 1);
}

VOID RC522_Delay(BYTE ucDelay)
{
	BYTE ucLoop;
	while (ucDelay-- > 0) {
		for (ucLoop = 0; ucLoop < 124; ucLoop++);
    }
}

//复位RC522
BYTE RC522_Reset(void)
{
	RC522_WriteRawRC(CommandReg, PCD_RESETPHASE);
	RC522_Delay(1);
	RC522_WriteRawRC(ModeReg, 0x3D);
	RC522_WriteRawRC(TReloadRegL, 30);
	RC522_WriteRawRC(TReloadRegH, 0);
	RC522_WriteRawRC(TModeReg, 0x8D);
	RC522_WriteRawRC(TPrescalerReg, 0x3E);   
	RC522_WriteRawRC(RFCfgReg, 0x70);	// 射频强度
	return MI_OK; 
}

//置RC522寄存器位
static void SetBitMask(unsigned char reg, unsigned char mask)  
{
	unsigned char tmp = 0x0;
	tmp = RC522_ReadRawRC(reg) | mask;
	RC522_WriteRawRC(reg, tmp | mask);  // set bit mask
}

//清RC522寄存器位
static void ClearBitMask(unsigned char reg, unsigned char mask)  
{
	unsigned char tmp = 0x0;
	tmp = RC522_ReadRawRC(reg)&(~mask);
	RC522_WriteRawRC(reg, tmp);  // clear bit mask
} 

//开启天线发射  
void RC522_AntennaOn(void)
{
	unsigned char i;
	RC522_WriteRawRC(TxASKReg,0x40);
	RC522_Delay(10);
	i = RC522_ReadRawRC(TxControlReg);
	if (!(i&0x03))
		SetBitMask(TxControlReg, 0x03);
	i = RC522_ReadRawRC(TxASKReg);
}

//关闭天线发射
void RC522_AntennaOff(void)
{
	ClearBitMask(TxControlReg, 0x03);
}

//通过RC522和ISO14443卡通讯
char RC522_ComMF522(unsigned char Command, unsigned char *pInData, 
                 unsigned char InLenByte, unsigned char *pOutData, 
                 unsigned int  *pOutLenBit)
{
	char status = MI_ERR;
	unsigned char irqEn   = 0x00;
	unsigned char waitFor = 0x00;
	unsigned char lastBits;
	unsigned char n;
	unsigned int  i;

	switch (Command)
	{
	case PCD_AUTHENT:
		irqEn   = 0x12;
		waitFor = 0x10;
	break;
	case PCD_TRANSCEIVE:
		irqEn   = 0x77;
		waitFor = 0x30;
	break;
	default:
	  break;
	}
	RC522_WriteRawRC(ComIEnReg,irqEn|0x80);
	ClearBitMask(ComIrqReg,0x80);
	RC522_WriteRawRC(CommandReg,PCD_IDLE);
	SetBitMask(FIFOLevelReg,0x80); 				// 清空FIFO 
	for (i=0; i<InLenByte; i++)
		RC522_WriteRawRC(FIFODataReg,pInData[i]); 		// 数据写入FIFO 
	RC522_WriteRawRC(CommandReg, Command); 				// 命令写入命令寄存器
	if(Command == PCD_TRANSCEIVE)
		SetBitMask(BitFramingReg,0x80); 			// 开始发送     
	i = 6000; 									//根据时钟频率调整，操作M1卡最大等待时间25ms
	do {
		n = RC522_ReadRawRC(ComIrqReg);
		i--;
	}
	while((i!=0)&&!(n&0x01)&&!(n&waitFor));
	ClearBitMask(BitFramingReg,0x80);
	if (i!=0){
	if (!(RC522_ReadRawRC(ErrorReg)&0x1B))
	{
		status = MI_OK;
		if (n&irqEn&0x01)
		status = MI_NOTAGERR;
		if(Command==PCD_TRANSCEIVE)
		{
			n = RC522_ReadRawRC(FIFOLevelReg);
			lastBits = RC522_ReadRawRC(ControlReg)&0x07;
			if(lastBits)
			  *pOutLenBit = (n-1)*8 + lastBits;
			else
			  *pOutLenBit = n*8;
			if(n==0)
			  n = 1;
			if(n>MAXRLEN)
			  n = MAXRLEN;
			for (i=0; i<n; i++)
			  pOutData[i] = RC522_ReadRawRC(FIFODataReg); 
		}
	}
	else
	  status = MI_ERR;        
	}
	SetBitMask(ControlReg,0x80);// stop timer now
	RC522_WriteRawRC(CommandReg,PCD_IDLE); 
	return status;
}

//寻卡
char RC522_Request(unsigned char req_code,unsigned char *pTagType)
{
	char status;  
	unsigned int  unLen;
	unsigned char ucComMF522Buf[MAXRLEN]; 

	ClearBitMask(Status2Reg,0x08);
	RC522_WriteRawRC(BitFramingReg,0x07);
	SetBitMask(TxControlReg,0x03);

	ucComMF522Buf[0] = req_code;

	status = RC522_ComMF522(PCD_TRANSCEIVE,ucComMF522Buf,
					   1,ucComMF522Buf,&unLen);
	if ((status == MI_OK) && (unLen == 0x10)){    
		*pTagType     = ucComMF522Buf[0];
		*(pTagType+1) = ucComMF522Buf[1];
	}
	else
	status = MI_ERR;
	return status;
}

//防冲撞
char RC522_Anticoll(unsigned char *pSnr)
{
    char status;
    unsigned char i,snr_check=0;
    unsigned int  unLen;
    unsigned char ucComMF522Buf[MAXRLEN]; 
    
    ClearBitMask(Status2Reg,0x08);
    RC522_WriteRawRC(BitFramingReg,0x00);
    ClearBitMask(CollReg,0x80);
 
    ucComMF522Buf[0] = PICC_ANTICOLL1;
    ucComMF522Buf[1] = 0x20;

    status = RC522_ComMF522(PCD_TRANSCEIVE,ucComMF522Buf,2,ucComMF522Buf,&unLen);

    if (status == MI_OK)
    {
    	 for (i = 0; i < 4; i++){   
             *(pSnr+i)  = ucComMF522Buf[i];
             snr_check ^= ucComMF522Buf[i];
         }
         if (snr_check != ucComMF522Buf[i]){   
			status = MI_ERR;   
		}
    }
    
    SetBitMask(CollReg,0x80);
    return status;
}

// 读卡信息
BYTE RC522_AutoReader(void)
{
	BYTE ucRet = 0;
	
    if(RC522_Request(0x52,ucCardData)==MI_OK)
    {
		if(ucCardData[0]==0x04&&ucCardData[1]==0x00)  ucRet = 1;
//			PutString("MFOne-S50");
		else if(ucCardData[0]==0x02&&ucCardData[1]==0x00) ucRet = 2;
//			PutString("MFOne-S70");
		else if(ucCardData[0]==0x44&&ucCardData[1]==0x00) ucRet = 3;
//			PutString("MF-UltraLight"); 
		else if(ucCardData[0]==0x08&&ucCardData[1]==0x00) ucRet = 4;
//			PutString("MF-Pro");
		else if(ucCardData[0]==0x44&&ucCardData[1]==0x03) ucRet = 5;
//			PutString("MF Desire");
		else
			ucRet = 0;
//			PutString("Unknown");
		if ((ucRet == 1) && RC522_Anticoll(ucCardData)==MI_OK)
		{ 
			ucCardTmpData[0]=ucCardData[0];
			ucCardTmpData[1]=ucCardData[1];
			ucCardTmpData[2]=ucCardData[2];
			ucCardTmpData[3]=ucCardData[3];
////			tochar(UID[1]);
////			tochar(UID[2]);
////			tochar(UID[3]);
////			UARTSendByte('\n');
		}
    }
	return ucRet;
}

