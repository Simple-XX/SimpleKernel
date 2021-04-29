
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
// Based on libgcc
// math.c for Simple-XX/SimpleKernel.

#ifdef __cplusplus
extern "C" {
#endif

#include "math.h"

long long divmoddi4(long long num, long long den, long long *rem_p) {
    int       minus = 0;
    long long v;

    if (num < 0) {
        num   = -num;
        minus = 1;
    }
    if (den < 0) {
        den = -den;
        minus ^= 1;
    }

    v = udivmoddi4(num, den, (unsigned long long *)rem_p);
    if (minus) {
        v = -v;
        if (rem_p)
            *rem_p = -(*rem_p);
    }

    return v;
}

unsigned long long udivmoddi4(unsigned long long num, unsigned long long den,
                              unsigned long long *rem_p) {
    unsigned long long quot = 0, qbit = 1;

    if (den == 0) {
        return 1 / ((unsigned)den); /* Intentional divide by zero, without
                                       triggering a compiler warning which
                                          would abort the build */
    }

    /* Left-justify denominator and count shift */
    while ((long long)den >= 0) {
        den <<= 1;
        qbit <<= 1;
    }

    while (qbit) {
        if (den <= num) {
            num -= den;
            quot += qbit;
        }
        den >>= 1;
        qbit >>= 1;
    }

    if (rem_p)
        *rem_p = num;

    return quot;
}

unsigned long long udivdi3(unsigned long long num, unsigned long long den) {
    return udivmoddi4(num, den, 0);
}

unsigned long long umoddi3(unsigned long long num, unsigned long long den) {
    unsigned long long v;
    (void)udivmoddi4(num, den, &v);
    return v;
}

#ifdef __cplusplus
}

#endif
