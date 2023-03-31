
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

#include "stdint.h"

int       abs(int);
int       atoi(const char *);
long      atol(const char *);
long long atoll(const char *);
int       itoa(int num, char *str, int len, int base);
long      strtol(const char *nptr, char **endptr, int base);
long long strtoll(const char *nptr, char **endptr, int base);

#ifdef __cplusplus
}
#endif

#endif /* SIMPLEKERNEL_STDLIB_H */
