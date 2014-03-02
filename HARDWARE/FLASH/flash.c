#include "flash.h" 
#include "spi.h"
#include "delay.h"
#include "usart.h"	 
#include "common.h"
#include "touch.h"
//////////////////////////////////////////////////////////////////////////////////	 
u16 SPI_FLASH_TYPE=FLASH_ID;//默认就是25Q64
////////////////////////////////////////////////////////////////////////////////// 	  
//32个扇区为1个Block
//256页为一个扇区
//AT45DB
//容量为2M字节,共有512个块,15个Sector（扇区）4096页，每页528字节
//初始化SPI FLASH的IO口
void SPI_Flash_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE );
	//Configure PC2 pin: M45PE_CS pin 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2|GPIO_Pin_3; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 	//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
	GPIO_Init(GPIOC,&GPIO_InitStructure);

	SPI1_Init();		   	//初始化SPI
	//SPI1_SetSpeed(SPI_BaudRatePrescaler_64);//设置为18M时钟,高速模式	 SPI_BaudRatePrescaler_2
	SPI_FLASH_TYPE=SPI_Flash_ReadID();//读取FLASH ID.  
	printf("FlashID=:%x\r\n",SPI_FLASH_TYPE); //打印LCD ID  
}
//读取SPI_FLASH的状态寄存器
//Status Register Format
//Bit 7     Bit 6  Bit 5  Bit 4  Bit 3  Bit 2  Bit 1      Bit 0
//RDY/BUSY  COMP   1      0      1      1      PROTECT    PAGE SIZE
//BUSY:忙标记位(0,忙;1,空闲)
//PAGE SIZE  0:512b  1:528b
//默认:0x00
u8 SPI_Flash_ReadSR(void)   
{  
	u8 byte=0;   
	CS_Seclect(CS_SPI_FLASH);//使能器件   
	SPI1_ReadWriteByte(READ_STATE_REGISTER);//发送读取状态寄存器命令
  SPI1_ReadWriteByte(0X00); 
	SPI1_ReadWriteByte(0X00); 
	SPI1_ReadWriteByte(0X00);     
	byte=SPI1_ReadWriteByte(0Xff);      //读取一个字节  
	SPI_FLASH_CS=1;                     //取消片选
	TCS=0;     
	return byte;   
} 
//读取芯片ID AT45DB161 的ID:0X1F26
u16 SPI_Flash_ReadID(void)
{
	u16 Temp = 0;
  SPI_Flash_Wait_Busy();//判断是否为忙 
	CS_Seclect(CS_SPI_FLASH);//使能器件   
	SPI1_ReadWriteByte(0x9F);//发送读取ID命令	    
	Temp|=SPI1_ReadWriteByte(Dummy_Byte)<<8;  
	Temp|=SPI1_ReadWriteByte(Dummy_Byte);
	SPI1_ReadWriteByte(Dummy_Byte);//通过写入0xFF可以读出数据 	    
	SPI1_ReadWriteByte(Dummy_Byte); 	    
	SPI_FLASH_CS=1;
	return Temp;
}   		    
//等待芯片空闲
void SPI_Flash_Wait_Busy(void)   
{   
	while (!(SPI_Flash_ReadSR()&0x80));//等待BUSY置1
}

