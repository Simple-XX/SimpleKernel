
// This file is apart of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).

// string.c for MRNIU/SimpleKernel.

#include "stdint.h"
#include "stddef.h"
#include "stdarg.h"

#ifndef _STRING_H
#define _STRING_H

// 获取字符串长度
size_t strlen(const char* str){
	size_t len = 0;
	while (str[len])
		len++;
	return len;
}


#endif
