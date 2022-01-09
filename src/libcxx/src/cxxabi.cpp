
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
// Based on https://wiki.osdev.org/C%2B%2B
// cxxabi.cpp for Simple-XX/SimpleKernel.

#include "cxxabi.h"
#include "new"

#ifdef __cplusplus
extern "C" {
#endif

#define ATEXIT_MAX_FUNCS 128

typedef void (*ctor_t)(void);
extern ctor_t ctors_start[];
extern ctor_t ctors_end[];

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

void cpp_init(void) {
    ctor_t *f;
    for (f = ctors_start; f < ctors_end; f++) {
        (*f)();
    }
    return;
}

void __cxa_pure_virtual(void) {
    return;
}

atexit_func_entry_t __atexit_funcs[ATEXIT_MAX_FUNCS];
uarch_t             __atexit_func_count = 0;

extern void *__dso_handle;

void __cxa_rethrow() {
    return;
}

void _Unwind_Resume() {
    return;
}

void __cxa_begin_catch() {
    return;
}

void __cxa_end_catch() {
    return;
}

void __gxx_personality_v0() {
    return;
}

int __cxa_atexit(void (*f)(void *), void *objptr, void *dso) {
    if (__atexit_func_count >= ATEXIT_MAX_FUNCS) {
        return -1;
    };
    __atexit_funcs[__atexit_func_count].destructor_func = f;
    __atexit_funcs[__atexit_func_count].obj_ptr         = objptr;
    __atexit_funcs[__atexit_func_count].dso_handle      = dso;
    __atexit_func_count++;
    return 0;
};

int __aeabi_atexit(void (*f)(void *), void *objptr, void *dso) {
    if (__atexit_func_count >= ATEXIT_MAX_FUNCS) {
        return -1;
    };
    __atexit_funcs[__atexit_func_count].destructor_func = f;
    __atexit_funcs[__atexit_func_count].obj_ptr         = objptr;
    __atexit_funcs[__atexit_func_count].dso_handle      = dso;
    __atexit_func_count++;
    return 0;
};

void __cxa_finalize(void *f) {
    uarch_t i = __atexit_func_count;
    if (!f) {
        /*
         * According to the Itanium C++ ABI, if __cxa_finalize is called
         *without a function ptr, then it means that we should destroy
         *EVERYTHING MUAHAHAHA!!
         *
         * TODO:
         * Note well, however, that deleting a function from here that
         *contains a __dso_handle means that one link to a shared object
         *file has been terminated. In other words, We should monitor this
         *list (optional, of course), since it tells us how many links to an
         *object file exist at runtime in a particular application. This can
         *be used to tell when a shared object is no longer in use. It is
         *one of many methods, however.
         **/
        // You may insert a prinf() here to tell you whether or not the
        // function gets called. Testing is CRITICAL!
        while (i--) {
            if (__atexit_funcs[i].destructor_func) {
                /* ^^^ That if statement is a safeguard...
                 * To make sure we don't call any entries that have already
                 *been called and unset at runtime. Those will contain a
                 *value of 0, and calling a function with value 0 will cause
                 *undefined behaviour. Remember that linear address 0, in a
                 *non-virtual address space (physical) contains the IVT and
                 *BDA.
                 *
                 * In a virtual environment, the kernel will receive a page
                 *fault, and then probably map in some trash, or a blank
                 *page, or something stupid like that. This will result in
                 *the processor executing trash, and...we don't want that.
                 **/
                (*__atexit_funcs[i].destructor_func)(__atexit_funcs[i].obj_ptr);
            };
        };
        return;
    };

    while (i--) {
        /*
         * The ABI states that multiple calls to the
         *__cxa_finalize(destructor_func_ptr) function should not destroy
         *objects multiple times. Only one call is needed to eliminate
         *multiple entries with the same address.
         *
         * FIXME:
         * This presents the obvious problem: all destructors must be stored
         *in the order they were placed in the list. I.e: the last
         *initialized object's destructor must be first in the list of
         *destructors to be called. But removing a destructor from the list
         *at runtime creates holes in the table with unfilled entries.
         *Remember that the insertion algorithm in __cxa_atexit simply
         *inserts the next destructor at the end of the table. So, we have
         *holes with our current algorithm This function should be modified
         *to move all the destructors above the one currently being called
         *and removed one place down in the list, so as to cover up the
         *hole. Otherwise, whenever a destructor is called and removed, an
         *entire space in the table is wasted.
         **/
        if (__atexit_funcs[i].destructor_func == f) {
            /*
             * Note that in the next line, not every destructor function is
             *a class destructor. It is perfectly legal to register a non
             *class destructor function as a simple cleanup function to be
             *called on program termination, in which case, it would not
             *NEED an object This pointer. A smart programmer may even take
             *advantage of this and register a C function in the table with
             *the address of some structure containing data about what to
             *clean up on exit. In the case of a function that takes no
             *arguments, it will simply be ignore within the function
             *itself. No worries.
             **/
            (*__atexit_funcs[i].destructor_func)(__atexit_funcs[i].obj_ptr);
            __atexit_funcs[i].destructor_func = 0;

            /*
             * Notice that we didn't decrement __atexit_func_count: this is
             *because this algorithm requires patching to deal with the
             *FIXME outlined above.
             **/
        };
    };
};

namespace __cxxabiv1 {

/* guard variables */

/* The ABI requires a 64-bit type.  */
__extension__ typedef int __guard __attribute__((mode(__DI__)));

int  __cxa_guard_acquire(__guard *);
void __cxa_guard_release(__guard *);
void __cxa_guard_abort(__guard *);

int __cxa_guard_acquire(__guard *g) {
    return !*(char *)(g);
}

void __cxa_guard_release(__guard *g) {
    *(char *)g = 1;
}

void __cxa_guard_abort(__guard *) {
    return;
}
} // namespace __cxxabiv1

#ifdef __cplusplus
};
#endif

namespace std {
type_info::type_info(const type_info &arg) : tname(arg.tname) {
    return;
}

type_info::type_info(const char *pname) : tname(pname) {
    return;
}

type_info::~type_info(void) {
    return;
}

const char *type_info::name(void) const {
    return tname;
}

bool type_info::operator==(const type_info &arg) const {
    return tname == arg.tname;
}

bool type_info::operator!=(const type_info &arg) const {
    return tname != arg.tname;
}
} // namespace std

namespace __cxxabiv1 {

#define ADD_CXX_TYPEINFO_SOURCE(t)                                             \
    t::t(const char *n) : std::type_info(n) {                                  \
        return;                                                                \
    }                                                                          \
    t::~t(void) {                                                              \
        return;                                                                \
    }

ADD_CXX_TYPEINFO_SOURCE(__fundamental_type_info)
ADD_CXX_TYPEINFO_SOURCE(__array_type_info)
ADD_CXX_TYPEINFO_SOURCE(__function_type_info)
ADD_CXX_TYPEINFO_SOURCE(__enum_type_info)
ADD_CXX_TYPEINFO_SOURCE(__pbase_type_info)
ADD_CXX_TYPEINFO_SOURCE(__pointer_type_info)
ADD_CXX_TYPEINFO_SOURCE(__pointer_to_member_type_info)
ADD_CXX_TYPEINFO_SOURCE(__class_type_info)
ADD_CXX_TYPEINFO_SOURCE(__si_class_type_info)
ADD_CXX_TYPEINFO_SOURCE(__vmi_class_type_info)

#undef ADD_CXX_TYPEINFO_SOURCE

} // namespace __cxxabiv1
