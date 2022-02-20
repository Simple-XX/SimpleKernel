
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

extern "C" void context_init(CPU::context_t *_context);
extern "C" void switch_context(CPU::context_t *_old, CPU::context_t *_new);
extern "C" void switch_os(CPU::context_t *_os);

/// idle 任务指针
task_t *idle_task[COMMON::CORES_COUNT] = {0};
/**
 * @brief idle 任务
 */
void idle(void) {
    while (1) {
#if defined(__riscv)
        asm("wfi");
#elif defined(__i386__) || defined(__x86_64__)
        asm("hlt");
#elif defined(__arm__) || defined(__aarch64__)
        (void);
#endif
    }
    // 不会执行到这里
    assert(0);
    return;
}

task_t *SMP_TASK::get_next_task(void) {
    task_t *ret = nullptr;
    // 遍历所有调度器
    for (auto i : schedulers) {
        ret = i->get_next_task();
        if (ret != nullptr) {
            break;
        }
    }
    // 如果没有可运行任务
    if (ret == nullptr) {
        // 运行 idle
        ret = idle_task[CPU::get_curr_core_id()];
    }
    return ret;
}

pid_t SMP_TASK::alloc_pid(void) {
    pid_t res = g_pid++;
    return res;
}

void SMP_TASK::free_pid(pid_t _pid) {
    _pid = _pid;
    return;
}

void SMP_TASK::switch_task(void) {
//#define DEBUG
#ifdef DEBUG
    info("switch_task\n");
#undef DEBUG
#endif
    // 获取下一个线程并替换为当前线程下一个线程
    auto tmp = get_next_task();
    // 设置 core 当前线程信息
    core_t::set_curr_task(tmp);
    // 切换
    switch_context(&core_t::cores[CPU::get_curr_core_id()].sched_task->context,
                   &core_t::get_curr_task()->context);
    return;
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
    // 初始化自旋锁
    spinlock.init("smp_task");
    // 按照优先级创建并插入调度器
    rr_scheduler_t   *rr_scheduler   = new rr_scheduler_t();
    fifo_scheduler_t *fifo_scheduler = new fifo_scheduler_t();
    cfs_scheduler_t  *cfs_scheduler  = new cfs_scheduler_t();
    schedulers.push_back((scheduler_t *)rr_scheduler);
    schedulers.push_back((scheduler_t *)fifo_scheduler);
    schedulers.push_back((scheduler_t *)cfs_scheduler);

    // 当前进程
    task_t *task = new task_t("init", nullptr);
    task->hartid = CPU::get_curr_core_id();
    task->pid    = 0;
    task->state  = RUNNING;
    // 原地跳转，填充启动进程的 task_t 信息
    context_init(&task->context);
    // 初始化 core 信息
    core_t::cores[CPU::get_curr_core_id()].core_id    = CPU::get_curr_core_id();
    core_t::cores[CPU::get_curr_core_id()].curr_task  = task;
    core_t::cores[CPU::get_curr_core_id()].sched_task = task;

    // 创建 idle 任务
    idle_task[CPU::get_curr_core_id()]        = new task_t("idle", idle);
    idle_task[CPU::get_curr_core_id()]->state = RUNNING;
    idle_task[CPU::get_curr_core_id()]->context.sstatus =
        task->context.sstatus | CPU::SSTATUS_SIE;
    idle_task[CPU::get_curr_core_id()]->context.sie =
        task->context.sie | CPU::SIE_STIE;
    idle_task[CPU::get_curr_core_id()]->context.sip = task->context.sip;
    info("smp_task init.\n");
    return true;
}

bool SMP_TASK::init_other_core(void) {
    // 当前进程
    task_t *task = new task_t("init other", nullptr);
    task->hartid = CPU::get_curr_core_id();
    task->pid    = 0;
    task->state  = RUNNING;
    // 原地跳转，填充启动进程的 task_t 信息
    context_init(&task->context);
    // 初始化 core 信息
    core_t::cores[CPU::get_curr_core_id()].core_id    = CPU::get_curr_core_id();
    core_t::cores[CPU::get_curr_core_id()].curr_task  = task;
    core_t::cores[CPU::get_curr_core_id()].sched_task = task;
    // 创建 idle 任务
    idle_task[CPU::get_curr_core_id()]        = new task_t("idle", idle);
    idle_task[CPU::get_curr_core_id()]->state = RUNNING;
    idle_task[CPU::get_curr_core_id()]->context.sstatus =
        task->context.sstatus | CPU::SSTATUS_SIE;
    idle_task[CPU::get_curr_core_id()]->context.sie =
        task->context.sie | CPU::SIE_STIE;
    idle_task[CPU::get_curr_core_id()]->context.sip = task->context.sip;

    info("smp_task other init.\n");
    return true;
}

bool SMP_TASK::add_task(task_t                    &_task,
                        SMP_TASK::scheduler_type_t _scheduler_type) {
    spinlock.lock();
    _task.pid = alloc_pid();
    schedulers.at(_scheduler_type)->add_task(&_task);
    spinlock.unlock();
    return true;
}

void SMP_TASK::remove_task(task_t &_task) {
    spinlock.lock();
    // 回收 pid
    free_pid(_task.pid);
    spinlock.unlock();
    return;
}

void SMP_TASK::sched(void) {
    // TODO: 根据当前任务的属性进行调度
//#define DEBUG
#ifdef DEBUG
    info("sched\n");
#undef DEBUG
#endif
    switch_task();
    return;
}

void SMP_TASK::exit(uint32_t _exit_code) {
    core_t::get_curr_task()->exit_code = _exit_code;
    core_t::get_curr_task()->state     = ZOMBIE;
    switch_os(&core_t::cores[CPU::get_curr_core_id()].sched_task->context);
    assert(0);
    return;
}

extern "C" void exit(int _status) {
    SMP_TASK::get_instance().exit(_status);
}
