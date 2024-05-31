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

Interrupt::InterruptFunc Interrupt::interrupt_handlers[Cpu::kInterruptMaxCount];

Interrupt::InterruptFunc Interrupt::exception_handlers[Cpu::kExceptionMaxCount];

static Interrupt interrupt __attribute__((init_priority(101)));

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
  (void)sie;
  (void)sstatus;
  (void)satp;
  (void)sscratch;
  printf(
      "sepc: 0x%p, stval: 0x%p, scause: 0x%p, all_regs(sp): 0x%p, sie: "
      "0x%p\nsstatus: ",
      sepc, stval, scause, all_regs, sie);
  // std::cout << sstatus << ", \nsatp: " << satp << ", \n";
  // printf("sscratch: 0x%p\n", sscratch);
  // 跳转到对应的处理函数
  interrupt.Do(scause, (uint8_t *)all_regs);
}

Interrupt::Interrupt() {
  if (is_inited == false) {
    // 注册默认中断处理函数
    for (auto &i : interrupt_handlers) {
      i = [](uint64_t cause, uint8_t *context) -> uint64_t {
        printf("[%s] %d, 0x%p\n", Cpu::kInterruptNames[cause], cause, context);
        return 0;
      };
    }
    // 注册默认异常处理函数
    for (auto &i : exception_handlers) {
      i = [](uint64_t cause, uint8_t *context) -> uint64_t {
        printf("[%s] %d, 0x%p\n", Cpu::kInterruptNames[cause], cause, context);
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

  // 开启内部中断
  Cpu::WriteSie(Cpu::ReadSie() | Cpu::kSieSsie);
  printf("Interrupt init.\n");
}

void Interrupt::Do(uint64_t scause, uint8_t *context) {
  auto is_interrupt = scause & Cpu::kCauseInterruptMask;
  auto cause = scause & Cpu::kCauseCodeMask;
  if (is_interrupt) {
    // 中断
    DoInterrupt(cause, context);
  } else {
    // 异常
    DoException(cause, context);
  }
}

void Interrupt::RegisterInterruptFunc(uint64_t scause, InterruptFunc func) {
  auto is_interrupt = scause & Cpu::kCauseInterruptMask;
  auto cause = scause & Cpu::kCauseCodeMask;
  if (is_interrupt) {
    if (cause < Cpu::kInterruptMaxCount) {
      interrupt_handlers[cause] = func;
      printf("[%s] %d, 0x%p\n", Cpu::kInterruptNames[cause], cause, func);
    }
  } else {
    if (cause < Cpu::kExceptionMaxCount) {
      exception_handlers[cause] = func;
      printf("[%s] %d, 0x%p\n", Cpu::kExceptionNames[cause], cause, func);
    }
  }
}

void Interrupt::DoInterrupt(uint64_t cause, uint8_t *context) {
  interrupt_handlers[cause](cause, context);
}

void Interrupt::DoException(uint64_t cause, uint8_t *context) {
  exception_handlers[cause](cause, context);
}

// 在 riscv64 情景下，argc 为启动核 id，argv 为 dtb 地址
uint32_t IntrInit(uint32_t argc, uint8_t *argv) {
  printf("boot hart id: %d\n", argc);
  printf("dtb info addr: %p\n", argv);

  // 注册时钟中断
  interrupt.RegisterInterruptFunc(
      Cpu::kIntrTimerSuperMode | Cpu::kCauseCodeMask,
      [](uint64_t, uint8_t *) -> uint64_t {
        printf("sss\n");
        while (1);
        sbi_set_timer(10000000);
        return 0;
      });

  // 设置时钟中断时间
  sbi_set_timer(10000000);

  printf("hello IntrInit\n");

  return 0;
}
