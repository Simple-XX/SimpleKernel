
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// stdint.h for Simple-XX/SimpleKernel.

#ifndef _STDINT_H_
#define _STDINT_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef signed char int8_t;
typedef short int   int16_t;
typedef int         int32_t;
// typedef long int32_t;
typedef long long int64_t;

typedef unsigned char      uint8_t;
typedef unsigned short int uint16_t;
typedef unsigned int       uint32_t;
// typedef unsigned long uint32_t;
typedef unsigned long long uint64_t;

#ifdef X8664
typedef uint64_t addr_t;
#else
typedef uint32_t addr_t;
#endif

#ifdef RASPI2
typedef uint32_t addr_t;
#endif

#ifdef __cplusplus
}
#endif

#endif /* _STDINT_H_ */
