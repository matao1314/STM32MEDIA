//Comon define
#ifndef _COMMON_H
#define _COMMON_H

typedef enum {
	CS_SPI_FLASH=0,
	CS_SPI_TOUCH,
	CS_SD_CARD,
	CS_OFF,
}_CS_SELECT;

extern void CS_Seclect(_CS_SELECT cs);

#endif

