
/**
 * @file spinlock.h
 * @brief 自旋锁定义
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2022-01-01
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2022-01-01<td>MRNIU<td>迁移到 doxygen
 * </table>
 */

#ifndef _SPINLOCK_H_
#define _SPINLOCK_H_

#include "stddef.h"

/**
 * @brief 自旋锁
 * @note 只能用于多核
 */
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
