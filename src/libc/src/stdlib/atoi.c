
/**
 * @file atoi.c
 * @brief atoi 定义
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

#include "stddef.h"
#include "stdlib.h"

int abs(int _i) {
    return _i < 0 ? -_i : _i;
}

int atoi(const char* _str) {
    return (int)strtol(_str, (char**)NULL, 10);
}

long atol(const char* _str) {
    return (long)strtoll(_str, (char**)NULL, 10);
}

long long atoll(const char* _str) {
    return (long long)strtoll(_str, (char**)NULL, 10);
}
