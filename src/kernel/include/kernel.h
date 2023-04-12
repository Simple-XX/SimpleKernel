
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
 * <tr><td>2021-09-18<td>digmouse233<td>迁移到 doxygen
 * </table>
 */

#ifndef SIMPLEKERNEL_KERNEL_H
#define SIMPLEKERNEL_KERNEL_H

/**
 * @brief 声明 kernel_main 用 C 方法编译
 * @note 这个函数不会返回
 */
extern "C" void kernel_main(void);

/**
 * @brief 输出系统信息
 */
void show_info(void);

#endif /* SIMPLEKERNEL_KERNEL_H */
