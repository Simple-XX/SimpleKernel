
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

task_t *core_t::sched_task = nullptr;
core_t  core_t::cores[COMMON::CORES_COUNT];

core_t::core_t(void) {
    core_id     = COMMON::get_curr_core_id(CPU::READ_SP());
    curr_task   = nullptr;
    sched_task  = nullptr;
    noff        = 0;
    intr_enable = false;
    return;
}

void core_t::set_curr_task(task_t *_task) {
    cores[COMMON::get_curr_core_id(CPU::READ_SP())].curr_task = _task;
    return;
}

task_t *core_t::get_curr_task(void) {
    return cores[COMMON::get_curr_core_id(CPU::READ_SP())].curr_task;
}
