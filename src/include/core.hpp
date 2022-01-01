
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// core.hpp for Simple-XX/SimpleKernel.

#ifndef _CORE_HPP_
#define _CORE_HPP_

#include "stdint.h"
#include "stddef.h"
#include "task.h"

struct core_t {
    static const size_t CORE_MAX_COUNT = 8;
    core_t(void) {
        core_id   = -1;
        curr_task = nullptr;
        return;
    }
    core_t(size_t _core_id) {
        core_id   = _core_id;
        curr_task = nullptr;
    }
    // 当前 core id
    size_t core_id;
    // 此 core 上运行的进程
    task_t *curr_task;
};

// static core_t cores[core_t::CORE_MAX_COUNT];
// static size_t core_count = 0;

#endif /* _CORE_HPP_ */
