# 数据结构

在操作系统中需要用到很多数据结构，其中最常用的就是链表，链表一般是内核中最常用最普通简单的数据结构，一般为包括单向链表和双向链表。链表是一种能够存放和操作可变数量元素的数据结构，其最大优势在于能够动态插入和删除，在进程管理中通过使用链表来管理进程。

<img src="./images/double link list.png" alt="双向链表"  />

这里可以看到在src\ds_alg的目录下有双向链表描述，_ListEntry为节点的描述node，其中包含三个成员，分别是data，prev和next。其中data用于存储数据内容，prev和next分别用于对节点进行操作。对于双向链表的操作方式可以参考其中代码实现。下面用进程的管理来描述下双线链表在操作系统中的应用。

举例：在进程中对进程的管理需要通过进程process control block(pcb)这个结构体来进行管理，参考文件src\include\task\task.h中task_pcb结构体，而这个**进程控制块是通过双向链表来进行操作**，可以打开这个文件看:

```c
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

这个task_pcb结构体中有进程的状态，进程pid，进程名，进程运行时间，以及他的父进程指针，内存信息，中断保存的寄存器信息，以及任务切换上下文信息以及任务退出代码。而在进程控制块中有两个毕竟重要的结构体，task_context_t和task_mem_t。