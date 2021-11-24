
/**
 * @file firstfit.h
 * @brief firstfit 内存分配器头文件
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

#ifndef _FIRTSTFIT_H_
#define _FIRTSTFIT_H_

#include "stdint.h"
#include "stddef.h"
#include "common.h"
#include "allocator.h"

/**
 * @brief 使用 first fit 算法的分配器
 */
class FIRSTFIT : ALLOCATOR {
private:
    /// 字长
    static constexpr const uint64_t BITS_PER_WORD = sizeof(uintptr_t);
#if __WORDSIZE == 64
    /// 字长为 64 时的 掩码
    static constexpr const uint64_t MASK = 0x3F;
    /// 2^6==64
    static constexpr const uint64_t SHIFT = 6;
#elif __WORDSIZE == 32
    /// 字长为 32 时的 掩码
    static constexpr const uint64_t MASK = 0x1F;
    /// 2^5==32
    static constexpr const uint64_t SHIFT = 5;
#endif
    /// 位图数组长度，设置为占用一个页，4kb，32768 个位，每个 bit
    /// 代表一页，最大表示 128MB
    static constexpr const size_t BITS_ARR_SIZE =
        COMMON::PAGE_SIZE / BITS_PER_WORD;
    /// 位图，每一位表示一页内存，1 表示已使用，0 表示未使用
    uintptr_t map[BITS_ARR_SIZE];

    /**
     * @brief 置位 _idx
     * @param  _idx            要置位的索引
     */
    void set(size_t _idx);

    /**
     * @brief 清零 _idx
     * @param  _idx            要清零的索引
     */
    void clr(size_t _idx);

    /**
     * @brief 测试 _idx
     * @param  _idx            要测试的索引
     * @return true            已使用
     * @return false           未使用
     */
    bool test(size_t _idx);

    /**
     * @brief 寻找连续 _len 个 _val 位，返回开始索引
     * @param  _len            连续
     * @param  _val            值
     * @return size_t          开始索引
     */
    size_t find_len(size_t _len, bool _val);

protected:
public:
    /**
     * @brief 创建分配器
     * @param  _name           分配器名称
     * @param  _addr           开始地址
     * @param  _len            长度，页
     */
    FIRSTFIT(const char *_name, uintptr_t _addr, size_t _len);

    ~FIRSTFIT(void);

    /**
     * @brief 分配长度为 _len 页的内存
     * @param  _len            页数
     * @return uintptr_t       分配的内存起点地址
     */
    uintptr_t alloc(size_t _len);

    /**
     * @brief 在 _addr 处分配长度为 _len 页的内存
     * @param  _addr           指定的地址
     * @param  _len            页数
     * @return true            成功
     * @return false           失败
     */
    bool alloc(uintptr_t _addr, size_t _len);

    /**
     * @brief 释放 _addr 处 _len 页的内存
     * @param  _addr           要释放内存起点地址
     * @param  _len            页数
     */
    void free(uintptr_t _addr, size_t _len);

    /**
     * @brief 获取已使用页数
     * @return size_t          已经使用的页数
     */
    size_t get_used_count(void) const;

    /**
     * @brief 获取未使用页数
     * @return size_t          未使用的页数
     */
    size_t get_free_count(void) const;
};

#endif /* _FIRTSTFIT_H_ */
