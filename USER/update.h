#ifndef _UPDATE_H
#define _UPDATE_H
#include "sys.h"
#include "stdio.h"	

#define APP_FLASH_ADDR 0x08010000 //64KB

void spiflash_update(void);
void udisk_update(void);

#endif
