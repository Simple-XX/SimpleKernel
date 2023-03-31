
/**
 * @file math.h
 * @brief math 定义
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2023-03-31
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * Based on libgcc
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2023-03-31<td>Zone.N<td>迁移到 doxygen
 * </table>
 */

#ifndef SIMPLEKERNEL_MATH_H
#define SIMPLEKERNEL_MATH_H

#ifdef __cplusplus
extern "C" {
#endif

// TODO: 更多支持
// TODO: 浮点数

long long          divmoddi4(long long num, long long den, long long *rem_p);
unsigned long long udivmoddi4(unsigned long long num, unsigned long long den,
                              unsigned long long *rem_p);
unsigned long long udivdi3(unsigned long long num, unsigned long long den);
unsigned long long umoddi3(unsigned long long num, unsigned long long den);

#ifdef __cplusplus
}
#endif

#endif /* SIMPLEKERNEL_MATH_H */
