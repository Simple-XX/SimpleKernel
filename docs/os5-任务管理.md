# 任务管理

​	对于计算机来说，最重要的就是处理任务和计算，所以最重要的就是就是对于每个运行的任务进行调度和管理。 在操作系统中，每个任务由进程或线程的形式来进行管理，通过进程控制块PCB（process control block）。在代码中，src\arch\x86_64\task\task.c 中是进程线程的相关代码，主要包括对。

该代码块是进程控制块的实现：

```c
// 进程控制块 PCB
typedef
    struct task_pcb {
	// 任务状态
	volatile task_status_t		status;
	// 任务的 pid
	pid_t pid;
	// 任务名称
	char * name;
	// 当前任务运行时间
	uint32_t run_time;
	// 父进程指针
	struct task_pcb *		parent;
	// 任务的内存信息
	task_mem_t * mm;
	// 任务中断保存的寄存器信息
	pt_regs_t * pt_regs;
	// 任务切换上下文信息
	task_context_t *		context;
	// 任务的退出代码
	int32_t	exit_code;
} task_pcb_t;
```

一个进程控制块主要包括任务状态，任务PID，内存信息以及上下文。可以从中看出，一个进程的主要包含的部分就是内存信息以及运行的指令（在上下文中体现）。但是这里只是描述了进程，那么线程和进程的区别是什么呢，在linux 中，线程就是轻量级的进程，没有大量的空间用于存储数据。此外，也可以看到，每个进程都会记录其父进程，所以进程是以树状的形式进行管理。操作系统也是一个进程，运行在内核空间的进程。

其中最重要的三个结构体的就是task_mem_t 、pt_regs_t和task_context_t，pt_regs_t分别会在中断章节进行介绍。下图为任务上下文结构体的

```c
typedef
    struct task_context {
	uint32_t	eip;
	uint32_t	esp;
	uint32_t	ebp;
	uint32_t	ebx;
	uint32_t	ecx;
	uint32_t	edx;
	uint32_t	esi;
	uint32_t	edi;
} task_context_t;
```

进程内存地址结构，这里就是

```c
// 进程内存地址结构
typedef
    struct task_mem {
	// 进程页表
	pgd_t *		pgd_dir;
	// 栈顶
	ptr_t		stack_top;
	// 栈底
	ptr_t		stack_bottom;
	// 内存起点
	ptr_t		task_start;
	// 代码段起止
	ptr_t		code_start;
	ptr_t		code_end;
	// 数据段起止
	ptr_t		data_start;
	ptr_t		data_end;
	// 内存结束
	ptr_t		task_end;
} task_mem_t;
```

<img src=".\images\memory.png" alt="memory" style="zoom:80%;" />

可以结合上图来看task_mem中的变量就是对应着上图中一个进程的各种段的起止节点位置。



下图所示为simplekernel中一些用于管理任务的数据结构：

```c
// 当前任务指针
task_pcb_t * curr_task = NULL;
// 当前任务数量
static uint32_t curr_task_count = 0;
// 全局 pid 值
static pid_t curr_pid = 0;
// 全部任务链表
ListEntry * task_list = NULL;
// 可调度进程链表
ListEntry * runnable_list = NULL;
// 等待进程链表
ListEntry * wait_list = NULL;
```

主要包括用于进程调度的相关链表。





