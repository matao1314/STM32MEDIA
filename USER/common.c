#include "common.h"
#include "flash.h"
#include "touch.h"

//Switch chip cs
void CS_Seclect(_CS_SELECT cs)
{
	switch(cs){
	 case CS_SPI_FLASH:{SPI_FLASH_CS=0;TCS=1;break;}//PC2
	 case CS_SPI_TOUCH:{TCS=0;SPI_FLASH_CS=1;break;}//PC4
	 case CS_SD_CARD:  {break;}//PC3暂时没用到
	 case CS_OFF:      {SPI_FLASH_CS=1;TCS=1;break;}
	 default:break; 
	}
}  
