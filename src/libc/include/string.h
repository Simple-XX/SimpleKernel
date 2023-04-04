
/**
 * @file string.h
 * @brief string 定义
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2023-03-31
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2023-03-31<td>Zone.N<td>迁移到 doxygen
 * </table>
 */

#ifndef SIMPLEKERNEL_STRING_H
#define SIMPLEKERNEL_STRING_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"
#include "stddef.h"

extern void *memcpy(void *_dest, const void *_src, size_t _len);
extern void *memset(void *_dest, int val, size_t _len);
extern int   memcmp(const void *_str1, const void *_str2, size_t _count);
extern void *memmove(void *_dest, const void *_src, size_t _len);
extern void  bzero(void *_dest, uint32_t _len);
// 字符串比较
extern int   strcmp(const char *_src, const char *_dest);
extern int   strncmp(const char *_s1, const char *_s2, size_t _n);
extern char *strcpy(char *_dest, const char *_src);
extern char *strncpy(char *_s1, const char *_s2, size_t _n);
// 字符串合并
extern char *strcat(char *_dest, const char *_src);
extern char *strchr(const char *_s, int _c);
// length of string
extern size_t strlen(const char *_src);
extern size_t strnlen(const char *_str, size_t _maxlen);
extern void   backspace(char *_src);
extern void   append(char *_src, char _dest);
extern void   strrev(char *_str);

#ifdef __cplusplus
}
#endif

#endif /* SIMPLEKERNEL_STRING_H */
