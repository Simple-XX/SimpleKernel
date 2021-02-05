
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// math.h for Simple-XX/SimpleKernel.

#ifndef _MATH_H_
#define _MATH_H_

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__arm__) || defined(__aarch64__)

unsigned long udivmodsi4(unsigned long num, unsigned long den, int modwanted);
long          divsi3(long a, long b);
long          modsi3(long a, long b);

#endif

#ifdef __cplusplus
}
#endif

#endif /* _MATH_H_ */
