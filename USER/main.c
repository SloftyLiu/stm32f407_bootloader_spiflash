#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "key.h"
#include "malloc.h" 
#include "w25qxx.h"
#include "ff.h"
#include "string.h"
#include "key.h"
#include "usbd_msc_core.h"
//#include "usbd_usr.h"
#include "usbd_desc.h"
#include "usb_conf.h"
#include "usbd_msc_bot.h"
#include "usbh_usr.h" 
#include "update.h"
#include "crc.h"

FATFS fs;
FATFS udisk_fs;
//USB_OTG_CORE_HANDLE USB_OTG_dev;
USBH_HOST  USB_Host;
USB_OTG_CORE_HANDLE  USB_OTG_Core;

typedef enum {
	SPIFLASH_UPDATE = 0,
	UDISK_UPDATE,
}Update_Tpyedef;

Update_Tpyedef update_type =  SPIFLASH_UPDATE;

//���U���Ƿ����firmware.bin
//����ֵ:0,����
//       1,������
u8 USH_User_App(void)
{
	FRESULT res = f_mount(&udisk_fs,"2:",1);
	if(res == 0)
	{
		printf("U disk mount OK\r\n");
		// �ж�flash��Ŀ¼���Ƿ���firmware.bin�ļ�
		FILINFO fno;
		res = f_stat("2:/firmware.bin", &fno);
		if (res == FR_OK) {
			// �ļ�����
			printf("firmware.bin found, size: %lu bytes\r\n", fno.fsize);
			update_type = UDISK_UPDATE;
		}	
		else {
				// �ļ�������
				printf("firmware.bin not found!\r\n");
		}
	}
	else
	{
		printf("U disk mount fail!!!!!!!!!res %d\r\n",res);
	}
	return res;
} 

//У��һ���ļ�CRC32��demo
int fileCRC32_check(void) 
{
    FATFS fs;
    FIL file;
    FRESULT res;
    UINT br;
    uint8_t buffer[512];
    uint32_t crc = 0;

    res = f_mount(&fs, "1:", 1);
    if (res != FR_OK) 
		{
        printf("Mount failed: %d\n", res);
        return 1;
    }

    res = f_open(&file, "1:/test.bin", FA_READ);
    if (res != FR_OK) 
		{
        printf("Open failed: %d\n", res);
        return 1;
    }

    crc = 0x0;
    do
		{
        res = f_read(&file, buffer, sizeof(buffer), &br);
        if (res != FR_OK) 
				{
            printf("Read failed: %d\n", res);
            f_close(&file);
            return 1;
        }
        if (br > 0) 
				{
            crc = CRC32Software(buffer, br, crc);
        }
    } 
		while (br == sizeof(buffer));

    f_close(&file);

    printf("File CRC32: 0X%08X\n", crc);

    return 0;
}

CRC_HandleTypeDef stm32_CRC;

int main(void)
{
  HAL_Init();                   	//��ʼ��HAL��    

  Stm32_Clock_Init(336,8,2,7);  	//����ʱ��,168Mhz
	delay_init(168);               	//��ʼ����ʱ����
	uart_init(115200);             	//��ʼ��USART
	LED_Init();						//��ʼ��LED	
	KEY_Init();						//��ʼ��KEY 
	W25QXX_Init();				    //��ʼ��W25Q256
	my_mem_init(SRAMIN);			//��ʼ���ڲ��ڴ��
		
	printf("*********************\r\n");
	printf("* Bootloader start! *\r\n");
	printf("* V0.4              *\r\n");
	printf("*********************\r\n");
	
	uint8_t  crc_soft[4] = {0x11,0x22,0x33,0x44};
	uint32_t crc_hard[4] = {0x11223344};
	
	__HAL_RCC_CRC_CLK_ENABLE();
	stm32_CRC.Instance = CRC;
	HAL_CRC_Init(&stm32_CRC);
	
	printf("crc32 soft = 0X%X, hard = 0X%X \r\n",CRC32Software(crc_soft,0x4,0xffffffff), HAL_CRC_Calculate(&stm32_CRC,crc_hard,1));
	fileCRC32_check();
	
	switch( KEY_Scan(0) )
	{
		case KEY0_PRES :
			printf("KEY0 is pressed! Now you can copy the firmware.bin!\r\n");
			//TODO:
			//USBD_Init(&USB_OTG_dev,USB_OTG_FS_CORE_ID,&USR_desc,&USBD_MSC_cb,&USR_cb);
			while(1);
		case KEY1_PRES :
			printf("KEY1 is pressed! Now the check firmware.bin in USB disk!\r\n");
		  USBH_Init(&USB_OTG_Core,USB_OTG_FS_CORE_ID,&USB_Host,&USBH_MSC_cb,&USR_cb);  
			while(1)
			{
				USBH_Process(&USB_OTG_Core, &USB_Host);
				if(UDISK_UPDATE == update_type)
				{
					break;
				}
				delay_ms(1);
			}					
			break;	
		default:
			break;
	}
	
	//ִ�и�������
	switch(update_type)
	{
		case SPIFLASH_UPDATE:
			spiflash_update();
			break;
		case UDISK_UPDATE:
			udisk_update();
			break;
	}
	
	printf("������ת��APP...\r\n");
	void (*app_entry_ptr)(void);
	__set_MSP(*(uint32_t *)APP_FLASH_ADDR); //����ջָ��
	app_entry_ptr = (void (*)(void))(*(uint32_t *)(APP_FLASH_ADDR+4));
	app_entry_ptr();
	
	//��Ӧ�����е�����
	while(1);	
}

