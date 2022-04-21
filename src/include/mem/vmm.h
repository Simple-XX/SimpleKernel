
/**
 * @file vmm.h
 * @brief 虚拟内存头文件
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

#ifndef _VMM_H_
#define _VMM_H_

#include "limits.h"
#include "common.h"

// TODO: 可以优化

/// 页表项，最底层
typedef uintptr_t pte_t;
/// 页表，也可以是页目录，它们的结构是一样的
typedef uintptr_t *pt_t;

/// 每个页表能映射多少页 = 页大小/页表项大小: 2^9
static constexpr const size_t VMM_PAGES_PRE_PAGE_TABLE =
    COMMON::PAGE_SIZE / sizeof(pte_t);

/// 映射内核空间的大小
static constexpr const size_t VMM_KERNEL_SPACE_SIZE = COMMON::KERNEL_SPACE_SIZE;

/// 内核映射的页数
static constexpr const size_t VMM_KERNEL_SPACE_PAGES =
    VMM_KERNEL_SPACE_SIZE / COMMON::PAGE_SIZE;

#if defined(__i386__)
/// P = 1 表示有效； P = 0 表示无效。
static constexpr const uint8_t VMM_PAGE_VALID = 1 << 0;
/// 如果为 0  表示页面只读或可执行。
static constexpr const uint8_t VMM_PAGE_READABLE   = 0;
static constexpr const uint8_t VMM_PAGE_WRITABLE   = 1 << 1;
static constexpr const uint8_t VMM_PAGE_EXECUTABLE = 0;
/// U/S-- 位 2 是用户 / 超级用户 (User/Supervisor) 标志。
/// 如果为 1 那么运行在任何特权级上的程序都可以访问该页面。
static constexpr const uint8_t VMM_PAGE_USER = 1 << 2;
/// 内核虚拟地址相对物理地址的偏移
static constexpr const size_t KERNEL_OFFSET = 0x0;
/// PTE 属性位数
static constexpr const size_t VMM_PTE_PROP_BITS = 12;
/// PTE 页内偏移位数
static constexpr const size_t VMM_PAGE_OFF_BITS = 12;
/// VPN 位数
static constexpr const size_t VMM_VPN_BITS = 10;
/// VPN 位数掩码，10 位 VPN
static constexpr const size_t VMM_VPN_BITS_MASK = 0x3FF;
/// i386 使用了两级页表
static constexpr const size_t VMM_PT_LEVEL = 2;

#elif defined(__x86_64__)
/// P = 1 表示有效； P = 0 表示无效。
static constexpr const uint8_t VMM_PAGE_VALID = 1 << 0;
/// 如果为 0  表示页面只读或可执行。
static constexpr const uint8_t VMM_PAGE_READABLE   = 0;
static constexpr const uint8_t VMM_PAGE_WRITABLE   = 1 << 1;
static constexpr const uint8_t VMM_PAGE_EXECUTABLE = 0;
/// U/S-- 位 2 是用户 / 超级用户 (User/Supervisor) 标志。
/// 如果为 1 那么运行在任何特权级上的程序都可以访问该页面。
static constexpr const uint8_t VMM_PAGE_USER = 1 << 2;
/// 内核虚拟地址相对物理地址的偏移
static constexpr const size_t KERNEL_OFFSET = 0x0;
/// PTE 属性位数
static constexpr const size_t VMM_PTE_PROP_BITS = 12;
/// PTE 页内偏移位数
static constexpr const size_t VMM_PAGE_OFF_BITS = 12;
/// VPN 位数
static constexpr const size_t VMM_VPN_BITS = 9;
/// VPN 位数掩码，9 位 VPN
static constexpr const size_t VMM_VPN_BITS_MASK = 0x1FF;
/// x86_64 使用了四级页表
static constexpr const size_t VMM_PT_LEVEL = 4;

#elif defined(__riscv)
enum {
    VMM_PAGE_VALID_OFFSET      = 0,
    VMM_PAGE_READABLE_OFFSET   = 1,
    VMM_PAGE_WRITABLE_OFFSET   = 2,
    VMM_PAGE_EXECUTABLE_OFFSET = 3,
    VMM_PAGE_USER_OFFSET       = 4,
    VMM_PAGE_GLOBAL_OFFSET     = 5,
    VMM_PAGE_ACCESSED_OFFSET   = 6,
    VMM_PAGE_DIRTY_OFFSET      = 7,
};
/// 有效位
static constexpr const uint8_t VMM_PAGE_VALID = 1 << VMM_PAGE_VALID_OFFSET;
/// 可读位
static constexpr const uint8_t VMM_PAGE_READABLE = 1
                                                   << VMM_PAGE_READABLE_OFFSET;
/// 可写位
static constexpr const uint8_t VMM_PAGE_WRITABLE = 1
                                                   << VMM_PAGE_WRITABLE_OFFSET;
/// 可执行位
static constexpr const uint8_t VMM_PAGE_EXECUTABLE =
    1 << VMM_PAGE_EXECUTABLE_OFFSET;
/// 用户位
static constexpr const uint8_t VMM_PAGE_USER = 1 << VMM_PAGE_USER_OFFSET;
/// 全局位，我们不会使用
static constexpr const uint8_t VMM_PAGE_GLOBAL = 1 << VMM_PAGE_GLOBAL_OFFSET;
/// 已使用位，用于替换算法
static constexpr const uint8_t VMM_PAGE_ACCESSED = 1
                                                   << VMM_PAGE_ACCESSED_OFFSET;
/// 已修改位，用于替换算法
static constexpr const uint8_t VMM_PAGE_DIRTY = 1 << VMM_PAGE_DIRTY_OFFSET;
/// 内核虚拟地址相对物理地址的偏移
static constexpr const size_t KERNEL_OFFSET = 0x0;
/// PTE 属性位数
static constexpr const size_t VMM_PTE_PROP_BITS = 10;
/// PTE 页内偏移位数
static constexpr const size_t VMM_PAGE_OFF_BITS = 12;
/// VPN 位数
static constexpr const size_t VMM_VPN_BITS = 9;
/// VPN 位数掩码，9 位 VPN
static constexpr const size_t VMM_VPN_BITS_MASK = 0x1FF;
/// riscv64 使用了三级页表
static constexpr const size_t VMM_PT_LEVEL = 3;
#endif

/**
 * @brief 虚拟地址到物理地址转换
 * @param  _va             要转换的虚拟地址
 * @return constexpr uintptr_t 转换好的地址
 */
