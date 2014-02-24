#ifndef __FLASH_H
#define __FLASH_H			    
#include "sys.h" 
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//Mini STM32开发板
//AT45DB 驱动函数	   
//修改日期:2011年11月13日21:38:42
///////////////////////////////////////////////////////////	  
#define	SPI_FLASH_CS     PCout(2)  //选中FLASH					 
///////////////////////////////////////////////////////////
extern u16 SPI_FLASH_TYPE;//定义我们使用的flash芯片型号

//AT45DB读写
#define FLASH_ID       0X1F26//ID
#define Dummy_Byte     0xA5	 //虚读
//指令表
#define SPI_FLASH_PageSize            528	//每页528个字节
#define SPI_FLASH_PageNum             4096//每扇区
///////////////////////////////////////////////////////////
#define BUFFER_1_WRITE                0x84// 写入第一缓冲区
#define BUFFER_2_WRITE                0x87// 写入第二缓冲区
#define BUFFER_1_READ                 0xD4// 读取第一缓冲区
#define BUFFER_2_READ                 0xD6// 读取第二缓冲区
#define B1_TO_MM_PAGE_PROG_WITH_ERASE 0x83// 将第一缓冲区的数据写入主存储器（擦除模式）
#define B2_TO_MM_PAGE_PROG_WITH_ERASE 0x86// 将第二缓冲区的数据写入主存储器（擦除模式）
#define MM_PAGE_TO_B1_XFER            0x53// 将主存储器的指定页数据加载到第一缓冲区
#define MM_PAGE_TO_B2_XFER            0x55// 将主存储器的指定页数据加载到第二缓冲区
#define PAGE_ERASE                    0x81// 页擦除  （每页512/528字节）
#define SECTOR_ERASE                  0x7C// 扇区擦除（每扇区128K字节）
#define READ_STATE_REGISTER           0xD7// 读取状态寄存器
/////////////////////////////////////////////////////////////
/*----- High layer function -----*/
void SPI_Flash_Write(u8 *pBuffer, u32 WriteAddr, u16 NumByteToWrite);//读取flash
void SPI_Flash_Read (u8 *pBuffer, u32 ReadAddr,  u16 NumByteToRead); //读取flash
/////////////////////////////////////////////////////////////////////////
void DF_mm_to_buf(u8 buffer,unsigned int page);//将指定主存储器页的数据转入指定缓冲区
void DF_buf_to_mm(u8 buffer,u16 page);//将指定缓冲区中的数据写入主存储区的指定页
void DF_page_earse(u16 page);       //擦除指定的主存储器页（地址范围0-4095）
/*----- Low layer function -----*/
u8	 SPI_Flash_ReadSR(void);        //读取状态寄存器
u16  SPI_Flash_ReadID(void);  	    //读取FLASH ID
void SPI_Flash_Init(void);					//SPI初始化
void SPI_Flash_Wait_Busy(void);     //等待空闲
#endif
















