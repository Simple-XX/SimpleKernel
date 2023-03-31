
/**
 * @file itoa.c
 * @brief itoa 定义
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

#include "stdlib.h"
#include "string.h"

int itoa(int num, char *str, int len, int base) {
    int sum = num;
    int i   = 0;
    int digit;
    if (len == 0) {
        return -1;
    }
    do {
        digit = sum % base;
        if (digit < 0xA) {
            str[i++] = '0' + digit;
        }
        else {
            str[i++] = 'A' + digit - 0xA;
        }
        sum /= base;
    } while (sum && (i < (len - 1)));
    if (i == (len - 1) && sum) {
        return -1;
    }
    str[i] = '\0';
    strrev(str);
    return 0;
}
