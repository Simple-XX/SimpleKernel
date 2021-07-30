
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
#define SCHAR_MAX (127)
// uint8_t
#define UCHAR_MAX (0xFF)

// int16_t
#define SHRT_MIN (-32768)
#define SHRT_MAX (32767)
// uint16_t
#define USHRT_MAX (0xFFFF)

// 4字节 int和unsigned int类型
// int32_t
#define INT_MIN (-2147483647 - 1)
#define INT_MAX (2147483647)
// uint32_t
#define UINT_MAX (0xFFFFFFFF)

// 32bit 与 64bit 不同
// 0x7FFFFFFF
#define LONG_MAX ((long)(ULONG_MAX >> 1))
// 0x80000000
#define LONG_MIN ((long)(~LONG_MAX))
// 0xFFFFFFFF
#define ULONG_MAX ((unsigned long)(~0L))

// 8字节 long long int和 unsigned long long int类型
// 0xFFFFFFFFFFFFFFFF
#define ULLONG_MAX (~(unsigned long long)0)
// 0x7FFFFFFFFFFFFFFF
#define LLONG_MAX ((long long)(ULLONG_MAX >> 1))
// 0x8000000000000000
#define LLONG_MIN (~LLONG_MAX)

#ifdef __cplusplus
}
#endif

#endif /* _LIMITS_H_ */
