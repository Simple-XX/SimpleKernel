
/**
 * @file smp_task.h
 * @brief 任务管理
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

#ifndef _SMP_TASK_H_
#define _SMP_TASK_H_

#include "scheduler.h"
#include "tmp_scheduler.h"
#include "fifo_scheduler.h"
#include "rr_scheduler.h"
#include "task.h"
#include "common.h"
#include "cpu.hpp"
#include "core.h"

// 任务管理
// 初始化 调度 pid 开始 退出 等待 etc.
// 调度--调度器
//      获取下一个任务
//      安排任务运行在哪个 cpu 上
//

class SMP_TASK {
private:
    /// 任务调度器队列
    /// 调度优先级 rt->fifo->cfs->idle
    mystl::vector<scheduler_t *> schedulers;

    task_t* get_next_task(void);

protected:
public:
    SMP_TASK(void);
    ~SMP_TASK(void);

    static SMP_TASK &get_instance(void);

    bool init(void);
    bool init_other_core(void);


};

#endif // _SMP_TASK_H_
