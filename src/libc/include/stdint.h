
/**
 * @file stdint.h
 * @brief stdint 定义
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2023-03-31
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * Based on GNU C Lib
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2023-03-31<td>Zone.N<td>迁移到 doxygen
 * </table>
 */

#ifndef SIMPLEKERNEL_STDINT_H
#define SIMPLEKERNEL_STDINT_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__x86_64__) || defined(__riscv)
#    define __WORDSIZE 64
#else
#    define __WORDSIZE 32
#endif

#ifdef __x86_64__
#    define __WORDSIZE_TIME64_COMPAT32 1
/* Both x86-64 and x32 use the 64-bit system call interface.  */
#    define __SYSCALL_WORDSIZE         64
#endif

/* Exact integral types.  */

/* Signed.  */

/* There is some amount of overlap with <sys/types.h> as known by inet code */
#ifndef __int8_t_defined
#    define __int8_t_defined
typedef signed char int8_t;
typedef short int   int16_t;
typedef int         int32_t;
#    if __WORDSIZE == 64
typedef long int int64_t;
#    else
__extension__ typedef long long int int64_t;
#    endif
#endif

/* Unsigned.  */
typedef unsigned char      uint8_t;
typedef unsigned short int uint16_t;
#ifndef __uint32_t_defined
typedef unsigned int uint32_t;
#    define __uint32_t_defined
#endif
#if __WORDSIZE == 64
typedef unsigned long int uint64_t;
#else
__extension__ typedef unsigned long long int uint64_t;
#endif

/* Small types.  */

/* Signed.  */
typedef signed char int_least8_t;
typedef short int   int_least16_t;
typedef int         int_least32_t;
#if __WORDSIZE == 64
typedef long int int_least64_t;
#else
__extension__ typedef long long int          int_least64_t;
#endif

/* Unsigned.  */
typedef unsigned char      uint_least8_t;
typedef unsigned short int uint_least16_t;
typedef unsigned int       uint_least32_t;
#if __WORDSIZE == 64
typedef unsigned long int uint_least64_t;
#else
__extension__ typedef unsigned long long int uint_least64_t;
#endif

/* Fast types.  */

/* Signed.  */
typedef signed char int_fast8_t;
#if __WORDSIZE == 64
typedef long int int_fast16_t;
typedef long int int_fast32_t;
typedef long int int_fast64_t;
#else
typedef int                                  int_fast16_t;
typedef int                                  int_fast32_t;
__extension__ typedef long long int          int_fast64_t;
#endif

/* Unsigned.  */
typedef unsigned char uint_fast8_t;
#if __WORDSIZE == 64
typedef unsigned long int uint_fast16_t;
typedef unsigned long int uint_fast32_t;
typedef unsigned long int uint_fast64_t;
#else
typedef unsigned int                         uint_fast16_t;
typedef unsigned int                         uint_fast32_t;
__extension__ typedef unsigned long long int uint_fast64_t;
#endif

/* Types for `void *' pointers.  */
#if __WORDSIZE == 64
#    ifndef __intptr_t_defined
typedef long int intptr_t;
#        define __intptr_t_defined
#    endif
typedef unsigned long int uintptr_t;
#else
#    ifndef __intptr_t_defined
typedef int                                  intptr_t;
#        define __intptr_t_defined
#    endif
typedef unsigned int                         uintptr_t;
#endif

/* Largest integral types.  */
#if __WORDSIZE == 64
typedef long int          intmax_t;
typedef unsigned long int uintmax_t;
#else
__extension__ typedef long long int          intmax_t;
__extension__ typedef unsigned long long int uintmax_t;
#endif

#ifndef __INT64_C
#    if __WORDSIZE == 64
#        define __INT64_C(c)  c##L
#        define __UINT64_C(c) c##UL
#    else
#        define __INT64_C(c)  c##LL
#        define __UINT64_C(c) c##ULL
#    endif
#endif

/* Limits of integral types.  */

/* Minimum of signed integral types.  */
#define INT8_MIN         (-128)
#define INT16_MIN        (-32767 - 1)
#define INT32_MIN        (-2147483647 - 1)
#define INT64_MIN        (-__INT64_C(9223372036854775807) - 1)
/* Maximum of signed integral types.  */
#define INT8_MAX         (127)
#define INT16_MAX        (32767)
#define INT32_MAX        (2147483647)
#define INT64_MAX        (__INT64_C(9223372036854775807))

/* Maximum of unsigned integral types.  */
#define UINT8_MAX        (255)
#define UINT16_MAX       (65535)
#define UINT32_MAX       (4294967295U)
#define UINT64_MAX       (__UINT64_C(18446744073709551615))

/* Minimum of signed integral types having a minimum size.  */
#define INT_LEAST8_MIN   (-128)
#define INT_LEAST16_MIN  (-32767 - 1)
#define INT_LEAST32_MIN  (-2147483647 - 1)
#define INT_LEAST64_MIN  (-__INT64_C(9223372036854775807) - 1)
/* Maximum of signed integral types having a minimum size.  */
#define INT_LEAST8_MAX   (127)
#define INT_LEAST16_MAX  (32767)
#define INT_LEAST32_MAX  (2147483647)
#define INT_LEAST64_MAX  (__INT64_C(9223372036854775807))

