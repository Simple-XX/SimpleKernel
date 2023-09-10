
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

#ifndef SIMPLEKERNEL_LIBCXX_H
#define SIMPLEKERNEL_LIBCXX_H

#include "cstdint"

/**
 * @brief 入口
 * @param  _argc                   参数个数
 * @param  _argv                   参数列表
 * @return int32_t                 正常返回 0
 */
int32_t libcxx(uint32_t _argc, uint8_t **_argv);

#endif /* SIMPLEKERNEL_LIBCXX_H */
