
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
//
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
typedef unsigned int size_t;
#endif

#undef NULL
#define NULL ( (void *)0)

#define UNUSED __attribute__( (unused) )


#ifdef __cplusplus
}
#endif

#endif /* _STDDEF_H_ */
