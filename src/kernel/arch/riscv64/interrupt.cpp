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

#include <typeinfo>

#include "cpu.hpp"
#include "csr.hpp"
#include "cstdio"
#include "fdt_parser.hpp"
#include "iostream"
#include "opensbi_interface.h"

Interrupt::InterruptFunc
    Interrupt::interrupt_handlers[Scause::kInterruptMaxCount];

Interrupt::InterruptFunc
    Interrupt::exception_handlers[Scause::kExceptionMaxCount];

static Interrupt interrupt __attribute__((init_priority(101)));

static __attribute__((interrupt("supervisor"))) void TarpEntry() {
  std::cout << "sepc: " << kAllCsr.sepc << std::endl;
  std::cout << "stval: " << kAllCsr.stval << std::endl;
  std::cout << "stvec: " << kAllCsr.stvec << std::endl;
  std::cout << "scause: " << kAllCsr.scause << std::endl;
  std::cout << "sie: " << kAllCsr.sie << std::endl;
  std::cout << "sstatus: " << kAllCsr.sstatus << std::endl;
  std::cout << "satp: " << kAllCsr.satp << std::endl;
  std::cout << "sscratch: " << kAllCsr.sscratch << std::endl;

  // printf("sepc: 0x%p, stval: 0x%p, all_regs(sp): 0x%p, sscratch: 0x%p\n",
  // sepc,
  //        stval, sp, sscratch);

  interrupt.Do((uint64_t)kAllCsr.scause.Read(), nullptr);

  printf("Done\n");
}

Interrupt::Interrupt() {
  if (is_inited == false) {
    // 注册默认中断处理函数
    for (auto &i : interrupt_handlers) {
      i = [](uint64_t cause, uint8_t *context) -> uint64_t {
        printf("Default Interrupt handler [%s] 0x%X, 0x%p\n",
               Scause::kInterruptNames[cause], cause, context);
        return 0;
      };
    }
    // 注册默认异常处理函数
    for (auto &i : exception_handlers) {
      i = [](uint64_t cause, uint8_t *context) -> uint64_t {
        printf("Default Exception handler [%s] 0x%X, 0x%p\n",
               Scause::kInterruptNames[cause], cause, context);
        return 0;
      };
    }

    // 设置 trap vector
    kAllCsr.stvec.SetDirect((uint64_t)TarpEntry);

    // 开启 Supervisor 中断
    kAllCsr.sstatus.sie.Set();

    // 开启内部中断
    kAllCsr.sie.ssie.Set();

    // 开启时钟中断
    kAllCsr.sie.stie.Set();

    // 开启外部中断
    kAllCsr.sie.seie.Set();

    is_inited = true;
  }

  printf("Interrupt init.\n");
}

void Interrupt::Do(uint64_t cause, uint8_t *context) {
  auto interrupt = kAllCsr.scause.interrupt.Get(cause);
  auto exception_code = kAllCsr.scause.exception_code.Get(cause);

  if (interrupt) {
    // 中断
    if (exception_code < Scause::kInterruptMaxCount) {
      interrupt_handlers[exception_code](exception_code, context);
    }
  } else {
    // 异常
    if (exception_code < Scause::kExceptionMaxCount) {
      exception_handlers[exception_code](exception_code, context);
    }
  }
}

void Interrupt::RegisterInterruptFunc(uint64_t cause, InterruptFunc func) {
  auto interrupt = kAllCsr.scause.interrupt.Get(cause);
  auto exception_code = kAllCsr.scause.exception_code.Get(cause);

  if (interrupt) {
    if (exception_code < Scause::kInterruptMaxCount) {
      interrupt_handlers[exception_code] = func;
      printf("RegisterInterruptFunc [%s] 0x%X, 0x%p\n",
             Scause::kInterruptNames[exception_code], cause, func);
    }
  } else {
    if (exception_code < Scause::kExceptionMaxCount) {
      exception_handlers[exception_code] = func;
      printf("RegisterInterruptFunc [%s] 0x%X, 0x%p\n",
             Scause::kExceptionNames[exception_code], cause, func);
    }
  }
}

// 在 riscv64 情景下，argc 为启动核 id，argv 为 dtb 地址
/// @todo 从 dtb 读取 cpu 速度
uint32_t IntrInit(uint32_t argc, uint8_t *argv) {
  (void)argc;
  (void)argv;

  // 注册时钟中断
  interrupt.RegisterInterruptFunc(Scause::kSupervisorTimerInterrupt,
                                  [](uint64_t, uint8_t *) -> uint64_t {
                                    printf("ttt\n");
                                    static uint32_t count = 0;
                                    if (count++ == 5) {
                                      while (1);
                                    }
                                    return 0;
                                  });

  // ebreak 中断
  interrupt.RegisterInterruptFunc(Scause::kBreakpoint,
                                  [](uint64_t, uint8_t *) -> uint64_t {
                                    kAllCsr.sepc.Write(kAllCsr.sepc.Read() + 2);
                                    return 0;
                                  });

  // 设置时钟中断时间
  // asm("ebreak");

  // sbi_set_timer(99999);

  printf("hello IntrInit\n");

  kAllCsr.sstatus.spp.Set();

  std::cout << "sepc: " << kAllCsr.sepc << std::endl;
  std::cout << "stval: " << kAllCsr.stval << std::endl;
  std::cout << "stvec: " << kAllCsr.stvec << std::endl;
  std::cout << "scause: " << kAllCsr.scause << std::endl;
  std::cout << "sie: " << kAllCsr.sie << std::endl;
  std::cout << "sstatus: " << kAllCsr.sstatus << std::endl;
  std::cout << "satp: " << kAllCsr.satp << std::endl;
  std::cout << "sscratch: " << kAllCsr.sscratch << std::endl;

  return 0;
}
