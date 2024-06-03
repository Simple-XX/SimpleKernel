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

static __attribute__((interrupt("supervisor"))) void TarpEntry() {
  auto sepc = Cpu::ReadSepc();
  auto stval = Cpu::ReadStval();
  auto scause = Cpu::ReadScause();
  auto sp = uint64_t(0);
  auto sie = Cpu::ReadSie();
  auto sstatus = Cpu::ReadSstatus();
  auto satp = Cpu::Satp();
  auto sscratch = Cpu::ReadSscratch();

  std::cout << "scause: " << scause << std::endl;
  std::cout << "sie: " << sie << std::endl;
  std::cout << "sstatus: " << sstatus << std::endl;
  std::cout << "satp: " << satp << std::endl;
  printf("sepc: 0x%p, stval: 0x%p, all_regs(sp): 0x%p, sscratch: 0x%p\n", sepc,
         stval, sp, sscratch);

  interrupt.Do((uint64_t)scause.val_, (uint8_t *)sp);

  printf("Done\n");
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
    Cpu::SetStvecDirect((uint64_t)TarpEntry);

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
  interrupt.RegisterInterruptFunc(Cpu::Xcause::kSupervisorTimerInterrupt,
                                  [](uint64_t, uint8_t *) -> uint64_t {
                                    printf("ttt\n");
                                    static uint32_t count = 0;
                                    if (count++ == 5) {
                                      while (1);
                                    }
                                    return 0;
                                  });

  // ebreak 中断
  interrupt.RegisterInterruptFunc(Cpu::Xcause::kBreakpoint,
                                  [](uint64_t, uint8_t *) -> uint64_t {
                                    Cpu::WriteSepc(Cpu::ReadSepc() + 2);
                                    return 0;
                                  });

  // 设置时钟中断时间
  asm("ebreak");

  // sbi_set_timer(99999);

  printf("hello IntrInit\n");

  return 0;
}
