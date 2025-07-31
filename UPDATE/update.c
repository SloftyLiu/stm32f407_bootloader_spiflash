#include "ff.h"
#include "update.h"
#include "stmflash.h"
#include "iap.h" 

extern FATFS sdcard_fs;
extern FATFS fs;
extern FATFS udisk_fs;

void sdcard_update()
{
	u8 res=0;
	res=f_mount(&sdcard_fs,"0:",1); 				//����FLASH.	
	if(res==FR_OK)//FLASH����,FAT�ļ�ϵͳ����
	{
		printf("Flash disk OK!\r\n");
	}

	// �ж�flash��Ŀ¼���Ƿ���firmware.bin�ļ�
	FRESULT fres;
	FILINFO fno;
	
	fres = f_stat("0:/firmware.bin", &fno);
	if (fres == FR_OK) {
		// �ļ�����
		printf("firmware.bin found, size: %lu bytes\r\n", fno.fsize);
		printf("��ʼ���¹̼�...\r\n");
		
		u8 file_buffer[512]={0};
		FIL firmware_file;
		UINT bytes_read;
		unsigned long total_bytes_read = 0;
		uint32_t flash_addr = APP_FLASH_ADDR;

		// �򿪹̼��ļ�
		fres = f_open(&firmware_file, "0:/firmware.bin", FA_READ);
		
		if (fres == FR_OK) {
			// ѭ����ȡ�ļ�����
			while (total_bytes_read < fno.fsize) {
					// ��ȡ���ݿ鵽������
					fres = f_read(&firmware_file, file_buffer, 512, &bytes_read);
					if (fres != FR_OK || bytes_read == 0) {
							// ��ȡ����򵽴��ļ�ĩβ
							printf("��ȡ�ļ�ʧ�ܻ��ļ��ѽ�����������: %d\r\n", fres);
							break;
					}
					// д�뵽FLASH
					printf("����д���ַ 0x%08X����С: %u �ֽ�\r\n", flash_addr, bytes_read);
					iap_write_appbin(flash_addr, file_buffer, bytes_read);

					
					// ���¼������͵�ַ
					total_bytes_read += bytes_read;
					flash_addr += bytes_read;
					
					// ��ʾ����
					printf("���½���: %lu/%lu bytes\r\n", 
								 total_bytes_read, fno.fsize);
			}
		
			// �ر��ļ�
			f_close(&firmware_file);
			
			if (total_bytes_read == fno.fsize) {
					printf("�̼��������! ��д�� %lu �ֽ�\r\n", total_bytes_read);
					//�̼�������ɺ�ɾ��firmware.bin�ļ�
					fres = f_unlink("0:/firmware.bin");
					if (fres == FR_OK) {
							printf("firmware.bin�ļ�ɾ���ɹ�\r\n");
					} else {
							printf("ɾ��firmware.bin�ļ�ʧ�ܣ�������: %d\r\n", fres);
					}
			} else {
					printf("�̼�����δ���! ��д�� %lu/%lu �ֽ�\r\n", total_bytes_read, fno.fsize);
			}
		}
	}	
	else {
			// �ļ�������
			printf("firmware.bin not found!\r\n");
	}
}



