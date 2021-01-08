
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
// Based on https://wiki.osdev.org/C%2B%2B
// cxxabi.h for Simple-XX/SimpleKernel.

#ifndef _ICXXABI_H_
#define _ICXXABI_H_

#define ATEXIT_MAX_FUNCS 128

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned uarch_t;

typedef struct atexit_func_entry {
    /*
     * Each member is at least 4 bytes large. Such that each entry is 12bytes.
     * 128 * 12 = 1.5KB exact.
     **/
    void (*destructor_func)(void *);
    void *obj_ptr;
    void *dso_handle;
} atexit_func_entry_t;

int  __cxa_atexit(void (*f)(void *), void *objptr, void *dso);
void __cxa_finalize(void *f);

#ifdef __cplusplus
};
#endif

#endif