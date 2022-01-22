
/**
 * @file rr_scheduler.cpp
 * @brief round-robin 调度器实现
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2022-01-22
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2022-01-22<td>MRNIU<td>新建文件
 * </table>
 */

#include "rr_scheduler.h"
#include "core.h"

rr_scheduler_t::rr_scheduler_t(void) : scheduler_t("unnamed rr_scheduler_t") {
    return;
}

rr_scheduler_t::rr_scheduler_t(const mystl::string _name) : scheduler_t(_name) {
    return;
}

rr_scheduler_t::~rr_scheduler_t(void) {
    return;
}

void rr_scheduler_t::add_task(task_t *_task) {
    // 更新任务状态
    _task->state = RUNNING;
    // 将新进程添加到队列
    task_queue.push(_task);
    return;
}

void rr_scheduler_t::remove_task(task_t *_task) {
    (void)_task;
    info("TODO\n");
    return;
}

task_t *rr_scheduler_t::get_next_task(void) {
    task_t *ret = nullptr;
    // 如果任务未结束
    if (core_t::get_curr_task()->state == RUNNING) {
        // 如果不是 os 线程
        if (core_t::get_curr_task()->pid != 0) {
            // 重新加入队列
            task_queue.push(core_t::get_curr_task());
        }
    }
    // 任务队列不为空的话弹出一个任务
    if (task_queue.empty() == false) {
        ret = task_queue.front();
        task_queue.pop();
    }
    return ret;
}
