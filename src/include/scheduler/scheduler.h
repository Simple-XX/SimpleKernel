
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// scheduler.h for Simple-XX/SimpleKernel.

#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_

#include "stdint.h"
#include "stdio.h"
#include "string"
#include "iostream"
#include "vector"
#include "queue"
#include "task.h"
#include "spinlock.h"

// 调度器抽象
class SCHEDULER {
private:
    static TASK::task_t *get_next_task(void);
    static void          switch_task(void);

protected:
public:
    static void sched(void);
};

extern void idle(void);

#endif /* _SCHEDULER_H_ */
