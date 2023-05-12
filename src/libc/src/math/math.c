
/**
 * @file math.c
 * @brief math 定义
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2023-03-31
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * Based on libgcc
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2023-03-31<td>Zone.N<td>迁移到 doxygen
 * </table>
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "math.h"

long long divmoddi4(long long _num, long long _den, long long* _rem_p) {
    int       minus = 0;
    long long v;

    if (_num < 0) {
        _num  = -_num;
        minus = 1;
    }
    if (_den < 0) {
        _den   = -_den;
        minus ^= 1;
    }

    v = udivmoddi4(_num, _den, (unsigned long long*)_rem_p);
    if (minus) {
        v = -v;
        if (_rem_p) {
            *_rem_p = -(*_rem_p);
        }
    }

    return v;
}

// BUG: 在 x86_64 下观察到了精度损失
unsigned long long udivmoddi4(unsigned long long _num, unsigned long long _den,
                              unsigned long long* _rem_p) {
    unsigned long long quot = 0, qbit = 1;

    if (_den == 0) {
        return 1 / ((unsigned)_den); /* Intentional divide by zero, without
                                       triggering a compiler warning which
                                          would abort the build */
    }

    /* Left-justify denominator and count shift */
    while ((long long)_den >= 0) {
        _den <<= 1;
        qbit <<= 1;
    }

    while (qbit) {
        if (_den <= _num) {
            _num -= _den;
            quot += qbit;
        }
        _den >>= 1;
        qbit >>= 1;
    }

    if (_rem_p) {
        *_rem_p = _num;
    }
    return quot;
}

unsigned long long udivdi3(unsigned long long _num, unsigned long long _den) {
    return udivmoddi4(_num, _den, 0);
}

unsigned long long umoddi3(unsigned long long _num, unsigned long long _den) {
    unsigned long long v;
    (void)udivmoddi4(_num, _den, &v);
    return v;
}

#ifdef __cplusplus
}
#endif
