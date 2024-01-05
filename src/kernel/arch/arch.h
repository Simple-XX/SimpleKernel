
/**
 * @file arch.h
 * @brief arch 头文件
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

#ifndef SIMPLEKERNEL_SRC_KERNEL_ARCH_ARCH_H
#define SIMPLEKERNEL_SRC_KERNEL_ARCH_ARCH_H

#include "cstdint"

int32_t arch_init(uint32_t _argc, uint8_t **_argv);

#endif /* SIMPLEKERNEL_SRC_KERNEL_ARCH_ARCH_H */
