/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "sys.h"
#include "usart.h"		
#include "delay.h"	
#include "led.h"
#include "lcd.h"	
#include "spi.h"
#include <string.h>
#include <stdio.h>

#include "vs10XX.h"	
#include "sdcard.h"
#include "diskio.h"
#include "sd_fs_app.h"

SD_CardInfo   SDCardInfo;    // ���SD������Ϣ
SD_Error SD_USER_Init(void);//SD����ʼ��
void TEST(void);
//ϵͳ��λ
void Sys_Soft_Reset(void)
{   
	SCB->AIRCR =0X05FA0000|(u32)0x04;	  
} 

//JTAGģʽ����,��������JTAG��ģʽ
//mode:jtag,swdģʽ����;00,ȫʹ��;01,ʹ��SWD;10,ȫ�ر�;
void JTAG_Set(u8 mode)
{
	u32 temp;
	temp=mode;
	temp<<=25;
	RCC->APB2ENR|=1<<0;     //��������ʱ��	   
	AFIO->MAPR&=0XF8FFFFFF; //���MAPR��[26:24]
	AFIO->MAPR|=temp;       //����jtagģʽ
} 


void NVIC_Configuration(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    
    /* Configure the NVIC Preemption Priority Bits */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    
    NVIC_InitStructure.NVIC_IRQChannel = SDIO_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
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
	Status = SD_SetDeviceMode( SD_DMA_MODE );
	//Status = SD_SetDeviceMode( SD_POLLING_MODE );
	//Status = SD_SetDeviceMode( SD_INTERRUPT_MODE );	
	}
	return ( Status );
}


/**
  * @brief  Main program.
  * @param  None
  * @retval None
  */
u16 ID1=0;
SD_CardInfo info;
FATFS	 fs;
int main(void)
{
  /* Add your application code here
     */
	uart_init(72,115200);	 	
	delay_init(72);	 
	JTAG_Set(1);  	 
	LCD_Init(); 
	LED_Init();

//���SD���Ƿ�ɹ�
while(SD_USER_Init()!=SD_OK)
{	POINT_COLOR=RED;
//LCD_ShowString(u16 x,u16 y,u16 width,u16 height,u8 size,u8 *p);
LCD_ShowString(20, 10,8,16,16, "SD Card File!            ");
LED0=0;
delay_ms(500);
LCD_ShowString(20, 10,8,16,16,"Please Check SD Card!");
LED0=1;
delay_ms(500);
}
  sd_fs_init();//�ļ�ϵͳ��ʼ��-�����ֿⱣ����sd���У������̷�����Ϊ0 
	NVIC_Configuration(); 
  f_mount(0,&fs); 		 		//����SD�� 
VS_Init();	  		//��ʼ��VS1053 
if(0==VS_HD_Reset()){
myprntf("HResetOk!\r\n"); 
}
VS_Soft_Reset();
printf("SResetOk!\r\n"); 
ID1=	VS_Ram_Test();
printf("Ram Test:0X%04X\r\n",VS_Ram_Test());//��ӡRAM���Խ��	    
VS_Sine_Test();	   

//SD_GetCardInfo(&info);
//printf( " \r\n Card ���ͣ� %d ", info.CardType );
//printf( " \r\n Card ����MB�� %d ", info.CardCapacity>>20 );
//printf( " \r\n Card ���С %d ", info.CardBlockSize );
//printf( " \r\n RCA  ��%d ", info.RCA);
//printf( " \r\n ������ ID is ��%d ", info.SD_cid.ManufacturerID );
printf("Board Init Over!\r\n"); 
TEST();
   while (1)
  {
	
  LCD_Clear(RED);
	delay_ms(500);

	 LED0=0;
   delay_ms(500);
	 LED0=1;

  }
}


void TEST()
{
FRESULT res;
FILINFO fno;
DIR     dir;
static  char path[20]; 
static  char aa[9]="MUSIC/";
res=f_opendir(&dir,"MUSIC");
printf("res:%d\r\n",res);
if(res == FR_OK) //���ļ���
{	
while (f_readdir(&dir, &fno) == FR_OK) //����˳����ļ���
{	
	printf("�ļ���:%s\r\n",fno.fname);
	//���������ʹ
	strcpy(path,aa); //��aa���ݸ��Ƶ�path��	
	if(!fno.fname[0])     break; //����ļ���Ϊ0,����
	{
		if(fno.fattrib == AM_ARC) //�ж��ļ�����			
		{
			strcat(path,fno.fname);//���ļ������ӵ�path����
			printf("·��:%s\r\n",path);
		}
	}
}
}
}
