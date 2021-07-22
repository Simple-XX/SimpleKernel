
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
// Based on https://wiki.osdev.org/C%2B%2B
// cxxabi.h for Simple-XX/SimpleKernel.

#ifndef _CXXABI_H_
#define _CXXABI_H_

#ifdef __cplusplus
extern "C" {
#endif

// c++ 初始化
void cpp_init(void);

int  __cxa_atexit(void (*f)(void *), void *objptr, void *dso);
void __cxa_finalize(void *f);

int  __aeabi_atexit(void (*f)(void *), void *objptr, void *dso);
void __cxa_finalize(void *f);

#ifdef __cplusplus
};
#endif

#endif /* _CXXABI_H_ */
