
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
// Based on libgcc
// math.c for Simple-XX/SimpleKernel.

#ifdef __cplusplus
extern "C" {
#endif

#ifdef RASPI2

#include "math.h"

unsigned long udivmodsi4(unsigned long num, unsigned long den, int modwanted) {
    unsigned long bit = 1;
    unsigned long res = 0;
    while (den < num && bit && !(den & (1L << 31))) {
        den <<= 1;
        bit <<= 1;
    }
    while (bit) {
        if (num >= den) {
            num -= den;
            res |= bit;
        }
        bit >>= 1;
        den >>= 1;
    }
    if (modwanted) {
        return num;
    }
    return res;
}

long divsi3(long a, long b) {
    int  neg = 0;
    long res;
    if (a < 0) {
        a   = -a;
        neg = !neg;
    }
    if (b < 0) {
        b   = -b;
        neg = !neg;
    }
    res = udivmodsi4(a, b, 0);
    if (neg) {
        res = -res;
    }
    return res;
}

long modsi3(long a, long b) {
    int  neg = 0;
    long res;
    if (a < 0) {
        a   = -a;
        neg = 1;
    }
    if (b < 0) {
        b = -b;
    }
    res = udivmodsi4(a, b, 1);
    if (neg) {
        res = -res;
    }
    return res;
}

#endif

#ifdef __cplusplus
}
#endif