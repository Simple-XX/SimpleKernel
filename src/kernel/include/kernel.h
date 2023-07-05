
/**
 * @file kernel.h
 * @brief 内核头文件
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2021-09-18
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2021-09-18<td>Zone.N (Zone.Niuzh@hotmail.com)<td>迁移到 doxygen
 * </table>
 */

#ifndef SIMPLEKERNEL_KERNEL_H
#define SIMPLEKERNEL_KERNEL_H

/// 内核入口函数指针
typedef int (*entry_func)(int, char**);

/// 内核启动参数结构体

/**
 * @brief 声明 kernel_main 用 C 方法编译
 */
extern "C" int kernel_main(int, char**);

#endif /* SIMPLEKERNEL_KERNEL_H */