/* Maximum of unsigned integral types having a minimum size.  */
#define UINT_LEAST8_MAX  (255)
#define UINT_LEAST16_MAX (65535)
#define UINT_LEAST32_MAX (4294967295U)
#define UINT_LEAST64_MAX (__UINT64_C(18446744073709551615))

/* Minimum of fast signed integral types having a minimum size.  */
#define INT_FAST8_MIN    (-128)
#if __WORDSIZE == 64
#    define INT_FAST16_MIN (-9223372036854775807L - 1)
#    define INT_FAST32_MIN (-9223372036854775807L - 1)
#else
#    define INT_FAST16_MIN (-2147483647 - 1)
#    define INT_FAST32_MIN (-2147483647 - 1)
#endif
#define INT_FAST64_MIN (-__INT64_C(9223372036854775807) - 1)
/* Maximum of fast signed integral types having a minimum size.  */
#define INT_FAST8_MAX  (127)
#if __WORDSIZE == 64
#    define INT_FAST16_MAX (9223372036854775807L)
#    define INT_FAST32_MAX (9223372036854775807L)
#else
#    define INT_FAST16_MAX (2147483647)
#    define INT_FAST32_MAX (2147483647)
#endif
#define INT_FAST64_MAX (__INT64_C(9223372036854775807))

/* Maximum of fast unsigned integral types having a minimum size.  */
#define UINT_FAST8_MAX (255)
#if __WORDSIZE == 64
#    define UINT_FAST16_MAX (18446744073709551615UL)
#    define UINT_FAST32_MAX (18446744073709551615UL)
#else
#    define UINT_FAST16_MAX (4294967295U)
#    define UINT_FAST32_MAX (4294967295U)
#endif
#define UINT_FAST64_MAX (__UINT64_C(18446744073709551615))
/* Values to test for integral types holding `void *' pointer.  */
#if __WORDSIZE == 64
#    define INTPTR_MIN  (-9223372036854775807L - 1)
#    define INTPTR_MAX  (9223372036854775807L)
#    define UINTPTR_MAX (18446744073709551615UL)
#else
#    define INTPTR_MIN  (-2147483647 - 1)
#    define INTPTR_MAX  (2147483647)
#    define UINTPTR_MAX (4294967295U)
#endif

/* Minimum for largest signed integral type.  */
#define INTMAX_MIN  (-__INT64_C(9223372036854775807) - 1)
/* Maximum for largest signed integral type.  */
#define INTMAX_MAX  (__INT64_C(9223372036854775807))

/* Maximum for largest unsigned integral type.  */
#define UINTMAX_MAX (__UINT64_C(18446744073709551615))

/* Limits of other integer types.  */

/* Limits of `ptrdiff_t' type.  */
#if __WORDSIZE == 64
#    define PTRDIFF_MIN (-9223372036854775807L - 1)
#    define PTRDIFF_MAX (9223372036854775807L)
#else
#    define PTRDIFF_MIN (-2147483647 - 1)
#    define PTRDIFF_MAX (2147483647)
#endif

/* Limits of `sig_atomic_t'.  */
#define SIG_ATOMIC_MIN (-2147483647 - 1)
#define SIG_ATOMIC_MAX (2147483647)

/* Limit of `size_t' type.  */
#if __WORDSIZE == 64
#    define SIZE_MAX (18446744073709551615UL)
#else
#    ifdef __WORDSIZE32_SIZE_ULONG
#        define SIZE_MAX (4294967295UL)
#    else
#        define SIZE_MAX (4294967295U)
#    endif
#endif

/* Limits of `wchar_t'.  */
#ifndef WCHAR_MIN
/* These constants might also be defined in <wchar.h>.  */
#    define WCHAR_MIN __WCHAR_MIN
#    define WCHAR_MAX __WCHAR_MAX
#endif

/* Limits of `wint_t'.  */
#define WINT_MIN    (0u)
#define WINT_MAX    (4294967295u)

/* Signed.  */
#define INT8_C(_c)  _c
#define INT16_C(_c) _c
#define INT32_C(_c) _c
#if __WORDSIZE == 64
#    define INT64_C(_c) _c##L
#else
#    define INT64_C(_c) _c##LL
#endif

/* Unsigned.  */
#define UINT8_C(_c)  _c
#define UINT16_C(_c) _c
#define UINT32_C(_c) _c##U
#if __WORDSIZE == 64
#    define UINT64_C(_c) _c##UL
#else
#    define UINT64_C(_c) _c##ULL
#endif

/* Maximal type.  */
#if __WORDSIZE == 64
#    define INTMAX_C(_c)  _c##L
#    define UINTMAX_C(_c) _c##UL
#else
#    define INTMAX_C(_c)  _c##LL
#    define UINTMAX_C(_c) _c##ULL
#endif

#ifdef __cplusplus
}
#endif

#endif /* SIMPLEKERNEL_STDINT_H */
