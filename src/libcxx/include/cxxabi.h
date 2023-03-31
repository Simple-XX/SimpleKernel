
/**
 * @file cxxabi.h
 * @brief C++ abi 支持
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2021-09-18
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * Based on https://wiki.osdev.org/C%2B%2B
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2021-09-18<td>digmouse233<td>迁移到 doxygen
 * </table>
 */

#ifndef SIMPLEKERNEL_CXXABI_H
#define SIMPLEKERNEL_CXXABI_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief c++ 初始化，全局对象的构造
 */
void cpp_init(void);

int  __cxa_atexit(void (*f)(void *), void *objptr, void *dso);
void __cxa_finalize(void *f);

int  __aeabi_atexit(void (*f)(void *), void *objptr, void *dso);
void __cxa_finalize(void *f);

#ifdef __cplusplus
};
#endif

#endif /* SIMPLEKERNEL_CXXABI_H */
