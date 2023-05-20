
/**
 * @file stdarg.h
 * @brief stdarg 定义
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

#ifndef SIMPLEKERNEL_STDARG_H
#define SIMPLEKERNEL_STDARG_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef va_list
#define va_list __builtin_va_list
#endif

#ifndef va_start
#define va_start(_v, _l) __builtin_va_start(_v, _l)
#endif

#ifndef va_arg
#define va_arg(_v, _l) __builtin_va_arg(_v, _l)
#endif

#ifndef va_end
#define va_end(_v) __builtin_va_end(_v)
#endif

#ifndef va_copy
#define va_copy(_d, _s) __builtin_va_copy(_d, _s)
#endif

#ifdef __cplusplus
}
#endif

#endif /* SIMPLEKERNEL_STDARG_H */
