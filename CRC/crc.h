#ifndef _CRC_H
#define _CRC_H
#include "sys.h"

uint32_t CRC32_calcu_software(const unsigned char *buf, int len, unsigned int init);
void CRC_test(void);
uint32_t CRC32_file_check(const char *path);

#endif
