
/**
 * @file string.h
 * @brief string 定义
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2023-05-08
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2023-05-08<td>Zone.N<td>创建文件
 * </table>
 */

#ifndef SIMPLEKERNEL_SRC_KERNEL_LIBC_INCLUDE_STRING_H_
#define SIMPLEKERNEL_SRC_KERNEL_LIBC_INCLUDE_STRING_H_

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// 复制内存块
void *memcpy(void *dest, const void *src, size_t n);

// 复制内存块，可以处理重叠区域。
void *memmove(void *dest, const void *src, size_t n);

// 设置内存块
void *memset(void *dest, int val, size_t n);

// 比较内存块
int memcmp(const void *str1, const void *str2, size_t n);

// 复制字符串
char *strcpy(char *dest, const char *src);

// 复制指定长度的字符串
char *strncpy(char *dest, const char *src, size_t n);

// 连接字符串
char *strcat(char *dest, const char *src);

// 比较字符串
int strcmp(const char *s1, const char *s2);

// 比较指定长度的字符串
int strncmp(const char *s1, const char *s2, size_t n);

// 获取字符串长度
size_t strlen(const char *s);

// 获取指定字符串长度
size_t strnlen(const char *s, size_t n);

// 查找字符在字符串中的首次出现
char *strchr(const char *s, int c);

#ifdef __cplusplus
}
#endif

#endif /* SIMPLEKERNEL_SRC_KERNEL_LIBC_INCLUDE_STRING_H_ */
