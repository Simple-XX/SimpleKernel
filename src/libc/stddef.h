
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).

// stddef.h for MRNIU/SimpleKernel.


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
typedef unsigned long size_t;
#endif

#undef NULL
#define NULL ( (void *)0 )

#define UNUSED(x) (void)( x )

#ifdef __cplusplus
}
#endif

#endif
