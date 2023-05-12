
/**
 * @file stdlib.h
 * @brief stdlib 定义
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

#ifndef SIMPLEKERNEL_STDLIB_H
#define SIMPLEKERNEL_STDLIB_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stddef.h"
#include "stdint.h"

int       abs(int);
int       atoi(const char*);
long      atol(const char*);
long long atoll(const char*);
int       itoa(int _num, char* _str, int _len, int _base);
long      strtol(const char* _nptr, char** _endptr, int _base);
long long strtoll(const char* _nptr, char** _endptr, int _base);

void*     malloc(size_t size);

void      free(void* ptr);

void*     kmalloc(size_t size);

void      kfree(void* ptr);

#ifdef __cplusplus
}
#endif

#endif /* SIMPLEKERNEL_STDLIB_H */
