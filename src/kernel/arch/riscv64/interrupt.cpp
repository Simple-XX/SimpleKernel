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

Interrupt::InterruptFunc
    Interrupt::interrupt_handlers[Cpu::Xcause::kInterruptMaxCount];

Interrupt::InterruptFunc
    Interrupt::exception_handlers[Cpu::Xcause::kExceptionMaxCount];

static Interrupt interrupt __attribute__((init_priority(101)));

/// 中断处理入口 intr_s.S
extern "C" void TrapEntry() __attribute__((interrupt));
extern "C" void __alltraps() __attribute__((interrupt));

/**
 * @brief 中断处理函数
 * @param  xepc           值
 * @param  xtval          值
 * @param  xcause         值
 * @param  all_regs       保存在栈上的所有寄存器，实际上是 sp
 * @param  xie            值
 * @param  xstatus        值
 * @param  satp           值
 * @param  sscratch       值
 */
extern "C" void TrapHandler(uintptr_t xepc, uintptr_t xtval, Cpu::Xcause xcause,
                            Cpu::AllRegs *all_regs, Cpu::Xie xie,
                            Cpu::Xstatus xstatus, Cpu::Satp satp,
                            uintptr_t sscratch) {
  // 消除 unused 警告
  (void)xepc;
  (void)xtval;
  (void)xie;
  (void)xstatus;
  (void)satp;
  (void)sscratch;

  printf("sepc: 0x%X\n", Cpu::ReadSepc());

  printf("xepc: 0x%p, xtval: 0x%p, all_regs(sp): 0x%p\n", xepc, xtval,
         all_regs);
  // std::cout << "xstatus: " << xcause << std::endl;
  // std::cout << "xie: " << xcause << std::endl;
  // std::cout << "xstatus: " << xstatus << std::endl;
  // std::cout << "satp: " << satp << std::endl;
  // 跳转到对应的处理函数
  interrupt.Do((uint64_t)xcause.val_, (uint8_t *)all_regs);
}

Interrupt::Interrupt() {
  if (is_inited == false) {
    // 注册默认中断处理函数
    for (auto &i : interrupt_handlers) {
      i = [](uint64_t cause, uint8_t *context) -> uint64_t {
        printf("Default Interrupt handler [%s] 0x%X, 0x%p\n",
               Cpu::Xcause::kInterruptNames[cause], cause, context);
        return 0;
      };
    }
    // 注册默认异常处理函数
    for (auto &i : exception_handlers) {
      i = [](uint64_t cause, uint8_t *context) -> uint64_t {
        printf("Default Exception handler [%s] 0x%X, 0x%p\n",
               Cpu::Xcause::kInterruptNames[cause], cause, context);
        return 0;
      };
    }

    // 设置 trap vector
    // Cpu::SetStvecDirect((uint64_t)TrapEntry);
    Cpu::SetStvecDirect((uint64_t)__alltraps);

    // 开启 Supervisor 中断
    Cpu::EnableSupervisorIntr();

    // 开启内部中断
    // Cpu::EnableSupervisorSoftware();

    // 开启时钟中断
    Cpu::EnableSupervisorTimer();

    // 开启外部中断
    // Cpu::EnableSupervisorExternal();

    is_inited = true;
  }

  printf("Interrupt init.\n");
}

void Interrupt::Do(uint64_t cause, uint8_t *context) {
  auto xcause = Cpu::Xcause(cause);

  if (xcause.xcause_.interrupt) {
    // 中断
    if (xcause.xcause_.exception_code < Cpu::Xcause::kInterruptMaxCount) {
      interrupt_handlers[xcause.xcause_.exception_code](
          xcause.xcause_.exception_code, context);
    }
  } else {
    // 异常
    if (xcause.xcause_.exception_code < Cpu::Xcause::kExceptionMaxCount) {
      exception_handlers[xcause.xcause_.exception_code](
          xcause.xcause_.exception_code, context);
    }
  }
}

void Interrupt::RegisterInterruptFunc(uint64_t cause, InterruptFunc func) {
  auto xcause = Cpu::Xcause(cause);

  if (xcause.xcause_.interrupt) {
    if (xcause.xcause_.exception_code < Cpu::Xcause::kInterruptMaxCount) {
      interrupt_handlers[xcause.xcause_.exception_code] = func;
      printf("RegisterInterruptFunc [%s] 0x%X, 0x%p\n",
             Cpu::Xcause::kInterruptNames[xcause.xcause_.exception_code], cause,
             func);
    }
  } else {
    if (xcause.xcause_.exception_code < Cpu::Xcause::kExceptionMaxCount) {
      exception_handlers[xcause.xcause_.exception_code] = func;
      printf("RegisterInterruptFunc [%s] 0x%X, 0x%p\n",
             Cpu::Xcause::kExceptionNames[xcause.xcause_.exception_code], cause,
             func);
    }
  }
}

// 在 riscv64 情景下，argc 为启动核 id，argv 为 dtb 地址
/// @todo 从 dtb 读取 cpu 速度
uint32_t IntrInit(uint32_t argc, uint8_t *argv) {
  (void)argc;
  (void)argv;

  // 注册时钟中断
  interrupt.RegisterInterruptFunc(Cpu::Xcause::kBreakpoint,
                                  [](uint64_t, uint8_t *) -> uint64_t {
                                    printf("sss\n");
                                    static uint32_t count = 0;
                                    if (count++ == 5) {
                                      // while (1);
                                    }
                                    return 0;
                                  });

  // 设置时钟中断时间
  asm("ebreak");
  // sbi_set_timer(99999);

  printf("hello IntrInit\n");

  return 0;
}
