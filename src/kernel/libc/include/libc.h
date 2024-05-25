
/**
 * @file libc.h
 * @brief libc 头文件
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

#ifndef SIMPLEKERNEL_SRC_KERNEL_LIBC_INCLUDE_LIBC_H_
#define SIMPLEKERNEL_SRC_KERNEL_LIBC_INCLUDE_LIBC_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"

/**
 * @brief 入口
 * @param  _argc                   参数个数
 * @param  _argv                   参数列表
 * @return int32_t                 正常返回 0
 */
int32_t libc(uint32_t _argc, uint8_t **_argv);

#ifdef __cplusplus
}
#endif

#endif /* SIMPLEKERNEL_SRC_KERNEL_LIBC_INCLUDE_LIBC_H_ */
