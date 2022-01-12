
/**
 * @file core.cpp
 * @brief core 抽象实现
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2022-01-07
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2022-01-07<td>MRNIU<td>迁移到 doxygen
 * </table>
 */

#include "core.h"
#include "common.h"

core_t core_t::cores[COMMON::CORES_COUNT];

core_t::core_t(void) {
    core_id     = SIZE_MAX;
    curr_task   = nullptr;
    sched_task  = nullptr;
    noff        = 0;
    intr_enable = false;
    return;
}

void core_t::set_curr_task(task_t *_task) {
    cores[CPU::get_curr_core_id()].curr_task = _task;
    return;
}

task_t *core_t::get_curr_task(void) {
    return cores[CPU::get_curr_core_id()].curr_task;
}

std::ostream &operator<<(std::ostream &_os, const core_t &_core) {
    printf("core id: 0x%X, curr task: %s, sched task: %s, noff: 0x%X, intr "
           "enable: %s.",
           _core.core_id,
           (_core.curr_task == nullptr ? ("null")
                                       : (_core.curr_task->name.c_str())),
           (_core.sched_task == nullptr ? ("null")
                                        : (_core.sched_task->name.c_str())),
           _core.noff, (_core.intr_enable == true ? ("true") : ("false")));
    return _os;
}
