
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// firstfit.h for Simple-XX/SimpleKernel.

#ifndef _FIRTSTFIT_H_
#define _FIRTSTFIT_H_

#include "stdint.h"
#include "stddef.h"
#include "allocator.h"

// 使用 first fit 算法的分配器
class FIRSTFIT : ALLOCATOR {
private:
    // 字长
    static constexpr const uint64_t BITS_PER_WORD = sizeof(intptr_t);
    // 字长为 64 时的 掩码
    static constexpr const uint64_t MASK = 0x3F;
    // 2^6==64
    static constexpr const uint64_t SHIFT = 6;

    // BUG: 32 位下不能正常工作，目测是字长问题
    // 位图，每一位表示一页内存，1 表示已使用，0 表示未使用
    uint64_t map[((COMMON::PMM_SIZE / COMMON::PAGE_SIZE) / BITS_PER_WORD)];
    // 置位 _idx
    void set(uint64_t _idx);
    // 清零 _idx
    void clr(uint64_t _idx);
    // 测试 _idx
    bool test(uint64_t _idx);
    // 连续 _len 个 _val 位，返回开始索引
    uint64_t find_len(uint64_t _len, bool _val);

protected:
public:
    // _len: 以页为单位
    FIRSTFIT(const char *_name, const void *_addr, size_t _len);
    ~FIRSTFIT(void);
    // 分配长度为 _len 页的内存
    void *alloc(size_t _len);
    // 在 _addr 处分配长度为 _len 页的内存
    // 如果此地址已使用，函数返回 false
    bool alloc(void *_addr, size_t _len);
    // 释放 _addr 处 _len 页的内存
    void free(void *_addr, size_t _len);
    // 已使用页数
    size_t get_used_count(void) const;
    // 未使用页数
    size_t get_free_count(void) const;
};

#endif /* _FIRTSTFIT_H_ */
