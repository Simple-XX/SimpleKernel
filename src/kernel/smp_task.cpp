
/**
 * @file smp_task.cpp
 * @brief 任务管理实现
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

#include "smp_task.h"
#include "rr_scheduler.h"
#include "fifo_scheduler.h"
#include "cfs_scheduler.h"
// 任务管理
// 初始化 调度 pid 开始 退出 等待 etc.
// 调度--调度器
//      获取下一个任务
//      安排任务运行在哪个 cpu 上
//

task_t *SMP_TASK::get_next_task(void) {
    task_t *ret = nullptr;
    for (auto i : schedulers) {
        ret = i->get_next_task();
        if (ret != nullptr) {
            break;
        }
    }
    return ret;
}

SMP_TASK::SMP_TASK(void) {
    return;
}

SMP_TASK::~SMP_TASK(void) {
    return;
}

SMP_TASK &SMP_TASK::get_instance(void) {
    static SMP_TASK smp_task_manager;
    return smp_task_manager;
}

bool SMP_TASK::init(void) {
    rr_scheduler_t   *rr_scheduler   = new rr_scheduler_t();
    fifo_scheduler_t *fifo_scheduler = new fifo_scheduler_t();
    cfs_scheduler_t  *cfs_scheduler  = new cfs_scheduler_t();
    schedulers.push_back((scheduler_t *)rr_scheduler);
    schedulers.push_back((scheduler_t *)fifo_scheduler);
    schedulers.push_back((scheduler_t *)cfs_scheduler);

    return true;
}
bool SMP_TASK::init_other_core(void) {
    //        scheduler->init_other_core();
    return true;
}
