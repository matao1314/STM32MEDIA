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
  printf( " \r\n Card ����MB�� %d ", SDCardInfo.CardCapacity>>20 );
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
u16 ID1=0;
const u8 TEXT_Buffer[]={"WarShipSTM32 IIC TEST"};
	u8 datatemp[sizeof(TEXT_Buffer)];
#define  EEP_Firstpage      0x00
u8 I2c_Buf_Write[256];
u8 I2c_Buf_Read[256];

void I2C_Test(void)
{
	u16 i;

	printf("д�������\n\r");
    
	for ( i=0; i<=255; i++ ) //��仺��
  {   
    I2c_Buf_Write[i] = i;

    printf("0x%02X ", I2c_Buf_Write[i]);
    if(i%16 == 15)    
        printf("\n\r");    
   }

  //��I2c_Buf_Write��˳�����������д��EERPOM�� 
	I2C_EE_BufferWrite( I2c_Buf_Write, EEP_Firstpage, 256);	 
  
  printf("\n\r����������\n\r");
  //��EEPROM��������˳�򱣳ֵ�I2c_Buf_Read�� 
	I2C_EE_BufferRead(I2c_Buf_Read, EEP_Firstpage, 256); 

  //��I2c_Buf_Read�е�����ͨ�����ڴ�ӡ
	for (i=0; i<256; i++)
	{	
		if(I2c_Buf_Read[i] != I2c_Buf_Write[i])
		{
			printf("0x%02X ", I2c_Buf_Read[i]);
			printf("����:I2C EEPROMд������������ݲ�һ��\n\r");
			return;
		}
    printf("0x%02X ", I2c_Buf_Read[i]);
    if(i%16 == 15)    
        printf("\n\r");
    
	}
  printf("I2C(AT24C02)��д���Գɹ�\n\r");
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
 	mem_init(SRAMIN);	//��ʼ���ڲ��ڴ��
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
 
	VS_Init();	  		//��ʼ��VS1053 
	if(0==VS_HD_Reset()){
	myprntf("HResetOk!\r\n"); 
	}

	VS_Soft_Reset();
	printf("SResetOk!\r\n"); 
	ID1=	VS_Ram_Test();
	printf("Ram Test:0X%04X\r\n",VS_Ram_Test());//��ӡRAM���Խ��	    
	VS_Sine_Test();	   
	printf("Board Init Over!\r\n");


LCD_ShowString(60,150,240,320,16,"SYSTEM OK! ");
gui_init();	
piclib_init();//��ʼ����ͼ	
Draw_mainPage();//����������
  while (1)
  {
	tp_dev.scan(0); 		 
		if(tp_dev.sta&TP_PRES_DOWN)			//������������
		{	

			printf("X=%d\r\n",tp_dev.x); 
			printf("Y=%d\r\n",tp_dev.y); 
		 	if(tp_dev.x<lcddev.width&&tp_dev.y<lcddev.height)
			{	
				if(tp_dev.x>(lcddev.width-24)&&tp_dev.y<16)Load_Drow_Dialog();//���
				else TP_Draw_Big_Point(tp_dev.x,tp_dev.y,RED);		//��ͼ	  			   
			}
		}
		mp3_play();
  	delay_ms(500);
	 LED0=0;
   delay_ms(500);
	 LED0=1;

  }
}
