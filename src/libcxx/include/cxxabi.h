
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
// Based on https://wiki.osdev.org/C%2B%2B
// cxxabi.h for Simple-XX/SimpleKernel.

#ifndef _CXXABI_H_
#define _CXXABI_H_

// See https://gcc.gnu.org/onlinedocs/gccint/Initialization.html for more info

#define ATEXIT_MAX_FUNCS 128

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned uarch_t;

struct atexit_func_entry_t {
    /*
     * Each member is at least 4 bytes large. Such that each entry is
     12bytes.
     * 128 * 12 = 1.5KB exact.
     **/
    void (*destructor_func)(void *);
    void *obj_ptr;
    void *dso_handle;
};

typedef void (*constructor_func)();
extern constructor_func ctors_start[];
extern constructor_func ctors_end[];
// c++ 初始化
void cpp_init(void);

int  __cxa_atexit(void (*f)(void *), void *objptr, void *dso);
void __cxa_finalize(void *f);
void __cxa_pure_virtual();
void __stack_chk_guard_setup();
void __attribute__((noreturn)) __stack_chk_fail();
void _Unwind_Resume();

namespace __cxxabiv1 {
    /* guard variables */

    /* The ABI requires a 64-bit type.  */
    __extension__ typedef int __guard __attribute__((mode(__DI__)));

    extern "C" int  __cxa_guard_acquire(__guard *);
    extern "C" void __cxa_guard_release(__guard *);
    extern "C" void __cxa_guard_abort(__guard *);
}

int  __aeabi_atexit(void (*f)(void *), void *objptr, void *dso);
void __cxa_finalize(void *f);

#ifdef __cplusplus
};
#endif

#endif /* _CXXABI_H_ */
