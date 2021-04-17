
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// limits.h for Simple-XX/SimpleKernel.

#ifndef _LIMITS_H_
#define _LIMITS_H_

#ifdef __cplusplus
extern "C" {
#endif

// int8_t
#define SCHAR_MIN (-128)
#define SCHAR_MAX 127
// uint8_t
#define UCHAR_MAX 0xFF

// int16_t
#define SHRT_MIN (-32768)
#define SHRT_MAX 32767
// uint16_t
#define USHRT_MAX 0xFFFF

// 4字节 int和unsigned int类型
// int32_t
#define INT_MIN (-2147483647 - 1)
#define INT_MAX 2147483647
// uint32_t
#define UINT_MAX 0xFFFFFFFF

// 32bit 与 64bit 不同
#define LONG_MIN (-2147483647L - 1)
#define LONG_MAX 2147483647L
#define ULONG_MAX 0xffffffffUL

// 8字节 long long int和unsigned long long int类型
#define LLONG_MAX                                                              \
    9223372036854775807i64 /* maximum signed long long int value */
#define LLONG_MIN                                                              \
    (-9223372036854775807i64 - 1) /* minimum signed long long int value */

#ifdef __cplusplus
}
#endif

#endif /* _LIMITS_H_ */
