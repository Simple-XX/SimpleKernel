
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

namespace std {
    class type_info {

    private:
        const char *tname;

    public:
        virtual ~type_info(void);

        type_info(const type_info &);

        explicit type_info(const char *);

        const char *name(void) const;

        bool operator==(const type_info &) const;

        bool operator!=(const type_info &) const;
    };
}

namespace __cxxabiv1 {

#define ADD_CXX_TYPEINFO_HEADER(t)                                             \
    class t : public std::type_info {                                          \
    private:                                                                   \
    protected:                                                                 \
    public:                                                                    \
        explicit t(const char *);                                              \
        virtual ~t(void);                                                      \
    }

    ADD_CXX_TYPEINFO_HEADER(__fundamental_type_info);
    ADD_CXX_TYPEINFO_HEADER(__array_type_info);
    ADD_CXX_TYPEINFO_HEADER(__function_type_info);
    ADD_CXX_TYPEINFO_HEADER(__enum_type_info);
    ADD_CXX_TYPEINFO_HEADER(__pbase_type_info);
    ADD_CXX_TYPEINFO_HEADER(__pointer_type_info);
    ADD_CXX_TYPEINFO_HEADER(__pointer_to_member_type_info);
    ADD_CXX_TYPEINFO_HEADER(__class_type_info);
    ADD_CXX_TYPEINFO_HEADER(__si_class_type_info);
    ADD_CXX_TYPEINFO_HEADER(__vmi_class_type_info);

#undef ADD_CXX_TYPEINFO_HEADER
}

#endif /* _CXXABI_H_ */
