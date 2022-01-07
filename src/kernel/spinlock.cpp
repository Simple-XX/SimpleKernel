
/**
 * @file spinlock.cpp
 * @brief 自旋锁实现
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

#include "spinlock.h"
#include "cpu.hpp"
#include "scheduler.h"
#include "core.h"
#include "cpu.hpp"
#include "string"

bool spinlock_t::is_holding(void) {
    bool r = (locked && (hartid == COMMON::get_curr_core_id()));
    return r;
}

void spinlock_t::push_off(void) {
    bool old = CPU::STATUS_INTR();

    CPU::DISABLE_INTR();

    if (core_t::cores[COMMON::get_curr_core_id()].noff == 0) {
        core_t::cores[COMMON::get_curr_core_id()].intr_enable = old;
    }
    core_t::cores[COMMON::get_curr_core_id()].noff += 1;

    return;
}

void spinlock_t::pop_off(void) {
    if (CPU::STATUS_INTR() == true) {
        err("pop_off - interruptible\n");
    }

    if (core_t::cores[COMMON::get_curr_core_id()].noff < 1) {
        err("pop_off\n");
    }
    core_t::cores[COMMON::get_curr_core_id()].noff -= 1;

    if ((core_t::cores[COMMON::get_curr_core_id()].noff == 0) &&
        (core_t::cores[COMMON::get_curr_core_id()].intr_enable == true)) {
        CPU::ENABLE_INTR();
    }

    return;
}

spinlock_t::spinlock_t(void) {
    name   = "";
    locked = false;
    hartid = -1;
    return;
}

spinlock_t::spinlock_t(const char *_name) : name(_name) {
    locked = false;
    hartid = COMMON::get_curr_core_id();
    info("spinlock: %s init.\n", name);
    return;
}

spinlock_t::spinlock_t(const mystl::string &_name) : name(_name.c_str()) {
    locked = false;
    hartid = COMMON::get_curr_core_id();
    info("spinlock: %s init.\n", name);
    return;
}

bool spinlock_t::init(const char *_name) {
    name   = _name;
    locked = false;
    hartid = COMMON::get_curr_core_id();
    info("spinlock: %s init.\n", name);
    return true;
}

void spinlock_t::lock(void) {
    push_off();
    assert(is_holding() != true);

    while (__atomic_test_and_set(&locked, 1) != 0) {
        ;
    }

    __sync_synchronize();

    hartid = COMMON::get_curr_core_id();
    return;
}

// Release the lock.
void spinlock_t::unlock(void) {
    assert(is_holding() != false);
    hartid = -1;
    __sync_synchronize();
    __sync_lock_release(&locked);
    pop_off();
    return;
}
