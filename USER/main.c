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
#include "key.h"
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
  printf("SDCard容量 = %dMB\r\n", SDCardInfo.CardCapacity>>20);
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
int main(void)
{
  /* Add your application code here
     */
	uart_init(72,115200);	 	
	delay_init(72);	 
	LCD_Init(); 
	LED_Init();
 	mem_init(SRAMIN);	//初始化内部内存池
	SPI1_Init();
	VS_Init();	  		//初始化VS1053 	
	KEY_Init();	
  I2C_EE_Init();
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
	tp_dev.init();
 
	if(0==VS_HD_Reset()){
		myprntf("HResetOk!\r\n"); 
	}

	VS_Soft_Reset();
	printf("SResetOk!\r\n"); 
	VS_Ram_Test();
	printf("Ram Test:0X%04X\r\n",VS_Ram_Test());//打印RAM测试结果	    
	VS_Sine_Test();	   
	printf("Board Init Over!\r\n");


LCD_ShowString(60,150,240,320,16,"SYSTEM OK! ");
gui_init();	
piclib_init();//初始化画图	
Draw_mainPage();//加载主界面
  while (1)
  {
	printf("AD VALUE==%d\r\n",KEY_Scan(0));
//	tp_dev.scan(0); 		 
//		if(tp_dev.sta&TP_PRES_DOWN)			//触摸屏被按下
//		{	
//			printf("X=%d\r\n",tp_dev.x); 
//			printf("Y=%d\r\n",tp_dev.y); 
//		 	if(tp_dev.x<lcddev.width&&tp_dev.y<lcddev.height)
//			{	
//				if(tp_dev.x>(lcddev.width-24)&&tp_dev.y<16)Load_Drow_Dialog();//清除
//				else TP_Draw_Big_Point(tp_dev.x,tp_dev.y,RED);		//画图	  			   
//			}
//		}
//		mp3_play();
  	delay_ms(500);
	 LED0=0;
   delay_ms(500);
	 LED0=1;
  }
}
