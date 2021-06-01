
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

extern void *memcpy(void *dest, const void *src, size_t len);
extern void *memset(void *dest, int val, size_t len);
extern int   memcmp(const void *str1, const void *str2, size_t count);
extern void *memmove(void *dest, const void *src, size_t len);
extern void  bzero(void *dest, uint32_t len);
// 字符串比较
extern int   strcmp(const char *src, const char *dest);
extern int   strncmp(const char *s1, const char *s2, size_t n);
extern char *strcpy(char *dest, const char *src);
extern char *strncpy(char *s1, const char *s2, size_t n);
// 字符串合并
extern char *strcat(char *dest, const char *src);
extern char *strchr(const char *s, int c);
// length of string
extern size_t strlen(const char *src);
extern size_t strnlen(const char *str, size_t maxlen);
extern void   backspace(char *src);
extern void   append(char *src, char dest);

#ifdef __cplusplus
}
#endif

#endif /* _STRING_H_ */
