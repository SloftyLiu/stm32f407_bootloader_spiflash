#ifndef _CRC_H
#define _CRC_H
#include "sys.h"

uint32_t CRC32Software(const unsigned char *buf, int len, unsigned int init);

#endif
