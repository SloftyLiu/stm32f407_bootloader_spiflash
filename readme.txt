工程说明：
	这个bootloader会通过FATFS读取外部spi flash文件系统根目录下的firmware.bin文件。
	如果firmware.bin存在，就会将该文件复制到芯片内部flash的0x08010000地址(留64kb給bootloader)
	复制成功后，会删除firmware.bin文件
					
					
					
					
					
					
					
					
					
					
					
					