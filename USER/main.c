/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include <stdio.h>
#include "stm32f10x.h"
#include "sys.h"
#include "usart.h"		
#include "delay.h"	
#include "led.h"
#include "lcd.h"	
#include "spi.h"
#include "vs10XX.h"	
#include "sdcard.h"
#include "diskio.h"
#include "sd_fs_app.h"
#include "exfuns.h"
#include "malloc.h"	    
#include "mp3player.h"
#include "myui.h"//操作界面
#include "guix.h"
#include "piclib.h"
#include "i2cee.h"
#include "fontupd.h"
#include "text.h"
#include "touch.h" 
#include "myui.h"

//u16 volatile OSTime;	//fix error



SD_CardInfo   SDCardInfo;    // 存放SD卡的信息
SD_Error SD_USER_Init(void);//SD卡初始化
void TEST(void);
//系统软复位
void Sys_Soft_Reset(void)
{   
	SCB->AIRCR =0X05FA0000|(u32)0x04;	  
} 

SD_Error SD_USER_Init(void)
{
	SD_Error   Status = SD_OK;
	/* SD Init */
	Status =SD_Init();
	if (Status == SD_OK)
	{
	/* Read CSD/CID MSD registers */
	Status = SD_GetCardInfo( &SDCardInfo );//获得SD卡索引信息
//printf( " \r\n Card 类型： %d ", SDCardInfo.CardType );
  printf( " \r\n Card 容量MB： %d ", SDCardInfo.CardCapacity>>20 );
//printf( " \r\n Card 块大小 %d ", SDCardInfo.CardBlockSize );
//printf( " \r\n RCA  ：%d ", SDCardInfo.RCA);
//printf( " \r\n 制造商 ID is ：%d ", SDCardInfo.SD_cid.ManufacturerID );
	}
	if (Status == SD_OK)
	{
	/* Select Card */
	Status = SD_SelectDeselect( (u32) (SDCardInfo.RCA << 16) );
	}
	if (Status == SD_OK)
	{
	/* set bus wide */
	Status = SD_EnableWideBusOperation( SDIO_BusWide_1b );
	}
	/* Set Device Transfer Mode to DMA */
	if (Status == SD_OK)
	{ 
	/* 任选一种都可以工作 */ 
	//Status = SD_SetDeviceMode( SD_DMA_MODE );
	Status = SD_SetDeviceMode( SD_POLLING_MODE );
	//Status = SD_SetDeviceMode( SD_INTERRUPT_MODE );	
	}
	return ( Status );
}
void Load_Drow_Dialog(void)
{
	LCD_Clear(WHITE);//清屏   
 	POINT_COLOR=BLUE;//设置字体为蓝色 
	LCD_ShowString(216,0,200,16,16,"RST");//显示清屏区域
  	POINT_COLOR=RED;//设置画笔蓝色 
}

/**
  * @brief  Main program.
  * @param  None
  * @retval None
  */
u16 ID1=0;
const u8 TEXT_Buffer[]={"WarShipSTM32 IIC TEST"};
	u8 datatemp[sizeof(TEXT_Buffer)];
#define  EEP_Firstpage      0x00
u8 I2c_Buf_Write[256];
u8 I2c_Buf_Read[256];

