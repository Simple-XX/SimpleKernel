# 任务管理

​	对于计算机来说，最重要的就是处理任务和计算，所以最重要的就是就是对于每个运行的任务进行调度和管理。 在操作系统中，每个任务由进程或线程的形式来进行管理，进程是操作系统中最重要的概念，而且也是能够将操作系统很多模块串联起来的一个重要节点，在通常的操作系统的书籍都是从进程开始讲起，可见其重要性。在操作系统中，每一个任务都是一个进程或线程，这就是任务的表现形式，而当我们有多个进程同时运行时，就会要考虑到CPU是如何对进程进行调度和管理的，这就是我们后面也会提到的进程调度算法。

###### 进程资源

当我们计算机需要完成一个任务时，就需要一定的空间来存储运行的代码，有时候也要需要一定空间去读取和存储数据，此外也要有一个ID用于标识每个进程，所以进程中就会有很多的数据来对进程的资源以及数据管理。

<img src=".\images\process.jpg" alt="process" style="zoom:60%;" />



**进程切换**

对于进程而言，重要的机制就是多进程同时运行，这就需要我们在运行程序的过程中让CPU去切换进程，在切换的同时就能够实现并发的运行程序，让每个进程轮流去占用CPU运行这个算法我们称之为**时间片轮转算法**。但是在切换过程需要操作系统进入更高的权限才可以进行，这时就需要进行中断，在中断机制触发后，进程A状态就会被保护起来，然后执行我们 的进程调度机制，另外一个进程B就会被调入运行。这就是一个中断过程的流程。

1. 进程A运行，ring1切换到ring0

2. 发生时钟中断，时钟中断程序启动

3. 进程调度，将进程B调入运行

4. 进程B恢复，ring0切换到ring1

5. 进程B运行

   <img src=".\images\process state.jpg" alt="process" style="zoom:100%;" />

那么实现这个功能就需要完成的

- 时钟中断程序

- 进程调度模块

- 进程的结构体实现

  

###### 时钟中断程序

我们切换进程时首要做的就是保存进程的状态，这样在下一次切换回来时就可以方便恢复上一次运行的状态，而每个进程最重要的就是各个寄存器的状态，这里使用堆栈去保存比较方便，push命令用来保存状态，pop用来恢复前一状态。我们用来恢复保存进程状态的一个结构叫做进程表，相当于一个进程的提纲，通过进程表我们能够很方便的管理进程。

<img src=".\images\process table.jpg" alt="process" style="zoom:80%;" />

上图所示为一个进程表，主要有进程管理，存储管理和文件管理。

一个进程主要是四个模块，进程表、进程体、TSS和GDT，在我们前面以及有对进程表TSS和GDT进行过介绍，下面我来看一下进程体，进程体主要体现形式就是这个进程控制块PCB（process control block）.

通过进程控制块。在代码中，src\arch\x86_64\task\task.c 中是进程线程的相关代码，主要包括对于进程的函数，以及一些用于管理进程的链表结构。

如下图所示为一些进程控制结构，包括当前任务指针也就是当前CPU运行的任务，当前任务数量即有多少个进程存在，全局pid，全部的任务链表，可调度进程链表以及等待进程链表。

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

每个进程都是由pcb所表示，下面所示代码块是进程控制块的实现：

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





