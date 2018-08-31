
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).

// string.h for MRNIU/SimpleKernel.

#ifndef _STRING_H_
#define _STRING_H_

#include "stdint.h"

extern void memcpy(void *dest, void *src, uint32_t len);
extern void memset(void *dest, uint8_t val, uint32_t len);
extern void bzero(void *dest, uint32_t len);
extern int8_t strcmp(const char *src, const char *dest); // 字符串比较
extern char *strcpy(char *dest, const char *src);
extern char *strncpy(char *dest, const char *src, uint32_t len);
extern char *strcat(char *dest, const char *src); // 字符串合并
extern size_t strlen(const char *src);  // length of string
extern void backspace(char *src);
extern void append(char *src, char dest);



#endif
