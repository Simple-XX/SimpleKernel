
/**
 * @file allocator.cpp
 * @brief 分配器抽象类实现
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

#include "stddef.h"
#include "stdint.h"
#include "assert.h"
#include "common.h"
#include "allocator.h"

ALLOCATOR::ALLOCATOR(const char *_name, uintptr_t _addr, size_t _len) {
    // 默认名字
    name                 = _name;
    allocator_start_addr = _addr;
    allocator_length     = _len;
    // 初始状态下所有页都未使用
    allocator_free_count = allocator_length;
    allocator_used_count = 0;
    return;
}

ALLOCATOR::~ALLOCATOR(void) {
    return;
}
