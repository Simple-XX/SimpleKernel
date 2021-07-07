
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// firstfit.h for Simple-XX/SimpleKernel.

#ifndef _FIRTSTFIT_H_
#define _FIRTSTFIT_H_

#include "stdint.h"
#include "allocator.h"

// TODO: 面向对象重构
class ff_list_entry_t {
public:
    // 当前页的地址
    uint8_t *addr;
    // 拥有多少个连续的页
    uint32_t npages;
    // 物理页被引用的次数
    int32_t ref;
    // 当前页状态
    uint32_t         flag;
    ff_list_entry_t *next;
    ff_list_entry_t *prev;
};

// 使用 first fit 算法的分配器
class FIRSTFIT : ALLOCATOR {
private:
protected:
public:
    FIRSTFIT(const void *_addr, size_t _len);
    ~FIRSTFIT(void);
    bool   init(void);
    void * alloc(void);
    bool   alloc(void *_addr);
    void   free(void *_addr);
    size_t get_used_pages_count(void) const;
    size_t get_free_pages_count(void) const;
};

#endif /* _FIRTSTFIT_H_ */