void spiflash_update()
{
	u8 res=0;
	res=f_mount(&fs,"1:",1); 				//����FLASH.	
	if(res==FR_OK)//FLASH����,FAT�ļ�ϵͳ����
	{
		printf("Flash disk OK!\r\n");
	}

	// �ж�flash��Ŀ¼���Ƿ���firmware.bin�ļ�
	FRESULT fres;
	FILINFO fno;
	
	fres = f_stat("1:/firmware.bin", &fno);
	if (fres == FR_OK) {
		// �ļ�����
		printf("firmware.bin found, size: %lu bytes\r\n", fno.fsize);
		printf("��ʼ���¹̼�...\r\n");
		
		u8 file_buffer[512]={0};
		FIL firmware_file;
		UINT bytes_read;
		unsigned long total_bytes_read = 0;
		uint32_t flash_addr = APP_FLASH_ADDR;

		// �򿪹̼��ļ�
		fres = f_open(&firmware_file, "1:/firmware.bin", FA_READ);
		
		if (fres == FR_OK) {
			// ѭ����ȡ�ļ�����
			while (total_bytes_read < fno.fsize) {
					// ��ȡ���ݿ鵽������
					fres = f_read(&firmware_file, file_buffer, 512, &bytes_read);
					if (fres != FR_OK || bytes_read == 0) {
							// ��ȡ����򵽴��ļ�ĩβ
							printf("��ȡ�ļ�ʧ�ܻ��ļ��ѽ�����������: %d\r\n", fres);
							break;
					}
					// д�뵽FLASH
					printf("����д���ַ 0x%08X����С: %u �ֽ�\r\n", flash_addr, bytes_read);
					iap_write_appbin(flash_addr, file_buffer, bytes_read);

					
					// ���¼������͵�ַ
					total_bytes_read += bytes_read;
					flash_addr += bytes_read;
					
					// ��ʾ����
					printf("���½���: %lu/%lu bytes\r\n", 
								 total_bytes_read, fno.fsize);
			}
		
			// �ر��ļ�
			f_close(&firmware_file);
			
			if (total_bytes_read == fno.fsize) {
					printf("�̼��������! ��д�� %lu �ֽ�\r\n", total_bytes_read);
					//�̼�������ɺ�ɾ��firmware.bin�ļ�
					fres = f_unlink("1:/firmware.bin");
					if (fres == FR_OK) {
							printf("firmware.bin�ļ�ɾ���ɹ�\r\n");
					} else {
							printf("ɾ��firmware.bin�ļ�ʧ�ܣ�������: %d\r\n", fres);
					}
			} else {
					printf("�̼�����δ���! ��д�� %lu/%lu �ֽ�\r\n", total_bytes_read, fno.fsize);
			}
		}
	}	
	else {
			// �ļ�������
			printf("firmware.bin not found!\r\n");
	}
}


void udisk_update(void)
{
	FRESULT fres;
	FILINFO fno;
	
	fres = f_stat("2:/firmware.bin", &fno);
	if (fres == FR_OK) {
		printf("��ʼ���¹̼�...\r\n");
		
		u8 file_buffer[512]={0};
		FIL firmware_file;
		UINT bytes_read;
		unsigned long total_bytes_read = 0;
		uint32_t flash_addr = APP_FLASH_ADDR;

		// �򿪹̼��ļ�
		fres = f_open(&firmware_file, "2:/firmware.bin", FA_READ);
		
		if (fres == FR_OK) {
			// ѭ����ȡ�ļ�����
			while (total_bytes_read < fno.fsize) {
					// ��ȡ���ݿ鵽������
					fres = f_read(&firmware_file, file_buffer, 512, &bytes_read);
					if (fres != FR_OK || bytes_read == 0) {
							// ��ȡ����򵽴��ļ�ĩβ
							printf("��ȡ�ļ�ʧ�ܻ��ļ��ѽ�����������: %d\r\n", fres);
							break;
					}
					// д�뵽FLASH
					printf("����д���ַ 0x%08X����С: %u �ֽ�\r\n", flash_addr, bytes_read);
					iap_write_appbin(flash_addr, file_buffer, bytes_read);

					
					// ���¼������͵�ַ
					total_bytes_read += bytes_read;
					flash_addr += bytes_read;
					
					// ��ʾ����
					printf("���½���: %lu/%lu bytes\r\n", total_bytes_read, fno.fsize);
			}
		
			// �ر��ļ�
			f_close(&firmware_file);
			
			if (total_bytes_read == fno.fsize) {
					printf("�̼��������! ��д�� %lu �ֽ�\r\n", total_bytes_read);
			} else {
					printf("�̼�����δ���! ��д�� %lu/%lu �ֽ�\r\n", total_bytes_read, fno.fsize);
			}
		}
	}
}
