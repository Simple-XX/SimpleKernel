
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

#ifndef SIMPLEKERNEL_SRC_KERNEL_ARCH_ARCH_H_
#define SIMPLEKERNEL_SRC_KERNEL_ARCH_ARCH_H_

#include <cstdint>

/**
 * 体系结构相关初始化
 * @param argc 在不同体系结构有不同含义
 * @param argv 在不同体系结构有不同含义
 */
uint32_t ArchInit(uint32_t argc, uint8_t *argv);

/**
 * 打印调用栈
 */
void DumpStack();

#endif /* SIMPLEKERNEL_SRC_KERNEL_ARCH_ARCH_H_ */
