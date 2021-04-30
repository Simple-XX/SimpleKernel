
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// string.h for Simple-XX/SimpleKernel.

#ifndef _STRING_H_
#define _STRING_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"
#include "stddef.h"

extern void memcpy(void *dest, void *src, uint32_t len);
extern void memset(void *dest, uint8_t val, uint32_t len);
extern void bzero(void *dest, uint32_t len);
// 字符串比较
extern int8_t strcmp(const char *src, const char *dest);
extern char * strcpy(char *dest, const char *src);
extern char * strncpy(char *dest, const char *src, uint32_t len);
// 字符串合并
extern char *strcat(char *dest, const char *src);
// length of string
extern size_t strlen(const char *src);
extern void   backspace(char *src);
extern void   append(char *src, char dest);

#ifdef __cplusplus
}
#endif

#endif /* _STRING_H_ */
