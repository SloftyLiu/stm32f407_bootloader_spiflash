#ifndef _UPDATE_H
#define _UPDATE_H
#include "sys.h"
#include "stdio.h"	

#define APP_FLASH_ADDR 0x08010000 //64KB

typedef enum {
	UPDATE_OK = 0,
	UPDATE_NO_NEED = 0,
	UPDATE_FAIL,
}UpdateResult_Tpyedef;

UpdateResult_Tpyedef spiflash_update(void);
UpdateResult_Tpyedef udisk_update(void);
UpdateResult_Tpyedef sdcard_update(void);

#endif
