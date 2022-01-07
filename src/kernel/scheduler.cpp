
/**
 * @file scheduler.h
 * @brief 调度器抽象类实现
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2022-01-01
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2022-01-01<td>MRNIU<td>迁移到 doxygen
 * </table>
 */

#include "scheduler.h"

SCHEDULER::SCHEDULER(const char *_name) : name(_name) {
    g_pid = 1;
    return;
}

SCHEDULER::~SCHEDULER(void) {
    return;
}