static constexpr uintptr_t VMM_VA2PA(uintptr_t _va) {
    return _va - KERNEL_OFFSET;
}

/**
 * @brief 物理地址到虚拟地址转换
 * @param  _pa             要转换的物理地址
 * @return constexpr uintptr_t 转换好的地址
 */
static constexpr uintptr_t VMM_PA2VA(uintptr_t _pa) {
    return _pa + KERNEL_OFFSET;
}

/**
 * @brief 虚拟内存抽象
 */
class VMM {
private:
    /**
     * @brief 物理地址转换到页表项
     * @param  _pa             物理地址
     * @return constexpr uintptr_t 对应的虚拟地址
     * @note 0~11: pte 属性
     * 12~31: 页表的物理页地址
     */
    static constexpr uintptr_t PA2PTE(uintptr_t _pa) {
        return (_pa >> VMM_PAGE_OFF_BITS) << VMM_PTE_PROP_BITS;
    }

    /**
     * @brief 页表项转换到物理地址
     * @param  _pte            页表
     * @return constexpr uintptr_t 对应的物理地址
     */
    static constexpr uintptr_t PTE2PA(const pte_t _pte) {
        return (((uintptr_t)_pte) >> VMM_PTE_PROP_BITS) << VMM_PAGE_OFF_BITS;
    }

    /**
     * @brief 计算 X 级页表的位置
     * @param  _level          级别
     * @return constexpr uintptr_t 偏移
     */
    static constexpr uintptr_t PXSHIFT(const size_t _level) {
        return VMM_PAGE_OFF_BITS + (VMM_VPN_BITS * _level);
    }

    /**
     * @brief 获取 _va 的第 _level 级 VPN
     * @note 例如虚拟地址右移 12+(10 * _level) 位，
     * 得到的就是第 _level 级页表的 VPN
     */
    static constexpr uintptr_t PX(size_t _level, uintptr_t _va) {
        return (_va >> PXSHIFT(_level)) & VMM_VPN_BITS_MASK;
    }

    /**
     * @brief 在 _pgd 中查找 _va 对应的页表项
     * 如果未找到，_alloc 为真时会进行分配
     * @param  _pgd            要查找的页目录
     * @param  _va             虚拟地址
     * @param  _alloc          是否分配
     * @return pte_t*          未找到返回 nullptr
     */
    pte_t *find(const pt_t _pgd, uintptr_t _va, bool _alloc);

protected:
public:
    /**
     * @brief 获取单例
     * @return VMM&             静态对象
     */
    static VMM &get_instance(void);

    /**
     * @brief 初始化
     * @return true            成功
     * @return false           失败
     */
    bool init(void);

    /**
     * @brief 获取当前页目录
     * @return pt_t            当前页目录
     */
    pt_t get_pgd(void);

    /**
     * @brief 设置当前页目录
     * @param  _pgd            要设置的页目录
     */
    void set_pgd(const pt_t _pgd);

    /**
     * @brief 映射物理地址到虚拟地址
     * @param  _pgd            要使用的页目录
     * @param  _va             要映射的虚拟地址
     * @param  _pa             物理地址
     * @param  _flag           属性
     */
    void mmap(const pt_t _pgd, uintptr_t _va, uintptr_t _pa, uint32_t _flag);

    /**
     * @brief 取消映射
     * @param  _pgd            要操作的页目录
     * @param  _va             要取消映射的虚拟地址
     */
    void unmmap(const pt_t _pgd, uintptr_t _va);

    /**
     * @brief 获取映射的物理地址
     * @param  _pgd            页目录
     * @param  _va             虚拟地址
     * @param  _pa             如果已经映射，保存映射的物理地址，否则为 nullptr
     * @return true            已映射
     * @return false           未映射
     */
    bool get_mmap(const pt_t _pgd, uintptr_t _va, const void *_pa);
};

#endif /* _VMM_H */
