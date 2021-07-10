
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// firstfit.h for Simple-XX/SimpleKernel.

#ifndef _FIRTSTFIT_H_
#define _FIRTSTFIT_H_

#include "stdint.h"
#include "stddef.h"
#include "allocator.h"
#include "list.hpp"

// 物理内存的分配
// 对于一段内存，需要以下信息进行管理
// 1. 开始地址
// 2. 长度，由于以页为最小单位，所以这个值是内存页数
// 3. 引用数，即如果这段内存每被分配一次，值 +1

// 使用 first fit 算法的分配器
class FIRSTFIT : ALLOCATOR {
private:
    // 管理结构
    struct ff_entry_t {
        // 当前页的地址
        void *addr;
        // 拥有多少个连续的页
        size_t npages;
        // 物理页是否被使用
        bool used;
        // 用于 tmp_list_t
        bool operator==(ff_entry_t &_ff) {
            if (addr == _ff.addr && npages == _ff.npages && used == _ff.used) {
                return true;
            }
            else {
                return false;
            }
        }
    };

    // 保存内存信息的链表指针
    tmp_list_t<ff_entry_t> list;

protected:
public:
    FIRSTFIT(const void *_addr, size_t _len);
    ~FIRSTFIT(void);
    bool   init(void);
    void * alloc(size_t _len);
    bool   alloc(void *_addr, size_t _len);
    void   free(void *_addr, size_t _len);
    size_t get_used_pages_count(void) const;
    size_t get_free_pages_count(void) const;
};

#endif /* _FIRTSTFIT_H_ */
