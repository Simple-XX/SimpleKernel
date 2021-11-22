
/**
 * @file e820.h
 * @brief e820 相关定义
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

#ifndef _E820_H_
#define _E820_H_

#include "stddef.h"
#include "stdint.h"

/// 最大数量
static constexpr const uint32_t E820_MAX = 8;
/// RAM 标记
static constexpr const uint32_t E820_RAM = 1;
/// 保留 标记
static constexpr const uint32_t E820_RESERVED = 2;
/// ACPI 标记
static constexpr const uint32_t E820_ACPI = 3;
/// NVS 标记
static constexpr const uint32_t E820_NVS = 4;
/// 不可用 标记
static constexpr const uint32_t E820_UNUSABLE = 5;

/**
 * @brief e820 项
 */
struct e820entry_t {
    // 数据类型由位数决定
    uintptr_t addr;
    size_t    length;
    uint32_t  type;
};

/**
 * @brief e820 表
 */
struct e820map_t {
    e820map_t(void) {
        nr_map = 0;
        return;
    }
    ~e820map_t(void) {
        return;
    }
    uint32_t    nr_map;
    e820entry_t map[E820_MAX];
};

#endif /* _E820_H_ */
