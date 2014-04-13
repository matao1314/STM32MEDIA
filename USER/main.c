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
#include "myui.h"//��������
#include "guix.h"
#include "piclib.h"
#include "i2cee.h"
#include "fontupd.h"
#include "text.h"
#include "touch.h" 
#include "myui.h"
#include "key.h"
//u16 volatile OSTime;	//fix error



SD_CardInfo   SDCardInfo;    // ���SD������Ϣ
SD_Error SD_USER_Init(void);//SD����ʼ��
void TEST(void);
//ϵͳ��λ
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
	Status = SD_GetCardInfo( &SDCardInfo );//���SD��������Ϣ
//printf( " \r\n Card ���ͣ� %d ", SDCardInfo.CardType );
  printf("SDCard���� = %dMB\r\n", SDCardInfo.CardCapacity>>20);
//printf( " \r\n Card ���С %d ", SDCardInfo.CardBlockSize );
//printf( " \r\n RCA  ��%d ", SDCardInfo.RCA);
//printf( " \r\n ������ ID is ��%d ", SDCardInfo.SD_cid.ManufacturerID );
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
	/* ��ѡһ�ֶ����Թ��� */ 
	//Status = SD_SetDeviceMode( SD_DMA_MODE );
	Status = SD_SetDeviceMode( SD_POLLING_MODE );
	//Status = SD_SetDeviceMode( SD_INTERRUPT_MODE );	
	}
	return ( Status );
}
void Load_Drow_Dialog(void)
{
	LCD_Clear(WHITE);//����   
 	POINT_COLOR=BLUE;//��������Ϊ��ɫ 
	LCD_ShowString(216,0,200,16,16,"RST");//��ʾ��������
  	POINT_COLOR=RED;//���û�����ɫ 
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
 	mem_init(SRAMIN);	//��ʼ���ڲ��ڴ��
	SPI1_Init();
	VS_Init();	  		//��ʼ��VS1053 	
	KEY_Init();	
  I2C_EE_Init();
	//���SD���Ƿ�ɹ�
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

	sd_fs_init();//�ļ�ϵͳ��ʼ��-�����ֿⱣ����sd���У������̷�����Ϊ0 
	exfuns_init();					//Ϊfatfs��ر��������ڴ�  
	f_mount(0,fs[0]); 		 		//����SD��
	f_mount(1,fs[1]); 				//����FLASH.
  while(font_init()) 				//����ֿ�
	{	    
		LCD_ShowString(60,50,200,16,16,"Font Error!");
		delay_ms(200);				  
		LCD_Fill(60,50,240,66,WHITE);//�����ʾ	 
		update_font(20,110,16,0);//��SD������  	 
	}  	 
	tp_dev.init();
 
	if(0==VS_HD_Reset()){
		myprntf("HResetOk!\r\n"); 
	}

	VS_Soft_Reset();
	printf("SResetOk!\r\n"); 
	VS_Ram_Test();
	printf("Ram Test:0X%04X\r\n",VS_Ram_Test());//��ӡRAM���Խ��	    
	VS_Sine_Test();	   
	printf("Board Init Over!\r\n");


LCD_ShowString(60,150,240,320,16,"SYSTEM OK! ");
gui_init();	
piclib_init();//��ʼ����ͼ	
Draw_mainPage();//����������
  while (1)
  {
	printf("AD VALUE==%d\r\n",KEY_Scan(0));
//	tp_dev.scan(0); 		 
//		if(tp_dev.sta&TP_PRES_DOWN)			//������������
//		{	
//			printf("X=%d\r\n",tp_dev.x); 
//			printf("Y=%d\r\n",tp_dev.y); 
//		 	if(tp_dev.x<lcddev.width&&tp_dev.y<lcddev.height)
//			{	
//				if(tp_dev.x>(lcddev.width-24)&&tp_dev.y<16)Load_Drow_Dialog();//���
//				else TP_Draw_Big_Point(tp_dev.x,tp_dev.y,RED);		//��ͼ	  			   
//			}
//		}
//		mp3_play();
  	delay_ms(500);
	 LED0=0;
   delay_ms(500);
	 LED0=1;
  }
}
