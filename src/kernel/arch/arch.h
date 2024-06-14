
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

#include <cstddef>
#include <cstdint>

/**
 * 体系结构相关初始化
 * @param argc 在不同体系结构有不同含义
 * @param argv 在不同体系结构有不同含义
 */
uint32_t ArchInit(uint32_t argc, uint8_t *argv);

/// 最多回溯 128 层调用栈
static constexpr const size_t kMaxFramesCount = 128;

/**
 * 获取调用栈
 * @param buffer 指向一个数组，该数组用于存储调用栈中的返回地址
 * @param size 数组的大小，即调用栈中最多存储多少个返回地址
 * @return int 成功时返回实际写入数组中的地址数量，失败时返回 -1
 */
__always_inline int backtrace(void **buffer, int size);

/**
 * 打印调用栈
 */
void DumpStack();

#endif /* SIMPLEKERNEL_SRC_KERNEL_ARCH_ARCH_H_ */
