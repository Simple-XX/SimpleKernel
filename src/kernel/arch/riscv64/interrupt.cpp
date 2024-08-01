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
#include "kernel_fdt.hpp"
#include "kernel_log.hpp"
#include "opensbi_interface.h"

Interrupt::InterruptFunc Interrupt::interrupt_handlers
    [cpu::reginfo::csr::ScauseInfo::kInterruptMaxCount];

Interrupt::InterruptFunc Interrupt::exception_handlers
    [cpu::reginfo::csr::ScauseInfo::kExceptionMaxCount];

__attribute__((interrupt("supervisor"))) alignas(4) static void TarpEntry() {
  std::cout << std::endl;
  std::cout << "sepc: " << cpu::kAllCsr.sepc << std::endl;
  std::cout << "stval: " << cpu::kAllCsr.stval << std::endl;
  std::cout << "stvec: " << cpu::kAllCsr.stvec << std::endl;
  std::cout << "scause: " << cpu::kAllCsr.scause << std::endl;
  std::cout << "sie: " << cpu::kAllCsr.sie << std::endl;
  std::cout << "sip: " << cpu::kAllCsr.sip << std::endl;
  std::cout << "sstatus: " << cpu::kAllCsr.sstatus << std::endl;
  std::cout << "satp: " << cpu::kAllCsr.satp << std::endl;
  std::cout << "sscratch: " << cpu::kAllCsr.sscratch << std::endl;

  kInterrupt.GetInstance().Do((uint64_t)cpu::kAllCsr.scause.Read(), nullptr);
}

Interrupt::Interrupt() {
  if (is_inited == false) {
    // 注册默认中断处理函数
    for (auto &i : interrupt_handlers) {
      i = [](uint64_t cause, uint8_t *context) -> uint64_t {
        printf("Default Interrupt handler [%s] 0x%X, 0x%p\n",
               cpu::reginfo::csr::ScauseInfo::kInterruptNames[cause], cause,
               context);
        return 0;
      };
    }
    // 注册默认异常处理函数
    for (auto &i : exception_handlers) {
      i = [](uint64_t cause, uint8_t *context) -> uint64_t {
        printf("Default Exception handler [%s] 0x%X, 0x%p\n",
               cpu::reginfo::csr::ScauseInfo::kExceptionNames[cause], cause,
               context);
        return 0;
      };
    }

    // 设置 trap vector
    cpu::kAllCsr.stvec.SetDirect((uint64_t)TarpEntry);

    // 开启 Supervisor 中断
    cpu::kAllCsr.sstatus.sie.Set();

    // 开启内部中断
    cpu::kAllCsr.sie.ssie.Set();

    // 开启时钟中断
    cpu::kAllCsr.sie.stie.Set();

    // 开启外部中断
    cpu::kAllCsr.sie.seie.Set();

    is_inited = true;
  }

  log::Info("Interrupt init.\n");
}

void Interrupt::Do(uint64_t cause, uint8_t *context) {
  auto interrupt = cpu::kAllCsr.scause.interrupt.Get(cause);
  auto exception_code = cpu::kAllCsr.scause.exception_code.Get(cause);

  if (interrupt) {
    // 中断
    if (exception_code < cpu::reginfo::csr::ScauseInfo::kInterruptMaxCount) {
      interrupt_handlers[exception_code](exception_code, context);
    }
  } else {
    // 异常
    if (exception_code < cpu::reginfo::csr::ScauseInfo::kExceptionMaxCount) {
      exception_handlers[exception_code](exception_code, context);
    }
  }
}

void Interrupt::RegisterInterruptFunc(uint64_t cause, InterruptFunc func) {
  auto interrupt = cpu::kAllCsr.scause.interrupt.Get(cause);
  auto exception_code = cpu::kAllCsr.scause.exception_code.Get(cause);

  if (interrupt) {
    if (exception_code < cpu::reginfo::csr::ScauseInfo::kInterruptMaxCount) {
      interrupt_handlers[exception_code] = func;
      printf("RegisterInterruptFunc [%s] 0x%X, 0x%p\n",
             cpu::reginfo::csr::ScauseInfo::kInterruptNames[exception_code],
             cause, func);
    }
  } else {
    if (exception_code < cpu::reginfo::csr::ScauseInfo::kExceptionMaxCount) {
      exception_handlers[exception_code] = func;
      printf("RegisterInterruptFunc [%s] 0x%X, 0x%p\n",
             cpu::reginfo::csr::ScauseInfo::kExceptionNames[exception_code],
             cause, func);
    }
  }
}

static uint64_t kInterval = 0;

uint32_t InterruptInit(uint32_t, uint8_t *) {
  // 获取 cpu 速度
  kInterval = kKernelFdt.GetInstance().GetTimebaseFrequency();
  printf("kInterval: 0x%X\n", kInterval);

  // 注册时钟中断
  kInterrupt.GetInstance().RegisterInterruptFunc(
      cpu::reginfo::csr::ScauseInfo::kSupervisorTimerInterrupt,
      [](uint64_t exception_code, uint8_t *) -> uint64_t {
        sbi_set_timer(cpu::kAllCsr.time.Read() + kInterval);
        printf("Handle %s\n",
               cpu::reginfo::csr::ScauseInfo::kInterruptNames[exception_code]);
        return 0;
      });

  // ebreak 中断
  kInterrupt.GetInstance().RegisterInterruptFunc(
      cpu::reginfo::csr::ScauseInfo::kBreakpoint,
      [](uint64_t exception_code, uint8_t *) -> uint64_t {
        cpu::kAllCsr.sepc.Write(cpu::kAllCsr.sepc.Read() + 2);
        printf("Handle %s\n",
               cpu::reginfo::csr::ScauseInfo::kExceptionNames[exception_code]);
        return 0;
      });

  asm("ebreak");

  // 设置时钟中断时间
  sbi_set_timer(kInterval);

  log::Info("Hello InterruptInit\n");

  return 0;
}
