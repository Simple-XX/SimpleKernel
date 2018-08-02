
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).

// string.c for MRNIU/SimpleKernel.

#include "stddef.h"
#include "stdint.h"


// 获取字符串长度
size_t strlen(const char* str){
	size_t len = 0;
	while(str[len])
		len++;
	return len;
}


// 如果 src > dest, 则返回值大于 0，如果 src = dest, 则返回值等于 0，
// 如果 srd  < dest ,则返回值小于 0。
int8_t strcmp(const char * src, const char * dest){
	while (*src && *dest && (*src == *dest)){
		src ++;
		dest ++;
	}
	return *src - *dest;
}

char *strcpy(char *dest, const char *src){
	char *address = dest;
	while((*dest++ = *src++) != '\0');
	return address;
}
