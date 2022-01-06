
/**
 * @file pmm.h
 * @brief 物理内存管理头文件
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

#ifndef _PMM_H_
#define _PMM_H_

#include "stddef.h"
#include "stdint.h"
#include "firstfit.h"
#include "allocator.h"

/**
 * @brief 物理内存管理接口
 * 对物理内存的管理来说
 * 1. 管理所有的物理内存，不论是否被机器保留/无法访问
 * 2. 内存开始地址与长度由 bootloader 给出: x86 下为 grub, riscv 下为 opensbi
 * 3.
 *    不关心内存是否被使用，但是默认的物理内存分配空间从内核结束后开始
 *    如果由体系结构需要分配内核开始前内存空间的，则尽量避免
 * 4. 最管理单位为页
 */
class PMM {
private:
    /// 物理内存开始地址
    uintptr_t start;
    /// 物理内存长度，单位为 bytes
    size_t length;
    /// 物理内存页数
    size_t total_pages;
    /// 内核空间起始地址
    uintptr_t kernel_space_start;
    /// 内核空间大小，单位为 bytes
    size_t kernel_space_length;
    /// 非内核空间起始地址
    uintptr_t non_kernel_space_start;
    /// 非内核空间大小，单位为 bytes
    size_t non_kernel_space_length;

    /// 内核空间不会位于内存中间，导致出现非内核空间被切割为两部分的情况
    /// 物理内存分配器，分配内核空间
    ALLOCATOR *kernel_space_allocator;
    /// 物理内存分配器，分配非内核空间
    ALLOCATOR *allocator;

    /**
     * @brief 将 multiboot2/dtb 信息移动到内核空间
     */
    void move_boot_info(void);

protected:
public:
    /**
     * @brief 获取单例
     * @return PMM&             静态对象
     */
    static PMM &get_instance(void);

    /**
     * @brief 初始化
     * @return true            成功
     * @return false           失败
     * @todo 移动到构造函数去
     */
    bool init(void);

    /**
     * @brief 获取物理内存长度
     * @return size_t          物理内存长度
     */
    size_t get_pmm_length(void) const;

    /**
     * @brief 分配一页
     * @return uintptr_t       分配的内存起始地址
     */
    uintptr_t alloc_page(void);

    /**
     * @brief 分配多页
     * @param  _len            页数
     * @return uintptr_t       分配的内存起始地址
     */
    uintptr_t alloc_pages(size_t _len);

    /**
     * @brief 分配以指定地址开始的 _len 页
     * @param  _addr           指定的地址
     * @param  _len            页数
     * @return true            成功
     * @return false           失败
     */
    bool alloc_pages(uintptr_t _addr, size_t _len);

    /**
     * @brief 在内核空间申请一页
     * @return uintptr_t       分配的内存起始地址
     */
    uintptr_t alloc_page_kernel(void);

    /**
     * @brief 在内核空间分配 _len 页
     * @param  _len            页数
     * @return uintptr_t       分配到的内存起始地址
     */
    uintptr_t alloc_pages_kernel(size_t _len);

    /**
     * @brief 在内核空间分配以指定地址开始的 _len 页
     * @param  _addr           指定的地址
     * @param  _len            页数
     * @return true            成功
     * @return false           失败
     */
    bool alloc_pages_kernel(uintptr_t _addr, size_t _len);

    /**
     * @brief 回收一页
     * @param  _addr           要回收的地址
     */
    void free_page(uintptr_t _addr);

    /**
     * @brief 回收多页
     * @param  _addr           要回收的地址
     * @param  _len            页数
     */
    void free_pages(uintptr_t _addr, size_t _len);

    /**
     * @brief 获取当前已使用页数
     * @return size_t          已使用页数
     */
    size_t get_used_pages_count(void) const;

    /**
     * @brief 获取当前空闲页
     * @return size_t          空闲页数
     */
    size_t get_free_pages_count(void) const;

    /**
     * @brief 获取非内核空间起始地址
     * @return uintptr_t        非内核空间起始地址
     */
    uintptr_t get_non_kernel_space_start(void) const;

    /**
     * @brief 获取非内核空间大小，单位为 byte
     * @return size_t           非内核空间大小
     */
    size_t get_non_kernel_space_length(void) const;
};

#endif /* _PMM_H_ */
