#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "key.h"
#include "malloc.h" 
#include "w25qxx.h"
#include "ff.h"
#include "string.h"
#include "stmflash.h"
#include "iap.h" 

#define APP_FLASH_ADDR 0x08010000 //64KB

FATFS fs;

int main(void)
{
	u8 t=0;	
	u8 res=0;
	
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
	printf("* V0.1              *\r\n");
	printf("*********************\r\n");
	
 	res=f_mount(&fs,"1:",1); 				//挂载FLASH.	
	if(res==FR_OK)//FLASH磁盘,FAT文件系统正常
	{
		printf("Flash disk OK!\r\n");
	}

	// 判断flash根目录下是否有firmware.bin文件
	FRESULT fres;
	FILINFO fno;
	
	//print_firmware_contents();
	//while(1);
	fres = f_stat("1:/firmware.bin", &fno);
	if (fres == FR_OK) {
		// 文件存在
		printf("firmware.bin found, size: %lu bytes\r\n", fno.fsize);
		printf("开始更新固件...\r\n");
		
		u8 file_buffer[512]={0};
		FIL firmware_file;
		UINT bytes_read;
		uint32_t total_bytes_read = 0;
		uint32_t flash_addr = APP_FLASH_ADDR;

		// 打开固件文件
		fres = f_open(&firmware_file, "1:/firmware.bin", FA_READ);
		
		if (fres == FR_OK) {
			// 循环读取文件内容
			while (total_bytes_read < fno.fsize) {
					// 读取数据块到缓冲区
					fres = f_read(&firmware_file, file_buffer, 512, &bytes_read);
					if (fres != FR_OK || bytes_read == 0) {
							// 读取出错或到达文件末尾
							printf("读取文件失败或文件已结束，错误码: %d\r\n", fres);
							break;
					}
					// 写入到FLASH
					printf("正在写入地址 0x%08X，大小: %u 字节\r\n", flash_addr, bytes_read);
					iap_write_appbin(flash_addr, file_buffer, bytes_read);

					
					// 更新计数器和地址
					total_bytes_read += bytes_read;
					flash_addr += bytes_read;
					
					// 显示进度
					printf("更新进度: %lu/%lu bytes (%.1f%%)\r\n", 
								 total_bytes_read, fno.fsize, 
								 (float)total_bytes_read * 100.0f / fno.fsize);
			}
    
			// 关闭文件
			f_close(&firmware_file);
			
			if (total_bytes_read == fno.fsize) {
					printf("固件更新完成! 共写入 %lu 字节\r\n", total_bytes_read);
					//固件更新完成后删除firmware.bin文件
					fres = f_unlink("1:/firmware.bin");
					if (fres == FR_OK) {
							printf("firmware.bin文件删除成功\r\n");
					} else {
							printf("删除firmware.bin文件失败，错误码: %d\r\n", fres);
					}
			} else {
					printf("固件更新未完成! 已写入 %lu/%lu 字节\r\n", total_bytes_read, fno.fsize);
			}
		}
	}	
	else {
			// 文件不存在
			printf("firmware.bin not found, error code: %d\r\n", fres);
	}
	
	__set_MSP(*(uint32_t *)APP_FLASH_ADDR); //设置栈指针
	void (*app_entry_ptr)(void);
	app_entry_ptr = (void (*)(void))(*(uint32_t *)(APP_FLASH_ADDR+4));
	app_entry_ptr();
	
	while(1)
	{
		t++; 
		delay_ms(200);		 			   
		LED0=!LED0;
	}	
}

