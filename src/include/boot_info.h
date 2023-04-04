
/**
 * @file boot_info.h
 * @brief 启动信息接口
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

#ifndef SIMPLEKERNEL_BOOT_INFO_H
#define SIMPLEKERNEL_BOOT_INFO_H

#include "stdint.h"
#include "resource.h"

/**
 * @brief 启动信息接口
 * 由引导传递的机器信息处理
 * 如 grub2 传递的 multiboot2 结构
 * opensbi 传递的 dtb 结构
 * 注意这部分是通过内存传递的，在重新保存之前不能被覆盖
 * 架构专有的数据在 dtb.h 或 multiboot2.h
 * 实现在 dtb.cpp 或 multiboot2.cpp
 */
namespace BOOT_INFO {
/// 声明，定义在具体的实现中
/// 是否已经初始化过
extern bool inited;
/// 地址
extern "C" uintptr_t boot_info_addr;
/// 长度
extern size_t boot_info_size;
/// 保存 sbi 传递的启动核
extern "C" size_t dtb_init_hart;

/**
 * @brief 初始化，定义在具体实现中
 * @return true            成功
 * @return false           成功
 */
extern bool init(void);

/**
 * @brief 获取物理内存信息
 * @return resource_t      物理内存资源信息
 */
extern resource_t get_memory(void);

/**
 * @brief 获取 clint 信息
 * @return resource_t       clint 资源信息
 */
extern resource_t get_clint(void);

/**
 * @brief 获取 plic 信息
 * @return resource_t       plic 资源信息
 */
extern resource_t get_plic(void);
}; // namespace BOOT_INFO

#endif /* SIMPLEKERNEL_BOOT_INFO_H */
