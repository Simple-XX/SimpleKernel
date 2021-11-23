
/**
 * @file intr.cpp
 * @brief 中断抽象
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2021-09-18
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2021-09-18<td>digmouse233<td>迁移到 doxygen
 * </table>
 */

#include "cpu.hpp"
#include "stdio.h"
#include "intr.h"
#include "cpu.hpp"

/**
 * @brief 中断处理函数
 * @param  _scause         原因
 * @param  _sepc           值
 * @param  _stval          值
 */
extern "C" void trap_handler(uint64_t _scause, uint64_t _sepc,
                             uint64_t _stval) {

    // 消除 unused 警告
    (void)_sepc;
    (void)_stval;
#define DEBUG
#ifdef DEBUG
    printf("scause: 0x%p, sepc: 0x%p, stval: 0x%p.\n", _scause, _sepc, _stval);
#undef DEBUG
#endif
    if (_scause & CPU::CAUSE_INTR_MASK) {
// 中断
// #define DEBUG
#ifdef DEBUG
        printf("intr: %s.\n",
               CLINT::intr_names[_scause & CPU::CAUSE_CODE_MASK]);
#undef DEBUG
#endif
        // 跳转到对应的处理函数
        CLINT::do_interrupt(_scause & CPU::CAUSE_CODE_MASK);
    }
    else {
// 异常
// 跳转到对应的处理函数
// #define DEBUG
#ifdef DEBUG
        printf("excp: %s.\n",
               CLINT::excp_names[_scause & CPU::CAUSE_CODE_MASK]);
#undef DEBUG
#endif
        CLINT::do_excp(_scause & CPU::CAUSE_CODE_MASK);
    }
    return;
}

/**
 * @brief 默认使用的中断处理函数
 */
void handler_default(void) {
    while (1) {
        ;
    }
    return;
}

int32_t INTR::init(void) {
    // 内部中断初始化
    CLINT::init();
    // 外部中断初始化
    // PLIC::init();
    // 设置时钟中断
    // TIMER::init();
    info("intr init.\n");
    return 0;
}
