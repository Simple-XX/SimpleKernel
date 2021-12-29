
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// task.h for Simple-XX/SimpleKernel.

#ifndef _TASK_H_
#define _TASK_H_

#include "stdint.h"
#include "types.h"
#include "stdio.h"
#include "string"
#include "iostream"
#include "vector"
#include "queue"
#include "spinlock.h"
#include "vmm.h"
#include "cpu.hpp"

// 任务状态
enum task_status_t : uint8_t {
    // 未使用
    UNUSED,
    // 睡眠
    SLEEPING,
    // 可运行
    RUNNABLE,
    // 运行中
    RUNNING,
    // 僵尸进程
    ZOMBIE,
};

// 任务管理抽象
class TASK {
public:
    // 任务抽象
    struct task_t {
        // 线程名
        mystl::string name;
        // 线程 id
        pid_t pid;
        // 进程状态
        enum task_status_t state;
        // 父进程
        task_t *parent;
        // 上下文
        CPU::context_t context;
        // 线程栈
        uintptr_t stack;
        // 进程页目录
        pt_t page_dir;
        // 本次运行时间片
        size_t slice;
        // 已运行时间片
        size_t slice_total;
        task_t(mystl::string _name = "", void (*_task)(void) = nullptr);
        size_t hartid;
        // Depth of push_off() nesting.
        int noff = 0;
        // Were interrupts enabled before push_off()?
        bool is_intr_enable;
        ~task_t(void);
    };

    // 当前任务
    static task_t *curr_task[CPU::CPUS];
    // 任务向量
    static mystl::queue<task_t *> *task_queue;
    // 全局 pid
    static pid_t   g_pid;
    static task_t *task_os;
    // 分配 pid
    static pid_t alloc_pid(void);
    // 回收 pid
    static void free_pid(pid_t _pid);

protected:
public:
    // 初始化
    static bool init(void);
    static bool init_other_core(void);
    // 获取当前任务
    static task_t *get_curr_task(void);
    // 退出
    static void exit(uint32_t _exit_code);
    // task_t 输出
    friend std::ostream &operator<<(std::ostream &_os, const task_t &_task);
};

#endif /* _TASK_H_ */
