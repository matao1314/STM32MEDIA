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
u16  OSTime;	//fix error
SD_CardInfo   SDCardInfo;    // 存放SD卡的信息
SD_Error SD_USER_Init(void);//SD卡初始化
void TEST(void);
//系统软复位
void Sys_Soft_Reset(void)
{   
	SCB->AIRCR =0X05FA0000|(u32)0x04;	  
} 

//JTAG模式设置,用于设置JTAG的模式
//mode:jtag,swd模式设置;00,全使能;01,使能SWD;10,全关闭;
void JTAG_Set(u8 mode)
{
	u32 temp;
	temp=mode;
	temp<<=25;
	RCC->APB2ENR|=1<<0;     //开启辅助时钟	   
	AFIO->MAPR&=0XF8FFFFFF; //清除MAPR的[26:24]
	AFIO->MAPR|=temp;       //设置jtag模式
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
FATFS ff;
u16 ID1=0;
int main(void)
{
  /* Add your application code here
     */
	uart_init(72,115200);	 	
	delay_init(72);	 
	JTAG_Set(1);  	 
	LCD_Init(); 
	LED_Init();
 	mem_init(SRAMIN);	//初始化内部内存池	
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
	NVIC_Configuration(); 
  exfuns_init();					//为fatfs相关变量申请内存  

	f_mount(0,fs[0]); 		 		//挂载SD卡 
//	f_mount(0,&ff); 		 		//挂载SD卡 
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
  while (1)
  {
		mp3_play();
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
if(res == FR_OK) //打开文件夹
{		
//printf("res:%d line=%d\r\n",f_readdir(&dir, &fno),__LINE__ );

while (f_readdir(&dir, &fno) == FR_OK) //按照顺序读文件夹
{	
	printf("文件名:%s\r\n",fno.fname);
	//加在这里好使
	strcpy(path,aa); //把aa内容复制到path中	
	if(!fno.fname[0])     break; //如果文件名为0,结束
	{
		if(fno.fattrib == AM_ARC) //判断文件属性			
		{
			strcat(path,fno.fname);//把文件名连接到path后面
			printf("路径:%s\r\n",path);
		}
	}
}
}
}
