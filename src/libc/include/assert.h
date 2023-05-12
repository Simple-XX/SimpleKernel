
/**
 * @file assert.h
 * @brief assert 实现
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * Based on https://wiki.osdev.org/Raspberry_Pi_Bare_Bones
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

#ifndef SIMPLEKERNEL_ASSERT_H
#define SIMPLEKERNEL_ASSERT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stdio.h"

#define assert(_e) \
    ((void)((_e) ? ((void)0) : __assert(#_e, __FILE__, __LINE__)))
#define __assert(_e, _file, _line) \
    ((void)err("%s:%d: failed assertion `%s'\n", _file, _line, _e))

#ifdef __cplusplus
}
#endif

#endif /* SIMPLEKERNEL_ASSERT_H */
