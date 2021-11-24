
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
// Based on libgcc
// math.h for Simple-XX/SimpleKernel.

#ifndef _MATH_H_
#define _MATH_H_

#ifdef __cplusplus
extern "C" {
#endif

// TODO: 更多支持
// TODO: 浮点数

long long          divmoddi4(long long num, long long den, long long *rem_p);
unsigned long long udivmoddi4(unsigned long long num, unsigned long long den,
                              unsigned long long *rem_p);
unsigned long long udivdi3(unsigned long long num, unsigned long long den);
unsigned long long umoddi3(unsigned long long num, unsigned long long den);

#ifdef __cplusplus
}
#endif

#endif /* _MATH_H_ */
