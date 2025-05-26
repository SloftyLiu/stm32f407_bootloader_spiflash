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
#include "key.h"
#include "usbd_msc_core.h"
#include "usbd_usr.h"
#include "usbd_desc.h"
#include "usb_conf.h"
#include "usbd_msc_bot.h"

#define APP_FLASH_ADDR 0x08010000 //64KB

FATFS fs;

USB_OTG_CORE_HANDLE USB_OTG_dev;

void do_update()
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
					printf("���½���: %lu/%lu bytes (%.1f%%)\r\n", 
								 total_bytes_read, fno.fsize, 
								 (float)total_bytes_read * 100.0f / fno.fsize);
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
	printf("* V0.2              *\r\n");
	printf("*********************\r\n");
	
	
	if( KEY0_PRES == KEY_Scan(0) )
	{
		printf("KEY0 is pressed! Now you can copy the firmware.bin!\r\n");
		USBD_Init(&USB_OTG_dev,USB_OTG_FS_CORE_ID,&USR_desc,&USBD_MSC_cb,&USR_cb);
		while(1);
	}
	
	//ִ�и�������
	do_update();
	
	printf("������ת��APP...\r\n");
	void (*app_entry_ptr)(void);
	__set_MSP(*(uint32_t *)APP_FLASH_ADDR); //����ջָ��
	app_entry_ptr = (void (*)(void))(*(uint32_t *)(APP_FLASH_ADDR+4));
	app_entry_ptr();
	
	//��Ӧ�����е�����
	while(1);	
}

