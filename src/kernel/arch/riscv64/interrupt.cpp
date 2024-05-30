/**
 * @file interrupt.cpp
 * @brief 中断初始化
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2023-07-15
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2023-07-15<td>Zone.N (Zone.Niuzh@hotmail.com)<td>创建文件
 * </table>
 */

#include "interrupt.h"

#include "cpu.hpp"
#include "cstdio"
#include "fdt_parser.hpp"
#include "interrupt_base.h"
#include "opensbi_interface.h"

/// 中断处理入口 intr_s.S
extern "C" void TrapEntry();

/**
 * @brief 中断处理函数
 * @param  _scause         原因
 * @param  _sepc           值
 * @param  _stval          值
 * @param  _scause         值
 * @param  _all_regs       保存在栈上的所有寄存器，实际上是 sp
 * @param  _sie            值
 * @param  _sstatus        值
 * @param  _sscratch       值
 */
extern "C" void TrapHandler(uintptr_t _sepc, uintptr_t _stval,
                            uintptr_t _scause, Cpu::AllRegs *_all_regs,
                            uintptr_t _sie, Cpu::Sstatus _sstatus,
                            Cpu::Satp _satp, uintptr_t _sscratch) {
  // 消除 unused 警告
  (void)_sepc;
  (void)_stval;
  (void)_scause;
  (void)_all_regs;
  (void)_sie;
  (void)_sstatus;
  (void)_satp;
  (void)_sscratch;
  if (_scause & Cpu::kCauseIntrMask) {
    // 中断
    // 跳转到对应的处理函数
    //    INTR::get_instance().do_interrupt(_scause & Cpu::kCauseCodeMask, 0,
    //                                      nullptr);
    // 如果是时钟中断
    if ((_scause & Cpu::kCauseCodeMask) == Cpu::kIntrTimerSuperMode) {
      // 切换到内核线程
      //      switch_context(
      //          &core_t::get_curr_task()->context,
      //          &core_t::cores[Cpu::get_curr_core_id()].sched_task->context);
      printf("1111\n");
    }
  } else {
    // 异常
    printf("2222\n");
    // 跳转到对应的处理函数
    //    INTR::get_instance().do_excp(_scause & Cpu::kCauseCodeMask, 0,
    //    nullptr);
  }
}

// 在 riscv64 情景下，argc 为启动核 id，argv 为 dtb 地址
uint32_t IntrInit(uint32_t argc, uint8_t *argv) {
  printf("boot hart id: %d\n", argc);
  printf("dtb info addr: %p\n", argv);

  auto dtb_info = FDT_PARSER::fdt_parser((uintptr_t)argv);

  auto resource_mem = FDT_PARSER::resource_t();
  dtb_info.find_via_prefix("serial@", &resource_mem);

  // 设置 trap vector
  Cpu::WriteStvec((uintptr_t)TrapEntry);
  // 直接跳转到处理函数
  Cpu::SetStvecDirect();
  // 设置处理函数
  //  for (auto &i : interrupt_handlers) {
  //    i = handler_default;
  //  }
  //  for (auto &i : excp_handlers) {
  //    i = handler_default;
  //  }

  auto interrupt = Interrupt();

  sbi_set_timer(10);

  Cpu::EnableIntr();

  // 开启时钟中断
  Cpu::EnableTimer();

  printf("hello IntrInit\n");

  return 0;
}
