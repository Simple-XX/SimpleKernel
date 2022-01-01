
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
#include "core.hpp"
#include "cpu.hpp"

// TODO

spinlock_t::spinlock_t(void) {
    name   = "";
    locked = false;
    hartid = -1;
    return;
}

spinlock_t::spinlock_t(const char *_name) : name(_name) {
    locked = false;
    hartid = -1;
    return;
}

void spinlock_t::acquire(void) {
    push_off();
    if (is_holding() == true) {
        err("acquire\n");
    }

    while (__atomic_test_and_set(&locked, true) != false) {
        ;
    }

    __sync_synchronize();

    hartid = CPU::get_curr_core_id();
    return;
}

// Release the lock.
void spinlock_t::release(void) {
    if (is_holding() == false) {
        err("release\n");
    }
    hartid = -1;
    __sync_synchronize();
    __sync_lock_release(&locked);
    pop_off();
    return;
}

bool spinlock_t::is_holding(void) {
    bool r = (locked && hartid == CPU::get_curr_core_id());
    return r;
}

void spinlock_t::push_off(void) {
    uint64_t x   = CPU::READ_SSTATUS();
    int      old = (x & CPU::SSTATUS_SIE) != 0;
    CPU::DISABLE_INTR();
    if (SCHEDULER::curr_task[CPU::get_curr_core_id()] != nullptr) {
        if (SCHEDULER::curr_task[CPU::get_curr_core_id()]->noff == 0) {
            SCHEDULER::curr_task[CPU::get_curr_core_id()]->is_intr_enable = old;
        }
        SCHEDULER::curr_task[CPU::get_curr_core_id()]->noff += 1;
    }
    return;
}

void spinlock_t::pop_off(void) {
    // struct cpu *c = mycpu();
    uint64_t x   = CPU::READ_SSTATUS();
    int      old = (x & CPU::SSTATUS_SIE) != 0;
    if (old) {
        err("pop_off - interruptible\n");
    }
    if (SCHEDULER::curr_task[CPU::get_curr_core_id()] != nullptr) {
        if (SCHEDULER::curr_task[CPU::get_curr_core_id()]->noff < 1) {
            err("pop_off\n");
        }
        SCHEDULER::curr_task[CPU::get_curr_core_id()]->noff -= 1;
        if (SCHEDULER::curr_task[CPU::get_curr_core_id()]->noff == 0 &&
            SCHEDULER::curr_task[CPU::get_curr_core_id()]->is_intr_enable) {
            CPU::ENABLE_INTR();
        }
    }
    return;
}
