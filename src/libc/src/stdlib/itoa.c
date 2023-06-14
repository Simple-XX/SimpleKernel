
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

int itoa(int _num, char* _str, int _len, int _base) {
    int sum = _num;
    int i   = 0;
    int digit;
    if (_len == 0) {
        return -1;
    }
    do {
        digit = sum % _base;
        if (digit < 0xA) {
            _str[i++] = '0' + digit;
        }
        else {
            _str[i++] = 'A' + digit - 0xA;
        }
        sum /= _base;
    } while (sum && (i < (_len - 1)));
    if (i == (_len - 1) && sum) {
        return -1;
    }
    _str[i] = '\0';
    strrev(_str);
    return 0;
}
