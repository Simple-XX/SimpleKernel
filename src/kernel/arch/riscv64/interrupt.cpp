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
#include "cstdio"
#include "fdt_parser.hpp"
#include "iostream"
#include "opensbi_interface.h"

Interrupt::InterruptFunc
    Interrupt::interrupt_handlers[cpu::csr::ScauseInfo::kInterruptMaxCount];

Interrupt::InterruptFunc
    Interrupt::exception_handlers[cpu::csr::ScauseInfo::kExceptionMaxCount];

static Interrupt interrupt __attribute__((init_priority(101)));

__attribute__((interrupt("supervisor"))) alignas(4) static void TarpEntry() {
  std::cout << "sepc: " << cpu::csr::kAllCsr.sepc << std::endl;
  std::cout << "stval: " << cpu::csr::kAllCsr.stval << std::endl;
  std::cout << "stvec: " << cpu::csr::kAllCsr.stvec << std::endl;
  std::cout << "scause: " << cpu::csr::kAllCsr.scause << std::endl;
  std::cout << "sie: " << cpu::csr::kAllCsr.sie << std::endl;
  std::cout << "sstatus: " << cpu::csr::kAllCsr.sstatus << std::endl;
  std::cout << "satp: " << cpu::csr::kAllCsr.satp << std::endl;
  std::cout << "sscratch: " << cpu::csr::kAllCsr.sscratch << std::endl;

  // printf("sepc: 0x%p, stval: 0x%p, all_regs(sp): 0x%p, sscratch: 0x%p\n",
  // sepc,
  //        stval, sp, sscratch);

  interrupt.Do((uint64_t)cpu::csr::kAllCsr.scause.Read(), nullptr);

  printf("Done\n");
}

Interrupt::Interrupt() {
  if (is_inited == false) {
    // 注册默认中断处理函数
    for (auto &i : interrupt_handlers) {
      i = [](uint64_t cause, uint8_t *context) -> uint64_t {
        printf("Default Interrupt handler [%s] 0x%X, 0x%p\n",
               cpu::csr::ScauseInfo::kInterruptNames[cause], cause, context);
        return 0;
      };
    }
    // 注册默认异常处理函数
    for (auto &i : exception_handlers) {
      i = [](uint64_t cause, uint8_t *context) -> uint64_t {
        printf("Default Exception handler [%s] 0x%X, 0x%p\n",
               cpu::csr::ScauseInfo::kInterruptNames[cause], cause, context);
        return 0;
      };
    }

    // 设置 trap vector
    cpu::csr::kAllCsr.stvec.SetDirect((uint64_t)TarpEntry);

    // 开启 Supervisor 中断
    cpu::csr::kAllCsr.sstatus.sie.Set();

    // 开启内部中断
    cpu::csr::kAllCsr.sie.ssie.Set();

    // 开启时钟中断
    cpu::csr::kAllCsr.sie.stie.Set();

    // 开启外部中断
    cpu::csr::kAllCsr.sie.seie.Set();

    cpu::csr::kAllCsr.sstatus.spp.Set();

    std::cout << "ctor sepc: " << cpu::csr::kAllCsr.sepc << std::endl;
    std::cout << "ctor stval: " << cpu::csr::kAllCsr.stval << std::endl;
    std::cout << "ctor stvec: " << cpu::csr::kAllCsr.stvec << std::endl;
    std::cout << "ctor scause: " << cpu::csr::kAllCsr.scause << std::endl;
    std::cout << "ctor sie: " << cpu::csr::kAllCsr.sie << std::endl;
    std::cout << "ctor sstatus: " << cpu::csr::kAllCsr.sstatus << std::endl;
    std::cout << "ctor satp: " << cpu::csr::kAllCsr.satp << std::endl;
    std::cout << "ctor sscratch: " << cpu::csr::kAllCsr.sscratch << std::endl;

    is_inited = true;
  }

  printf("Interrupt init.\n");
}

void Interrupt::Do(uint64_t cause, uint8_t *context) {
  auto interrupt = cpu::csr::kAllCsr.scause.interrupt.Get(cause);
  auto exception_code = cpu::csr::kAllCsr.scause.exception_code.Get(cause);

  if (interrupt) {
    // 中断
    if (exception_code < cpu::csr::ScauseInfo::kInterruptMaxCount) {
      interrupt_handlers[exception_code](exception_code, context);
    }
  } else {
    // 异常
    if (exception_code < cpu::csr::ScauseInfo::kExceptionMaxCount) {
      exception_handlers[exception_code](exception_code, context);
    }
  }
}

void Interrupt::RegisterInterruptFunc(uint64_t cause, InterruptFunc func) {
  auto interrupt = cpu::csr::kAllCsr.scause.interrupt.Get(cause);
  auto exception_code = cpu::csr::kAllCsr.scause.exception_code.Get(cause);

  if (interrupt) {
    if (exception_code < cpu::csr::ScauseInfo::kInterruptMaxCount) {
      interrupt_handlers[exception_code] = func;
      printf("RegisterInterruptFunc [%s] 0x%X, 0x%p\n",
             cpu::csr::ScauseInfo::kInterruptNames[exception_code], cause,
             func);
    }
  } else {
    if (exception_code < cpu::csr::ScauseInfo::kExceptionMaxCount) {
      exception_handlers[exception_code] = func;
      printf("RegisterInterruptFunc [%s] 0x%X, 0x%p\n",
             cpu::csr::ScauseInfo::kExceptionNames[exception_code], cause,
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
  interrupt.RegisterInterruptFunc(
      cpu::csr::ScauseInfo::kSupervisorTimerInterrupt,
      [](uint64_t, uint8_t *) -> uint64_t {
        static uint32_t count = 0;
        if (count++ == 5) {
          while (1);
        }
        return 0;
      });

  // ebreak 中断
  interrupt.RegisterInterruptFunc(
      cpu::csr::ScauseInfo::kBreakpoint, [](uint64_t, uint8_t *) -> uint64_t {
        printf("Handle ebreak.\n");
        printf("Handle %s\n", cpu::csr::ScauseInfo::kInterruptNames
                                  [cpu::csr::ScauseInfo::kBreakpoint]);
        cpu::csr::kAllCsr.sepc.Write(cpu::csr::kAllCsr.sepc.Read() + 2);
        return 0;
      });

  asm("ebreak");

  // 设置时钟中断时间
  // sbi_set_timer(99999);

  printf("hello IntrInit\n");

  cpu::csr::kAllCsr.sstatus.spp.Set();

  std::cout << "sepc: " << cpu::csr::kAllCsr.sepc << std::endl;
  std::cout << "stval: " << cpu::csr::kAllCsr.stval << std::endl;
  std::cout << "stvec: " << cpu::csr::kAllCsr.stvec << std::endl;
  std::cout << "scause: " << cpu::csr::kAllCsr.scause << std::endl;
  std::cout << "sie: " << cpu::csr::kAllCsr.sie << std::endl;
  std::cout << "sstatus: " << cpu::csr::kAllCsr.sstatus << std::endl;
  std::cout << "satp: " << cpu::csr::kAllCsr.satp << std::endl;
  std::cout << "sscratch: " << cpu::csr::kAllCsr.sscratch << std::endl;

  return 0;
}
