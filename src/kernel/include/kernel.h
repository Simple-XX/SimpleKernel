
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

#ifndef _KERNEL_H_
#define _KERNEL_H_

/**
 * @brief 声明 kernel_main 用 C 方法编译
 * @note 这个函数不会返回
 */
// extern "C" void kernel_main(void);

extern "C" void kernel_main(uintptr_t _hart_id, uintptr_t _dtb_addr);
extern "C" void kernel_main_smp(void);

/**
 * @brief 物理内存测试函数
 * @return int             0 成功
 */
int test_pmm(void);

/**
 * @brief 虚拟内存测试函数
 * @return int             0 成功
 */
int test_vmm(void);

/**
 * @brief 堆测试函数
 * @return int             desc
 */
int test_heap(void);

/**
 * @brief 输出系统信息
 */
void show_info(void);

#endif /* _KERNEL_H_ */