void I2C_Test(void)
{
	u16 i;

	printf("写入的数据\n\r");
    
	for ( i=0; i<=255; i++ ) //填充缓冲
  {   
    I2c_Buf_Write[i] = i;

    printf("0x%02X ", I2c_Buf_Write[i]);
    if(i%16 == 15)    
        printf("\n\r");    
   }

  //将I2c_Buf_Write中顺序递增的数据写入EERPOM中 
	I2C_EE_BufferWrite( I2c_Buf_Write, EEP_Firstpage, 256);	 
  
  printf("\n\r读出的数据\n\r");
  //将EEPROM读出数据顺序保持到I2c_Buf_Read中 
	I2C_EE_BufferRead(I2c_Buf_Read, EEP_Firstpage, 256); 

  //将I2c_Buf_Read中的数据通过串口打印
	for (i=0; i<256; i++)
	{	
		if(I2c_Buf_Read[i] != I2c_Buf_Write[i])
		{
			printf("0x%02X ", I2c_Buf_Read[i]);
			printf("错误:I2C EEPROM写入与读出的数据不一致\n\r");
			return;
		}
    printf("0x%02X ", I2c_Buf_Read[i]);
    if(i%16 == 15)    
        printf("\n\r");
    
	}
  printf("I2C(AT24C02)读写测试成功\n\r");
}

int main(void)
{
  /* Add your application code here
     */
		 u8 a=1;
		 u8 b=2; 
	uart_init(72,115200);	 	
	delay_init(72);	 
	LCD_Init(); 
	LED_Init();
 	mem_init(SRAMIN);	//初始化内部内存池
	SPI1_Init();		
  //I2C_EE_Init();
	//I2C_Test();

	I2C_EE_BufferWrite( &a, EEP_Firstpage, 1);	 
	I2C_EE_BufferWrite( &b, EEP_Firstpage+1, 1);	 

	I2C_EE_BufferRead(&b, EEP_Firstpage, 1); 
	I2C_EE_BufferRead(&a, EEP_Firstpage+1, 1);
	printf("a=%d",a); 
	printf("b=%d",b); 
	tp_dev.init();


	//检测SD卡是否成功
	while(SD_USER_Init()!=SD_OK)
	{
		POINT_COLOR=RED;
		LCD_ShowString(20, 10,8,16,16, "SD Card File!            ");
		LED0=0;
		delay_ms(500);
		LCD_ShowString(20, 10,8,16,16,"Please Check SD Card!");
		LED0=1;
		delay_ms(500);
	}

	sd_fs_init();//文件系统初始化-汉字字库保存在sd卡中，并将盘符设置为0 
	exfuns_init();					//为fatfs相关变量申请内存  
	f_mount(0,fs[0]); 		 		//挂载SD卡
	f_mount(1,fs[1]); 				//挂载FLASH.
  	while(font_init()) 				//检查字库
	{	    
		LCD_ShowString(60,50,200,16,16,"Font Error!");
		delay_ms(200);				  
		LCD_Fill(60,50,240,66,WHITE);//清除显示	 
			 	update_font(20,110,16,0);//从SD卡更新  	 
    
	}  	 
 
	VS_Init();	  		//初始化VS1053 
	if(0==VS_HD_Reset()){
	myprntf("HResetOk!\r\n"); 
	}

	VS_Soft_Reset();
	printf("SResetOk!\r\n"); 
	ID1=	VS_Ram_Test();
	printf("Ram Test:0X%04X\r\n",VS_Ram_Test());//打印RAM测试结果	    
	VS_Sine_Test();	   
	printf("Board Init Over!\r\n");


LCD_ShowString(60,150,240,320,16,"SYSTEM OK! ");
gui_init();	
piclib_init();//初始化画图	
Draw_mainPage();//加载主界面
  while (1)
  {
	tp_dev.scan(0); 		 
		if(tp_dev.sta&TP_PRES_DOWN)			//触摸屏被按下
		{	

			printf("X=%d\r\n",tp_dev.x); 
			printf("Y=%d\r\n",tp_dev.y); 
		 	if(tp_dev.x<lcddev.width&&tp_dev.y<lcddev.height)
			{	
				if(tp_dev.x>(lcddev.width-24)&&tp_dev.y<16)Load_Drow_Dialog();//清除
				else TP_Draw_Big_Point(tp_dev.x,tp_dev.y,RED);		//画图	  			   
			}
		}
		mp3_play();
  	delay_ms(500);
	 LED0=0;
   delay_ms(500);
	 LED0=1;

  }
}
