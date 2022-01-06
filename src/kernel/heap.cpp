
/**
 * @file heap.cpp
 * @brief 堆抽象头文件
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

#include "stdio.h"
#include "common.h"
#include "pmm.h"
#include "heap.h"

HEAP &HEAP::get_instance(void) {
    /// 定义全局 HEAP 对象
    static HEAP heap;
    return heap;
}

bool HEAP::init(void) {
    static SLAB slab_allocator(
        "SLAB Allocator", PMM::get_instance().get_non_kernel_space_start(),
        PMM::get_instance().get_non_kernel_space_length() * COMMON::PAGE_SIZE);
    allocator = (ALLOCATOR *)&slab_allocator;
    info("heap init.\n");
    return 0;
}

void *HEAP::malloc(size_t _byte) {
    return (void *)allocator->alloc(_byte);
}

void HEAP::free(void *_addr) {
    // 堆不需要 _len 参数
    allocator->free((uintptr_t)_addr, 0);
    return;
}

/**
 * @brief malloc 定义
 * @param  _size           要申请的 bytes
 * @return void*           申请到的地址
 */
extern "C" void *malloc(size_t _size) {
    return (void *)HEAP::get_instance().malloc(_size);
}

/**
 * @brief free 定义
 * @param  _p              要释放的内存地址
 */
extern "C" void free(void *_p) {
    HEAP::get_instance().free(_p);
    return;
}
