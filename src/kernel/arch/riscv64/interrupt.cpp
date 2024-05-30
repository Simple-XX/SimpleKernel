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
#include "opensbi_interface.h"

static Interrupt interrupt;

/// 中断处理入口 intr_s.S
extern "C" void TrapEntry();

/**
 * @brief 中断处理函数
 * @param  sepc           值
 * @param  stval          值
 * @param  scause         值
 * @param  all_regs       保存在栈上的所有寄存器，实际上是 sp
 * @param  sie            值
 * @param  sstatus        值
 * @param  satp           值
 * @param  sscratch       值
 */
extern "C" void TrapHandler(uintptr_t sepc, uintptr_t stval, uintptr_t scause,
                            Cpu::AllRegs *all_regs, uintptr_t sie,
                            Cpu::Sstatus sstatus, Cpu::Satp satp,
                            uintptr_t sscratch) {
  // 消除 unused 警告
  (void)sepc;
  (void)stval;
  (void)scause;
  (void)all_regs;
  (void)sie;
  (void)sstatus;
  (void)satp;
  (void)sscratch;
  auto is_interrupt = scause & Cpu::kCauseInterruptMask;
  auto cause = scause & Cpu::kCauseCodeMask;
  if (is_interrupt) {
    // 中断
    // 跳转到对应的处理函数
    interrupt.DoInterrupt(cause, nullptr);
  } else {
    // 异常
    // 跳转到对应的处理函数
    interrupt.DoException(cause, nullptr);
  }
}

Clint::Clint() {
  // 开启内部中断
  Cpu::WriteSie(Cpu::ReadSie() | Cpu::kSieSsie);
  printf("Clint init.\n");
}

Interrupt::Interrupt() {
  if (is_inited == false) {
    // 注册默认中断处理函数
    for (auto &i : interrupt_handlers) {
      i = [](uint32_t argc, uint8_t *argv) -> uint32_t {
        printf("Default Interrupt Handler: %d, 0x%p\n", argc, argv);
        return 0;
      };
    }
    // 注册默认异常处理函数
    for (auto &i : exception_handlers) {
      i = [](uint32_t argc, uint8_t *argv) -> uint32_t {
        printf("Default Exception Handler: %d, 0x%p\n", argc, argv);
        return 0;
      };
    }

    // 设置 trap vector
    Cpu::WriteStvec((uintptr_t)TrapEntry);

    // 直接跳转到处理函数
    Cpu::SetStvecDirect();

    // 开启 Supervisor 中断
    Cpu::EnableSupervisorIntr();

    // 开启时钟中断
    Cpu::EnableSupervisorTimer();

    is_inited = true;
  }

  // 初始化本地中断
  Clint();
  printf("Interrupt init.\n");
}

uint32_t Interrupt::DoInterrupt(uint32_t intr_no, uint8_t *context) {
  printf("[%s] %d, 0x%p\n", Cpu::kInterruptNames[intr_no], intr_no, context);
  return 0;
}

uint32_t Interrupt::DoException(uint32_t excp_no, uint8_t *context) {
  printf("[%s] %d, 0x%p\n", Cpu::kExceptionNames[excp_no], excp_no, context);
  return 0;
}

// 在 riscv64 情景下，argc 为启动核 id，argv 为 dtb 地址
uint32_t IntrInit(uint32_t argc, uint8_t *argv) {
  printf("boot hart id: %d\n", argc);
  printf("dtb info addr: %p\n", argv);

  auto dtb_info = FDT_PARSER::fdt_parser((uintptr_t)argv);

  auto resource_mem = FDT_PARSER::resource_t();
  dtb_info.find_via_prefix("serial@", &resource_mem);

  // 设置时钟中断时间
  sbi_set_timer(1000);

  printf("hello IntrInit\n");

  return 0;
}
