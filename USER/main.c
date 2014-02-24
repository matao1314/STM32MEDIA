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
#if 1
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
	uart_init(72,115200);	 	
	delay_init(72);	 
	LCD_Init(); 
	LED_Init();
 	mem_init(SRAMIN);	//初始化内部内存池	
// 	tp_dev.init();

  I2C_EE_Init();
	I2C_Test();

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
//		mp3_play();
  	delay_ms(500);
	 LED0=0;
   delay_ms(500);
	 LED0=1;

  }
}
#else 

//得到path路径下,目标文件的总个数
//path:路径		    
//返回值:总有效文件数
u16 pic_get_tnum(u8 *path)
{	  
	u8 res;
	u16 rval=0;
 	DIR tdir;	 		//临时目录
	FILINFO tfileinfo;	//临时文件信息	
	u8 *fn;	 			 			   			     
    res=f_opendir(&tdir,(const TCHAR*)path); //打开目录
  	tfileinfo.lfsize=_MAX_LFN*2+1;						//长文件名最大长度
	tfileinfo.lfname=mymalloc(SRAMIN,tfileinfo.lfsize);	//为长文件缓存区分配内存
	if(res==FR_OK&&tfileinfo.lfname!=NULL)
	{
		while(1)//查询总的有效文件数
		{
	        res=f_readdir(&tdir,&tfileinfo);       		//读取目录下的一个文件
	        if(res!=FR_OK||tfileinfo.fname[0]==0)break;	//错误了/到末尾了,退出		  
     		fn=(u8*)(*tfileinfo.lfname?tfileinfo.lfname:tfileinfo.fname);			 
			res=f_typetell(fn);	
			if((res&0XF0)==0X50)//取高四位,看看是不是图片文件	
			{
				rval++;//有效文件数增加1
			}	    
		}  
	} 
	return rval;
}
	
 int main(void)
 {	 
	u8 res;
 	DIR picdir;	 		//图片目录
	FILINFO picfileinfo;//文件信息
	u8 *fn;   			//长文件名
	u8 *pname;			//带路径的文件名
	u16 totpicnum; 		//图片文件总数
	u16 curindex;		//图片当前索引
	u8 key;				//键值
	u8 pause=0;			//暂停标记
	u8 t;
	u16 temp;
	u16 *picindextbl;	//图片索引表

 
	uart_init(72,115200);	 	
	delay_init(72);	 
//	NVIC_Configuration(); 	 //设置NVIC中断分组2:2位抢占优先级，2位响应优先级
 	LED_Init();			     //LED端口初始化
	LCD_Init();	
//	usmart_dev.init(72);//usmart初始化	
 	mem_init(SRAMIN);	//初始化内部内存池	
	SD_USER_Init();
 	exfuns_init();					//为fatfs相关变量申请内存  
  f_mount(0,fs[0]); 		 		//挂载SD卡 
 	f_mount(1,fs[1]); 				//挂载FLASH.
	POINT_COLOR=RED;      
 	while(font_init()) 				//检查字库
	{	    
		LCD_ShowString(60,50,200,16,16,"Font Error!");
		delay_ms(200);				  
		LCD_Fill(60,50,240,66,WHITE);//清除显示	 
			 	update_font(20,110,16,0);//从SD卡更新  	 
    
	}  	 
 	Show_Str(60,50,200,16,"战舰 STM32开发板",16,0);				    	 
	Show_Str(60,70,200,16,"图片显示程序",16,0);				    	 
	Show_Str(60,90,200,16,"KEY0:NEXT KEY2:PREV",16,0);				    	 
	Show_Str(60,110,200,16,"正点原子@ALIENTEK",16,0);				    	 
	Show_Str(60,130,200,16,"2012年9月19日",16,0);
 	while(f_opendir(&picdir,"0:/PICTURE"))//打开图片文件夹
 	{	    
		Show_Str(60,150,240,16,"PICTURE文件夹错误!",16,0);
		delay_ms(200);				  
		LCD_Fill(60,150,240,146,WHITE);//清除显示	     
		delay_ms(200);				  
	}  
	totpicnum=pic_get_tnum("0:/PICTURE"); //得到总有效文件数
  	while(totpicnum==NULL)//图片文件为0		
 	{	    
		Show_Str(60,150,240,16,"没有图片文件!",16,0);
		delay_ms(200);				  
		LCD_Fill(60,150,240,146,WHITE);//清除显示	     
		delay_ms(200);				  
	}
  	picfileinfo.lfsize=_MAX_LFN*2+1;						//长文件名最大长度
	picfileinfo.lfname=mymalloc(SRAMIN,picfileinfo.lfsize);	//为长文件缓存区分配内存
 	pname=mymalloc(SRAMIN,picfileinfo.lfsize);				//为带路径的文件名分配内存
 	picindextbl=mymalloc(SRAMIN,2*totpicnum);				//申请2*totpicnum个字节的内存,用于存放图片索引
 	while(picfileinfo.lfname==NULL||pname==NULL||picindextbl==NULL)//内存分配出错
 	{	    
		Show_Str(60,150,240,16,"内存分配失败!",16,0);
		delay_ms(200);				  
		LCD_Fill(60,150,240,146,WHITE);//清除显示	     
		delay_ms(200);				  
	}  	
	//记录索引
    res=f_opendir(&picdir,"0:/PICTURE"); //打开目录
	if(res==FR_OK)
	{
		curindex=0;//当前索引为0
		while(1)//全部查询一遍
		{
			temp=picdir.index;								//记录当前index
	        res=f_readdir(&picdir,&picfileinfo);       		//读取目录下的一个文件
	        if(res!=FR_OK||picfileinfo.fname[0]==0)break;	//错误了/到末尾了,退出		  
     		fn=(u8*)(*picfileinfo.lfname?picfileinfo.lfname:picfileinfo.fname);			 
			res=f_typetell(fn);	
			if((res&0XF0)==0X50)//取高四位,看看是不是图片文件	
			{
				picindextbl[curindex]=temp;//记录索引
				curindex++;
			}	    
		} 
	}   
	Show_Str(60,150,240,16,"开始显示...",16,0); 
	delay_ms(1500);
	piclib_init();										//初始化画图	   	   
  	curindex=0;											//从0开始显示
   	res=f_opendir(&picdir,(const TCHAR*)"0:/PICTURE"); 	//打开目录
	while(res==FR_OK)//打开成功
	{	
		dir_sdi(&picdir,picindextbl[curindex]);			//改变当前目录索引	   
        res=f_readdir(&picdir,&picfileinfo);       		//读取目录下的一个文件
        if(res!=FR_OK||picfileinfo.fname[0]==0)break;	//错误了/到末尾了,退出
     	fn=(u8*)(*picfileinfo.lfname?picfileinfo.lfname:picfileinfo.fname);			 
		strcpy((char*)pname,"0:/PICTURE/");				//复制路径(目录)
		strcat((char*)pname,(const char*)fn);  			//将文件名接在后面
		printf("name=%s\n\r",pname);
		LCD_Clear(BLACK);
 		ai_load_picfile(pname,0,0,lcddev.width,lcddev.height);//显示图片    
		Show_Str(2,2,240,16,pname,16,1); 				//显示图片名字
			curindex++;		   	
				if(curindex>=totpicnum)curindex=0;//到末尾的时候,自动从头开始
//		t=0;
//		while(1) 
//		{
//			key=1;
//			if(t>250)key=2;	//模拟一次按下右键	    
//			if(key==1)		//上一张
//			{
//				if(curindex)curindex--;
//				else curindex=totpicnum-1;
//				break;
//			}else if(key==1)//下一张
//			{
//				curindex++;		   	
//				if(curindex>=totpicnum)curindex=0;//到末尾的时候,自动从头开始
//				break;
//			}
//			if(pause==0)t++;
			delay_ms(500); 
//		}					    
		res=0;  
	} 											  
	myfree(SRAMIN,picfileinfo.lfname);	//释放内存			    
	myfree(SRAMIN,pname);				//释放内存			    
	myfree(SRAMIN,picindextbl);			//释放内存			    
}
#endif

