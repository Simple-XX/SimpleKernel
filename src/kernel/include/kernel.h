
/**
 * @file kernel.h
 * @brief 内核头文件
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

#ifndef SIMPLEKERNEL_SRC_KERNEL_INCLUDE_KERNEL_H_
#define SIMPLEKERNEL_SRC_KERNEL_INCLUDE_KERNEL_H_

#include <cstddef>
#include <cstdint>

/**
 * @brief 负责 crtbegin 的工作
 * @param argc
 *          riscv64: 启动核 id
 *          x86_64: 参数个数
 * @param argv 参数指针
 *          riscv64: dtb 地址
 *          x86_64: BasicInfo 地址
 * @return uint32_t 正常返回 0
 */
extern "C" [[maybe_unused]] [[noreturn]] void _start(uint32_t argc,
                                                     uint8_t* argv);

/**
 * @brief 内核入口
 * @param argc 同 _start
 * @param argv 同 _start
 * @return uint32_t 正常返回 0
 */
uint32_t main(uint32_t argc, uint8_t* argv);

uint32_t InterruptInit(uint32_t argc, uint8_t* argv);

#endif /* SIMPLEKERNEL_SRC_KERNEL_INCLUDE_KERNEL_H_ */
