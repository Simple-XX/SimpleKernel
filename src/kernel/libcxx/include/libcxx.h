
/**
 * @file libcxx.h
 * @brief libcxx 头文件
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2023-07-15
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2023-07-15<td>Zone.N (Zone.Niuzh@hotmail.com)<td>创建文件
 * </table>
 */

#ifndef SIMPLEKERNEL_SRC_KERNEL_LIBCXX_INCLUDE_LIBCXX_H_
#define SIMPLEKERNEL_SRC_KERNEL_LIBCXX_INCLUDE_LIBCXX_H_

#include <cstdint>

/**
 * @brief 构造 c++ 全局对象
 */
void CppInit();

/**
 * @brief 析构 c++ 全局对象
 */
void CppDeInit();

#endif /* SIMPLEKERNEL_SRC_KERNEL_LIBCXX_INCLUDE_LIBCXX_H_ */
