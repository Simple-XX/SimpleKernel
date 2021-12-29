
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// spinlock.h for Simple-XX/SimpleKernel.

#ifndef _SPINLOCK_H_
#define _SPINLOCK_H_

#include "stddef.h"

// 自旋锁
// TODO
struct spinlock_t {
private:
    bool is_holding(void);
    void push_off(void);
    void pop_off(void);

public:
    // 自旋锁名称
    const char *name;
    // 是否 lock
    volatile bool   locked;
    volatile size_t hartid;
    spinlock_t(void);
    spinlock_t(const char *_name);
    void acquire(void);
    void release(void);
};

#endif /* _SPINLOCK_H_ */
