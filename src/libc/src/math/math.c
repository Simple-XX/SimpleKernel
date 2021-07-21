
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

unsigned long long udivmoddi4(unsigned long long n, unsigned long long d,
                              unsigned long long *rp) {
    unsigned long long q = 0, r = n, y = d;
    unsigned int       lz1, lz2, i, k;

    /*
     * Implements align divisor shift dividend method. This algorithm
     * aligns the divisor under the dividend and then perform number of
     * test-subtract iterations which shift the dividend left. Number of
     * iterations is k + 1 where k is the number of bit positions the
     * divisor must be shifted left  to align it under the dividend.
     * quotient bits can be saved in the rightmost positions of the
     * dividend as it shifts left on each test-subtract iteration.
     */

    if (y <= r) {
        lz1 = __builtin_clzll(d);
        lz2 = __builtin_clzll(n);

        k = lz1 - lz2;
        y = (y << k);

        /*
         * Dividend can exceed 2 ^ (width - 1) - 1 but still be less
         * than the aligned divisor. Normal iteration can drops the
         * high order bit of the dividend. Therefore, first
         * test-subtract iteration is a special case, saving its
         * quotient bit in a separate location and not shifting
         * the dividend.
         */

        if (r >= y) {
            r = r - y;
            q = (1ULL << k);
        }

        if (k > 0) {
            { y = y >> 1; }

            /*
             * k additional iterations where k regular test
             * subtract shift dividend iterations are done.
             */
            i = k;
            do {
                if (r >= y)
                    r = ((r - y) << 1) + 1;
                else
                    r = (r << 1);
                i = i - 1;
            } while (i != 0);

            /*
             * First quotient bit is combined with the quotient
             * bits resulting from the k regular iterations.
             */
            q = q + r;
            r = r >> k;
            q = q - (r << k);
        }
    }

    if (rp) {
        *rp = r;
    }
    return q;
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
