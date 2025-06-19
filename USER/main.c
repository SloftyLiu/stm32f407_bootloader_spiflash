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
#include "usbd_usr.h"
#include "usbd_desc.h"
#include "usb_conf.h"
#include "usbd_msc_bot.h"
#include "usbh_usr.h" 
#include "update.h"
#include "crc.h"
#include "usb_hcd_int.h"
#include "usb_dcd_int.h"

FATFS fs;
FATFS udisk_fs;
USB_OTG_CORE_HANDLE USB_OTG_dev;
USBH_HOST  USB_Host;
USB_OTG_CORE_HANDLE  USB_OTG_Core;

typedef enum {
	USB_ROLE_DEVICE = 0,
	USB_ROLE_HOST,
	USB_ROLE_NONE,
}USBRole_Tpyedef;

USBRole_Tpyedef USBRole = USB_ROLE_NONE;

void OTG_FS_IRQHandler(void)
{
	if(USBRole == USB_ROLE_DEVICE)
  	USBD_OTG_ISR_Handler(&USB_OTG_dev);
	if(USBRole == USB_ROLE_HOST)
		USBH_OTG_ISR_Handler(&USB_OTG_Core);
}  

typedef enum {
	SPIFLASH_UPDATE = 0,
	UDISK_UPDATE,
}Update_Tpyedef;

Update_Tpyedef update_type =  SPIFLASH_UPDATE;

//检查U盘是否存在firmware.bin
//返回值:0,正常
//       1,有问题
u8 USH_User_App(void)
{
	FRESULT res = f_mount(&udisk_fs,"2:",1);
	if(res == 0)
	{
		printf("U disk mount OK\r\n");
		// 判断flash根目录下是否有firmware.bin文件
		FILINFO fno;
		res = f_stat("2:/firmware.bin", &fno);
		if (res == FR_OK) {
			// 文件存在
			printf("firmware.bin found, size: %lu bytes\r\n", fno.fsize);
			update_type = UDISK_UPDATE;
		}	
		else {
				// 文件不存在
				printf("firmware.bin not found!\r\n");
		}
	}
	else
	{
		printf("U disk mount fail!!!!!!!!!res %d\r\n",res);
	}
	return res;
}



int main(void)
{
  HAL_Init();                   	//初始化HAL库    

  Stm32_Clock_Init(336,8,2,7);  	//设置时钟,168Mhz
	delay_init(168);               	//初始化延时函数
	uart_init(115200);             	//初始化USART
	LED_Init();						//初始化LED	
	KEY_Init();						//初始化KEY 
	W25QXX_Init();				    //初始化W25Q256
	my_mem_init(SRAMIN);			//初始化内部内存池
		
	printf("*********************\r\n");
	printf("* Bootloader start! *\r\n");
	printf("* V0.5              *\r\n");
	printf("*********************\r\n");
	
	
	switch( KEY_Scan(0) )
	{
		case KEY0_PRES :
			printf("KEY0 is pressed! Now you can copy the firmware.bin!\r\n");
			USBRole = USB_ROLE_DEVICE;
			USBD_Init(&USB_OTG_dev,USB_OTG_FS_CORE_ID,&USR_desc,&USBD_MSC_cb,&USR_cb_Device);
			while(1);
		case KEY1_PRES :
			USBRole = USB_ROLE_HOST;
			printf("KEY1 is pressed! Now the check firmware.bin in USB disk!\r\n");
		  USBH_Init(&USB_OTG_Core,USB_OTG_FS_CORE_ID,&USB_Host,&USBH_MSC_cb,&USR_cb_Host);  
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
	
	//执行更新流程
	switch(update_type)
	{
		case SPIFLASH_UPDATE:
			spiflash_update();
			break;
		case UDISK_UPDATE:
			udisk_update();
			break;
	}
	
	printf("正在跳转到APP...\r\n");
	void (*app_entry_ptr)(void);
	__set_MSP(*(uint32_t *)APP_FLASH_ADDR); //设置栈指针
	app_entry_ptr = (void (*)(void))(*(uint32_t *)(APP_FLASH_ADDR+4));
	app_entry_ptr();
	
	//不应该运行到这里
	while(1);	
}