//将指定主存储器页的数据转入指定缓冲区
void DF_mm_to_buf(u8 buffer,unsigned int page)
{	  
  SPI_Flash_Wait_Busy();//判断是否为忙 
	CS_Seclect(CS_SPI_FLASH);//使能器件   
	if(buffer==1)   SPI1_ReadWriteByte(MM_PAGE_TO_B1_XFER);	  
	else            SPI1_ReadWriteByte(MM_PAGE_TO_B2_XFER);  
	SPI1_ReadWriteByte((u8)(page >> 6));
  SPI1_ReadWriteByte((u8)(page << 2));
  SPI1_ReadWriteByte(0x00);
	SPI_FLASH_CS=1;
} 
//将指定缓冲区中的数据写入主存储区的指定页
void DF_buf_to_mm(u8 buffer,u16 page)
{
  SPI_Flash_Wait_Busy();//判断是否为忙 
	if(page<SPI_FLASH_PageNum)
	{
   	CS_Seclect(CS_SPI_FLASH);//使能器件   
		if(buffer==1)    SPI1_ReadWriteByte(B1_TO_MM_PAGE_PROG_WITH_ERASE); 
		else             SPI1_ReadWriteByte(B2_TO_MM_PAGE_PROG_WITH_ERASE);	 
		SPI1_ReadWriteByte((u8)(page>>6));
		SPI1_ReadWriteByte((u8)(page<<2));
		SPI1_ReadWriteByte(0x00);
	  SPI_FLASH_CS=1;
	}
}
//从指定地址开始写入指定个数的数据	     
void SPI_Flash_Write(u8 *pBuffer,u32 WriteAddr,u16 NumByteToWrite)
{
	u16 offaddr,i;
	u32 paddr;//页地址
	SPI1_SetSpeed(SPI_BaudRatePrescaler_2);//设置到高速模式
	paddr=WriteAddr/SPI_FLASH_PageSize;  //得到开始写入的首页地址
	offaddr=WriteAddr%SPI_FLASH_PageSize;//得到在首页地址里的偏移
	DF_mm_to_buf(1,paddr);//将开始页数据读出到buf1			  
  SPI_Flash_Wait_Busy();//判断是否为忙 
	CS_Seclect(CS_SPI_FLASH);//使能器件   
	SPI1_ReadWriteByte(BUFFER_1_WRITE);//写入1缓冲区命令	 		   
	SPI1_ReadWriteByte(0x00);//14bit 无效数据+10bit地址数据 
	SPI1_ReadWriteByte((u8)(offaddr>>8));//写入在该页的偏移地址   
	SPI1_ReadWriteByte((u8)offaddr);
	for (i=0;i<NumByteToWrite;)//发送数据
	{
		SPI1_ReadWriteByte(*pBuffer);//写入一个数据
		pBuffer++;
		i++;
		if((i+offaddr)%SPI_FLASH_PageSize==0)
		{
	    SPI_FLASH_CS=1;				    
			DF_buf_to_mm(1,paddr);//把BUF1的内容写入主存储器
			paddr++;	   
			if(paddr>=SPI_FLASH_PageNum)return; //超出了AT45DB161的范围
			DF_mm_to_buf(1,paddr); //将开始页数据读出到buf1			  
      SPI_Flash_Wait_Busy();//判断是否为忙 
			CS_Seclect(CS_SPI_FLASH);//使能器件   
			SPI1_ReadWriteByte(BUFFER_1_WRITE);//写入1缓冲区命令	 	 		   
			SPI1_ReadWriteByte(0x00);//14bit 无效数据+10bit地址数据 
			SPI1_ReadWriteByte(0x00);//设置地址到0 
			SPI1_ReadWriteByte(0x00);		    
		}			   
	}
	SPI_FLASH_CS=1;
	DF_buf_to_mm(1,paddr);//把BUF1的内容写入主存储器			  
} 

//从指定地址开始读出指定个数的数据	     
void SPI_Flash_Read(u8* pBuffer, u32 ReadAddr, u16 NumByteToRead)
{
	u16 offaddr,i;
	u32 paddr;//页地址
	SPI1_SetSpeed(SPI_BaudRatePrescaler_2);//设置到高速模式
	paddr=ReadAddr/SPI_FLASH_PageSize;  //得到开始写入的首页地址
	offaddr=ReadAddr%SPI_FLASH_PageSize;//得到在首页地址里的偏移
	DF_mm_to_buf(1,paddr);//将开始页数据读出到buf1			  
  SPI_Flash_Wait_Busy();//判断是否为忙 
	CS_Seclect(CS_SPI_FLASH);//使能器件   
	SPI1_ReadWriteByte(BUFFER_1_READ);//读1缓冲区命令	 		   
	SPI1_ReadWriteByte(0x00);//14bit 无效数据+10bit地址数据 
	SPI1_ReadWriteByte((u8)(offaddr>>8));//写入在该页的偏移地址   
	SPI1_ReadWriteByte((u8)offaddr);
	SPI1_ReadWriteByte(0x00); //等待 
	for (i=0;i<NumByteToRead;)//读取NumByteToRead个数据
	{
		*pBuffer=SPI1_ReadWriteByte(0xff);//读取一个数据
		pBuffer++;
		i++;
		if((i+offaddr)%SPI_FLASH_PageSize==0)
		{
	    SPI_FLASH_CS=1;				    
			paddr++;	    
			if(paddr>=SPI_FLASH_PageNum)   return; //超出了AT45DB161的范围
			DF_mm_to_buf(1,paddr); //将开始页数据读出到buf1			  
      SPI_Flash_Wait_Busy();//判断是否为忙 
			CS_Seclect(CS_SPI_FLASH);//使能器件   
			SPI1_ReadWriteByte(BUFFER_1_READ);//读1缓冲区命令	 		   
			SPI1_ReadWriteByte(0x00);//14bit 无效数据+10bit地址数据 
			SPI1_ReadWriteByte(0x00);//设置地址到0 
			SPI1_ReadWriteByte(0x00);
			SPI1_ReadWriteByte(0x00);//等待		    
		}			   
	}
	SPI_FLASH_CS=1;	
} 
//擦除指定的主存储器页（地址范围0-4095）
void DF_page_earse(u16 page)
{		  
  SPI_Flash_Wait_Busy();//判断是否为忙 
	CS_Seclect(CS_SPI_FLASH);//使能器件   
	SPI1_ReadWriteByte(PAGE_ERASE);
	SPI1_ReadWriteByte((u8)(page >> 6));
	SPI1_ReadWriteByte((u8)(page << 2));
	SPI1_ReadWriteByte(0x00);
	SPI_FLASH_CS=1;
}
