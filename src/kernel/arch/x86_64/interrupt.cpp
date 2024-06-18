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
#include "iostream"
#include "kernel_log.hpp"

// Interrupt::InterruptFunc
//     Interrupt::interrupt_handlers[cpu::cr::ScauseInfo::kInterruptMaxCount];

// Interrupt::InterruptFunc
//     Interrupt::exception_handlers[cpu::cr::ScauseInfo::kExceptionMaxCount];

// __attribute__((interrupt("supervisor"))) alignas(4) static void TarpEntry() {
// std::cout << std::endl;
// std::cout << "sepc: " << cpu::cr::kAllCsr.sepc << std::endl;
// std::cout << "stval: " << cpu::cr::kAllCsr.stval << std::endl;
// std::cout << "stvec: " << cpu::cr::kAllCsr.stvec << std::endl;
// std::cout << "scause: " << cpu::cr::kAllCsr.scause << std::endl;
// std::cout << "sie: " << cpu::cr::kAllCsr.sie << std::endl;
// std::cout << "sip: " << cpu::cr::kAllCsr.sip << std::endl;
// std::cout << "sstatus: " << cpu::cr::kAllCsr.sstatus << std::endl;
// std::cout << "satp: " << cpu::cr::kAllCsr.satp << std::endl;
// std::cout << "sscratch: " << cpu::cr::kAllCsr.sscratch << std::endl;

// kInterrupt.GetInstance().Do((uint64_t)cpu::cr::kAllCsr.scause.Read(),
//                             nullptr);
// }

Interrupt::Interrupt() {
  if (is_inited == false) {
    // 注册默认中断处理函数
    // for (auto &i : interrupt_handlers) {
    //   i = [](uint64_t cause, uint8_t *context) -> uint64_t {
    //     printf("Default Interrupt handler [%s] 0x%X, 0x%p\n",
    //            cpu::cr::ScauseInfo::kInterruptNames[cause], cause, context);
    //     return 0;
    //   };
    // }
    // // 注册默认异常处理函数
    // for (auto &i : exception_handlers) {
    //   i = [](uint64_t cause, uint8_t *context) -> uint64_t {
    //     printf("Default Exception handler [%s] 0x%X, 0x%p\n",
    //            cpu::cr::ScauseInfo::kExceptionNames[cause], cause, context);
    //     return 0;
    //   };
    // }

    // // 设置 trap vector
    // cpu::cr::kAllCsr.stvec.SetDirect((uint64_t)TarpEntry);

    // // 开启 Supervisor 中断
    // cpu::cr::kAllCsr.sstatus.sie.Set();

    // // 开启内部中断
    // cpu::cr::kAllCsr.sie.ssie.Set();

    // // 开启时钟中断
    // cpu::cr::kAllCsr.sie.stie.Set();

    // // 开启外部中断
    // cpu::cr::kAllCsr.sie.seie.Set();

    is_inited = true;
  }

  Info("Interrupt init.\n");
}

void Interrupt::Do(uint64_t cause, uint8_t *context) {
  // auto interrupt = cpu::cr::kAllCsr.scause.interrupt.Get(cause);
  // auto exception_code = cpu::cr::kAllCsr.scause.exception_code.Get(cause);

  // if (interrupt) {
  //   // 中断
  //   if (exception_code < cpu::cr::ScauseInfo::kInterruptMaxCount) {
  //     interrupt_handlers[exception_code](exception_code, context);
  //   }
  // } else {
  //   // 异常
  //   if (exception_code < cpu::cr::ScauseInfo::kExceptionMaxCount) {
  //     exception_handlers[exception_code](exception_code, context);
  //   }
  // }
}

void Interrupt::RegisterInterruptFunc(uint64_t cause, InterruptFunc func) {
  // auto interrupt = cpu::cr::kAllCsr.scause.interrupt.Get(cause);
  // auto exception_code = cpu::cr::kAllCsr.scause.exception_code.Get(cause);

  // if (interrupt) {
  //   if (exception_code < cpu::cr::ScauseInfo::kInterruptMaxCount) {
  //     interrupt_handlers[exception_code] = func;
  //     printf("RegisterInterruptFunc [%s] 0x%X, 0x%p\n",
  //            cpu::cr::ScauseInfo::kInterruptNames[exception_code], cause,
  //            func);
  //   }
  // } else {
  //   if (exception_code < cpu::cr::ScauseInfo::kExceptionMaxCount) {
  //     exception_handlers[exception_code] = func;
  //     printf("RegisterInterruptFunc [%s] 0x%X, 0x%p\n",
  //            cpu::cr::ScauseInfo::kExceptionNames[exception_code], cause,
  //            func);
  //   }
  // }
}

static uint64_t kInterval = 0;

// 在 riscv64 情景下，argc 为启动核 id，argv 为 dtb 地址
/// @todo 从 dtb 读取 cpu 速度
uint32_t InterruptInit(uint32_t argc, uint8_t *argv) {
  (void)argc;
  (void)argv;

  // 获取 cpu 速度
  kInterval = 100000;
  printf("kInterval: 0x%X\n", kInterval);

  std::cout << cpu::kAllCr.gdtr << std::endl;
  std::cout << cpu::kAllCr.gdtr.limit.Get() << std::endl;
  std::cout << cpu::kAllCr.gdtr.limit.Get(71) << std::endl;
  std::cout << cpu::kAllCr.gdtr.base.Get() << std::endl;
  std::cout << cpu::kAllCr.gdtr.base.Get(127855256) << std::endl;
  std::cout << cpu::kAllCr.rflags << std::endl;
  std::cout << cpu::kAllCr.cr0 << std::endl;
  std::cout << cpu::kAllCr.cr2 << std::endl;
  std::cout << cpu::kAllCr.cr3 << std::endl;
  std::cout << cpu::kAllCr.cr4 << std::endl;
  std::cout << cpu::kAllCr.cr8 << std::endl;

  // 注册时钟中断
  // kInterrupt.GetInstance().RegisterInterruptFunc(
  //     cpu::cr::ScauseInfo::kSupervisorTimerInterrupt,
  //     [](uint64_t exception_code, uint8_t *) -> uint64_t {
  //       sbi_set_timer(cpu::cr::kAllCsr.time.Read() + kInterval);
  //       printf("Handle %s\n",
  //              cpu::cr::ScauseInfo::kInterruptNames[exception_code]);
  //       return 0;
  //     });

  // ebreak 中断
  // kInterrupt.GetInstance().RegisterInterruptFunc(
  //     cpu::cr::ScauseInfo::kBreakpoint,
  //     [](uint64_t exception_code, uint8_t *) -> uint64_t {
  //       cpu::cr::kAllCsr.sepc.Write(cpu::cr::kAllCsr.sepc.Read() + 2);
  //       printf("Handle %s\n",
  //              cpu::cr::ScauseInfo::kExceptionNames[exception_code]);
  //       return 0;
  //     });

  // asm("ebreak");

  // 设置时钟中断时间
  // sbi_set_timer(kInterval);

  Info("Hello InterruptInit\n");

  return 0;
}
