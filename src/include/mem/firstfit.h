
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
    static constexpr const uint64_t BITS_PER_WORD = sizeof(uint64_t);
    static constexpr const uint64_t MASK          = 0x3F;
    static constexpr const uint64_t SHIFT         = 6;

    // 位图，每一位表示一页内存，1 表示已使用，0 表示未使用
    uint64_t map[((COMMON::PMM_SIZE / COMMON::PAGE_SIZE) / BITS_PER_WORD)];
    // 置位 _idx
    void set(uint64_t _idx) {
        map[_idx >> SHIFT] |= (uint64_t)1 << (_idx & MASK);
        return;
    }
    // 清零 _idx
    void clr(uint64_t _idx) {
        map[_idx >> SHIFT] &= ~((uint64_t)1 << (_idx & MASK));
        return;
    }
    // 测试 _idx
    bool test(uint64_t _idx) {
        return map[_idx >> SHIFT] & ((uint64_t)1 << (_idx & MASK));
    }

    // 连续 _len 个 _val 位，返回开始索引
    uint64_t find_len(uint64_t _len, bool _val) {
        uint64_t count = 0;
        uint64_t idx   = 0;
        // 遍历位图
        for (uint64_t i = 0; i < (COMMON::PMM_SIZE / COMMON::PAGE_SIZE); i++) {
            if (test(i) != _val) {
                count = 0;
                idx   = i;
            }
            else {
                count++;
            }
            if (count == _len) {
                return idx;
            }
        }
        return ~(uint64_t)0;
    }

protected:
public:
    FIRSTFIT(const void *_addr, size_t _len);
    ~FIRSTFIT(void);
    void * alloc(size_t _len);
    bool   alloc(void *_addr, size_t _len);
    void   free(void *_addr, size_t _len);
    size_t get_used_pages_count(void) const;
    size_t get_free_pages_count(void) const;
};

#endif /* _FIRTSTFIT_H_ */
