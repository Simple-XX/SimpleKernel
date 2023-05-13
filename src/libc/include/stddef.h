
/**
 * @file stddef.h
 * @brief stddef 定义
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2023-03-31
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2023-03-31<td>Zone.N<td>迁移到 doxygen
 * </table>
 */

#ifndef SIMPLEKERNEL_STDDEF_H
#define SIMPLEKERNEL_STDDEF_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _PTRDIFF_T
#    define _PTRDIFF_T
typedef long ptrdiff_t;
#endif

#ifndef _SIZE_T
#    define _SIZE_T
#    undef size_t
typedef unsigned int size_t;
#endif

#ifndef NULL
#    define NULL ((void*)0)
#endif

#ifdef __cplusplus
}
#endif

#endif /* SIMPLEKERNEL_STDDEF_H */
