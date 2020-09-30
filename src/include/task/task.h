
// This file is a part of MRNIU/SimpleKernel
// (https://github.com/MRNIU/SimpleKernel).
//
// task.h for MRNIU/SimpleKernel.

#ifndef _TASK_H_
#define _TASK_H_

#ifdef __cplusplus
extern "C" {
#endif
// 关于任务管理的资料见 intel 手册 3ACh7
#include "stdint.h"
#include "types.h"
#include "mem/pmm.h"
#include "mem/vmm.h"
#include "intr/include/intr.h"
#include "linkedlist.h"

// 最长任务名
#define TASK_NAME_MAX 64
// 最大任务数量
#define TASK_MAX 128
// 任务栈大小
#define TASK_STACK_SIZE KERNEL_STACK_SIZE

// 进程状态描述
typedef enum task_status {
    // 未初始化
    TASK_UNINIT = 0,
    // 睡眠中
    TASK_SLEEPING = 1,
    // 可运行
    TASK_RUNNABLE = 2,
    // 正在运行
    TASK_RUNNING = 3,
    // 僵尸状态
    TASK_ZOMBIE = 4,
} task_status_t;

// 内核线程的上下文切换保存的信息
// Saved registers for kernel context switches.
// Don't need to save all the segment registers (%cs, etc),
// because they are constant across kernel contexts.
// Don't need to save %eax, %ecx, %edx, because the
// x86 convention is that the caller has saved them.
// Contexts are stored at the bottom of the stack they
// describe; the stack pointer is the address of the context.
// The layout of the context must match the code in swtch.S.
typedef struct task_context {
    uint32_t eip;
    uint32_t esp;
    uint32_t ebp;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
    uint32_t esi;
    uint32_t edi;
} task_context_t;

// 进程内存地址结构
typedef struct task_mem {
    // 进程页表
    pgd_t *pgd_dir;
    // 栈顶
    ptr_t stack_top;
    // 栈底
    ptr_t stack_bottom;
    // 内存起点
    ptr_t task_start;
    // 代码段起止
    ptr_t code_start;
    ptr_t code_end;
    // 数据段起止
    ptr_t data_start;
    ptr_t data_end;
    // 内存结束
    ptr_t task_end;
} task_mem_t;

// 进程控制块 PCB
typedef struct task_pcb {
    // 任务状态
    volatile task_status_t status;
    // 任务的 pid
    pid_t pid;
    // 任务名称
    char *name;
    // 当前任务运行时间
    uint32_t run_time;
    // 父进程指针
    struct task_pcb *parent;
    // 任务的内存信息
    task_mem_t *mm;
    // 任务中断保存的寄存器信息
    pt_regs_t *pt_regs;
    // 任务切换上下文信息
    task_context_t *context;
    // 任务的退出代码
    int32_t exit_code;
} task_pcb_t;

// 全部任务链表
extern ListEntry *task_list;
// 可调度进程链表
extern ListEntry *runnable_list;
// 等待进程链表
extern ListEntry *wait_list;
// 当前任务指针
extern task_pcb_t *curr_task;
// 内核线程入口函数 tasl_s.s
extern int32_t kthread_entry(void *args);
// intr_s.s
extern void forkret_s233(pt_regs_t *pt_regs);

// 初始化
void task_init(void);
// 获取正在运行的进程
task_pcb_t *get_current_task(void);
// 创建内核线程
pid_t kernel_thread(int32_t (*fun)(void *), void *args, uint32_t flags);
// 线程退出函数
void kthread_exit(void);
// 进程切换硬件上下文处理
task_pcb_t *__switch_to(task_pcb_t *curr, task_pcb_t *next)
    __attribute__((regparm(3)));
// 将进程放入进程队列
pid_t do_fork(uint32_t flags, pt_regs_t *pt_regs);
// 将进程移除进程队列
void do_exit(int32_t exit_code);
// 线程创建
pid_t kfork(int (*fn)(void *), void *arg);
// 线程退出
void kexit(void);
// 设置进程名
int32_t set_task_name(pid_t pid, char *name);

// 从 pid 获取进程结构体
task_pcb_t *get_task(pid_t pid);
// 显示指定 pid 进程的 mem 信息
void show_task_mem(pid_t pid);
// 显示指定 pid 进程的 pt_regs 信息
void show_task_pt_regs(pid_t pid);
// 显示指定 pid 进程的 context 信息
void show_task_context(pid_t pid);
// 显示指定 pid 进程信息，pid 为 TASK_MAX+1 时，显示所有进程信息
void show_task(pid_t pid);
// 显示目前运行进程信息
void show_curr_task(void);

int32_t kexec();
int32_t kwait();
void    kwakeup();
void    kkill();

inline void print_curr(task_context_t *curr) {
    printk_debug("curr 0x%08X\t", curr);
    printk_debug("curr->eip 0x%08X\t", curr->eip);
    printk_debug("curr->esp 0x%08X\t", curr->esp);
    printk_debug("curr->ebp 0x%08X\t", curr->ebp);
    printk_debug("curr->ebx 0x%08X\t", curr->ebx);
    printk_debug("curr->ecx 0x%08X\t", curr->ecx);
    printk_debug("curr->edx 0x%08X\t", curr->edx);
    printk_debug("curr->esi 0x%08X\t", curr->esi);
    printk_debug("curr->edi 0x%08X\n", curr->edi);
    return;
}

inline void print_next(task_context_t *next) {
    printk_debug("next 0x%08X\t", next);
    printk_debug("next->eip 0x%08X\t", next->eip);
    printk_debug("next->esp 0x%08X\t", next->esp);
    printk_debug("next->ebp 0x%08X\t", next->ebp);
    printk_debug("next->ebx 0x%08X\t", next->ebx);
    printk_debug("next->ecx 0x%08X\t", next->ecx);
    printk_debug("next->edx 0x%08X\t", next->edx);
    printk_debug("next->esi 0x%08X\t", next->esi);
    printk_debug("next->edi 0x%08X\n", next->edi);
    return;
}

#ifdef __cplusplus
}
#endif

#endif /* _TASK_H_ */
