#ifndef __KEY_H
#define __KEY_H	 
#include "sys.h"

#define KEY_NOPRESS 255

#define KEY_DOWN	4
#define KEY_UP 		3
#define KEY_LEFT	2
#define KEY_RIGHT	1

void KEY_Init(void);//IO初始化
u8 KEY_Scan(u8);  	//按键扫描函数					    
#endif
