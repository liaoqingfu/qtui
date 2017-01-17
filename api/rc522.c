/********************************************************************************************************* 
    Copyright (C) 2012, Changsha Spon Electronic Co., Ltd
	created:	2014/08/06
	filename: 	RC522.c
	author:		leilei <drylei@163.com>
	modify:        lhg
	
    Description  : RC522配置
******************************************************************************************************** 
* $Id:$ 
******************************************************************************************************** 
*/

//----------------- 2. 变量定义 -----------------------
unsigned char ucCardData[4];
unsigned char ucCardTmpData[4];
typedef unsigned char  BYTE;

#include "rc522.h"
#include <sys/ioctl.h> 
#include <fcntl.h> 
#include <linux/i2c-dev.h> 
#include <linux/i2c.h> 

#define CHIP "/dev/i2c-1" 
#define CHIP_ADDR 0x28 

int rc522_fd = 0;

int RC522_init(void)
{
	if( rc522_fd > 0)  //alread opened
		return 0;
	
	rc522_fd = open(CHIP, O_RDWR);      
	if (rc522_fd < 0) 
	{         
	  printf("open "CHIP"failed\n");          
	  goto exit;      
	}      
	if (ioctl(rc522_fd, I2C_SLAVE_FORCE, CHIP_ADDR) < 0) 
	{         /* 设置芯片地址 */         
	  printf("oictl:set slave address failed\n");        
	  goto close;      
	}

	return rc522_fd;
	close:      
	close( rc522_fd ); 
	exit:      
	return 0; 
}

void RC522_uninit(void)
{
	if (rc522_fd > 0) 
		close( rc522_fd ); 
	rc522_fd = 0;

}


int main() 
{      
   printf("hello, this is i2c tester\n");         
   int fd = open(CHIP, O_RDWR);      
   if (fd < 0) 
   {         
      printf("open "CHIP"failed\n");          
      goto exit;      
   }      
   if (ioctl(fd, I2C_SLAVE_FORCE, CHIP_ADDR) < 0) 
   {         /* 设置芯片地址 */         
      printf("oictl:set slave address failed\n");        
      goto close;      
   }

   unsigned char rddata;      
   unsigned char rdaddr[2] = {0, 0};     /* 将要读取的数据在芯片中的偏移量 */      
   unsigned char wrbuf[3] = {0, 0, 0x3c};     /* 要写的数据，头两字节为偏移量 */         
 /*
   printf("input a char you want to write to E2PROM\n");     
   wrbuf[2] = getchar();     
   printf("write return:%d, write data:%x\n", write(fd, wrbuf, 3), wrbuf[2]);     
   sleep(1);     */
   printf("write address return: %d\n",write(fd, rdaddr, 2));     /* 读取之前首先设置读取的偏移量 */     
   printf("read data return:%d\n", read(fd, &rddata, 1));     
   printf("rddata: %c\n", rddata); 
close:      
  close(fd); 
exit:      
  return 0; 
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
	if( rc522_fd > 0)
	{
		unsigned char wrbuf[2] = {ucAddress, ucValue};
		write(rc522_fd, wrbuf, 2);
	}
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

