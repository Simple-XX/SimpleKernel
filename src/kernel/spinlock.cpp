
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
#include "core.h"
#include "string"
#include "atomic"

bool spinlock_t::is_holding(void) {
    bool r = (locked._M_i && (hartid == CPU::get_curr_core_id()));
    return r;
}

void spinlock_t::push_off(void) {
    bool old = CPU::STATUS_INTR();

    CPU::DISABLE_INTR();

    if (core_t::cores[CPU::get_curr_core_id()].noff == 0) {
        core_t::cores[CPU::get_curr_core_id()].intr_enable = old;
    }
    core_t::cores[CPU::get_curr_core_id()].noff += 1;

    return;
}

void spinlock_t::pop_off(void) {
    if (CPU::STATUS_INTR() == true) {
        err("pop_off - interruptible\n");
    }

    if (core_t::cores[CPU::get_curr_core_id()].noff < 1) {
        err("pop_off\n");
    }
    core_t::cores[CPU::get_curr_core_id()].noff -= 1;

    if ((core_t::cores[CPU::get_curr_core_id()].noff == 0) &&
        (core_t::cores[CPU::get_curr_core_id()].intr_enable == true)) {
        CPU::ENABLE_INTR();
    }

    return;
}

spinlock_t::spinlock_t(void) {
    name = "unnamed";
    locked.clear();
    hartid = SIZE_MAX;
    return;
}

spinlock_t::spinlock_t(const char *_name) : name(_name) {
    locked.clear();
    hartid = CPU::get_curr_core_id();
    return;
}

spinlock_t::spinlock_t(const mystl::string &_name) : name(_name.c_str()) {
    locked.clear();
    hartid = CPU::get_curr_core_id();
    return;
}

bool spinlock_t::init(const char *_name) {
    name = _name;
    locked.clear();
    hartid = CPU::get_curr_core_id();
    return true;
}

/// @see https://gcc.gnu.org/onlinedocs/gcc/_005f_005fatomic-Builtins.html
void spinlock_t::lock(void) {
    push_off();
    if (is_holding() == true) {
        err("spinlock %s is_holding == true.\n", name);
    }
    size_t i = 0;
    while ((locked.test_and_set(std::memory_order_acquire)) &&
           (i++ < 0xFFFFFFFF)) {
        ;
    }
    if (i > 0xFFFFFFFF) {
        assert(!"Deadlock!");
    }

    std::atomic_signal_fence(std::memory_order_acquire);
    std::atomic_thread_fence(std::memory_order_acquire);

    hartid = CPU::get_curr_core_id();
    return;
}

void spinlock_t::unlock(void) {
    if (is_holding() == false) {
        err("spinlock %s is_holding == false.\n", name);
    }
    hartid = SIZE_MAX;

    std::atomic_signal_fence(std::memory_order_release);
    std::atomic_thread_fence(std::memory_order_release);

    locked.clear(std::memory_order_release);

    pop_off();
    return;
}
