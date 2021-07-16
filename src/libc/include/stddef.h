
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// stddef.h for Simple-XX/SimpleKernel.

#ifndef _STDDEF_H_
#define _STDDEF_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _PTRDIFF_T
#define _PTRDIFF_T
typedef long ptrdiff_t;
#endif

#ifndef _SIZE_T
#define _SIZE_T
#undef size_t
#if defined(__i386__)
typedef unsigned int size_t;
#elif defined(__riscv) || defined(__x86_64__)
typedef long unsigned int size_t;
#endif
#endif

#ifndef _SSIZE_T
#define _SSIZE_T
#undef ssize_t
#if defined(__i386__)
typedef int ssize_t;
#elif defined(__riscv) || defined(__x86_64__)
typedef long int          ssize_t;
#endif
#endif

#ifdef __cplusplus
}
#endif

#endif /* _STDDEF_H_ */
