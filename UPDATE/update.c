#include "ff.h"
#include "update.h"
#include "stmflash.h"
#include "iap.h" 
#include "crc.h"
#include <stdio.h>
#include <stdlib.h>

extern FATFS sdcard_fs;
extern FATFS fs;
extern FATFS udisk_fs;

UpdateResult_Tpyedef sdcard_update()
{
	u8 res=0;
	res=f_mount(&sdcard_fs,"0:",1); 				//挂载FLASH.	
	if(res==FR_OK)//FLASH磁盘,FAT文件系统正常
	{
		printf("Flash disk OK!\r\n");
	}

	FRESULT fres;
	FILINFO fno;
	
	fres = f_stat("0:/firmware.bin", &fno);
	if (fres != FR_OK) {
		// 文件不存在
		printf("firmware.bin not found!\r\n");
		return UPDATE_NO_NEED;
	}
	
	fres = f_stat("0:/crc32.txt", &fno);
	if (fres != FR_OK) {
		// 文件不存在
		printf("crc32.txt not found!\r\n");
		return UPDATE_NO_NEED;
	}
	
	printf("firmware.bin found, size: %lu bytes\r\n", fno.fsize);
	printf("开始更新固件...\r\n");
	
	u8 file_buffer[512]={0};
	FIL firmware_file, crc_file;
	UINT bytes_read;
	unsigned long total_bytes_read = 0;
	uint32_t flash_addr = APP_FLASH_ADDR;

	//校验固件
	f_open(&crc_file, "0:/crc32.txt", FA_READ);
	f_read(&crc_file, file_buffer, 10, &bytes_read);
	uint32_t crc_value_in_file = 0;
	sscanf((const char *)file_buffer, "0x%X", &crc_value_in_file);
	if(crc_value_in_file == CRC32_file_check("0:/firmware.bin"))
	{
		printf("firmware.bin check OK!\r\n");
	}
	else
	{
		printf("firmware.bin check Fail!\r\n");
		return UPDATE_FAIL;
	}
	
	// 打开固件文件
	f_open(&firmware_file, "0:/firmware.bin", FA_READ);
	f_stat("0:/firmware.bin", &fno);
	
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
			printf("更新进度: %lu/%lu bytes\r\n", 
						 total_bytes_read, fno.fsize);
	}

	// 关闭文件
	f_close(&firmware_file);
	
	//校验Flash中的固件
	if(crc_value_in_file == CRC32_calcu_software((const unsigned char *)APP_FLASH_ADDR, fno.fsize, 0x0))
	{
		printf("Flash check OK!\r\n");
	}
	else
	{
		printf("Flash check Fail!\r\n");
		return UPDATE_FAIL;
	}
	
	if (total_bytes_read == fno.fsize) {
			printf("固件更新完成! 共写入 %lu 字节\r\n", total_bytes_read);
		
	} else {
			printf("固件更新未完成! 已写入 %lu/%lu 字节\r\n", total_bytes_read, fno.fsize);
	}
	return UPDATE_OK;
}



UpdateResult_Tpyedef spiflash_update()
{
	u8 res=0;
	res=f_mount(&fs,"1:",1); 				//挂载FLASH.	
	if(res==FR_OK)//FLASH磁盘,FAT文件系统正常
	{
		printf("Flash disk OK!\r\n");
	}

	// 判断flash根目录下是否有firmware.bin文件
	FRESULT fres;
	FILINFO fno;
	
	fres = f_stat("1:/firmware.bin", &fno);
	if (fres == FR_OK) {
		// 文件存在
		printf("firmware.bin found, size: %lu bytes\r\n", fno.fsize);
		printf("开始更新固件...\r\n");
		
		u8 file_buffer[512]={0};
		FIL firmware_file;
		UINT bytes_read;
		unsigned long total_bytes_read = 0;
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
					printf("更新进度: %lu/%lu bytes\r\n", 
								 total_bytes_read, fno.fsize);
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
		printf("firmware.bin not found!\r\n");
		return UPDATE_NO_NEED;
	}
}


UpdateResult_Tpyedef udisk_update(void)
{
	FRESULT fres;
	FILINFO fno;
	
	fres = f_stat("2:/firmware.bin", &fno);
	if (fres == FR_OK) {
		printf("开始更新固件...\r\n");
		
		u8 file_buffer[512]={0};
		FIL firmware_file;
		UINT bytes_read;
		unsigned long total_bytes_read = 0;
		uint32_t flash_addr = APP_FLASH_ADDR;

		// 打开固件文件
		fres = f_open(&firmware_file, "2:/firmware.bin", FA_READ);
		
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
					printf("更新进度: %lu/%lu bytes\r\n", total_bytes_read, fno.fsize);
			}
		
			// 关闭文件
			f_close(&firmware_file);
			
			if (total_bytes_read == fno.fsize) {
					printf("固件更新完成! 共写入 %lu 字节\r\n", total_bytes_read);
			} else {
					printf("固件更新未完成! 已写入 %lu/%lu 字节\r\n", total_bytes_read, fno.fsize);
			}
		} else {
			// 文件不存在
			printf("firmware.bin not found!\r\n");
			return UPDATE_NO_NEED;
		}
	}
}
