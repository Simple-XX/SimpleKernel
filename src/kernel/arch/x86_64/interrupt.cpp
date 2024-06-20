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

Interrupt::InterruptFunc
    Interrupt::interrupt_handlers[cpu::reginfo::IdtrInfo::kInterruptMaxCount];

cpu::reginfo::IdtrInfo::Idt
    Interrupt::idts[cpu::reginfo::IdtrInfo::kInterruptMaxCount];

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

void set_idt(cpu::reginfo::IdtrInfo::Idt *idt, uint64_t base, uint16_t selector,
             uint8_t ist, uint8_t type, uint8_t dpl, uint8_t p) {
  idt->idt.offset1 = base & 0xFFFF;
  idt->idt.selector = selector;
  idt->idt.ist = ist;
  idt->idt.zero0 = 0;
  idt->idt.type = type;
  idt->idt.zero1 = 0;
  idt->idt.dpl = dpl;
  idt->idt.p = p;
  idt->idt.offset2 = (base >> 16) & 0xFFFF;
  idt->idt.offset3 = base >> 32;
  idt->idt.reserved = 0;
  return;
}

Interrupt::Interrupt() {
  if (is_inited == false) {
    // 注册默认中断处理函数
    for (auto &i : interrupt_handlers) {
      i = [](uint64_t cause, uint8_t *context) -> uint64_t {
        printf("Default Interrupt handler [%s] 0x%X, 0x%p\n",
               cpu::reginfo::IdtrInfo::kInterruptNames[cause], cause, context);
        return 0;
      };
    }

    // 初始化 idts
    // for (auto &i : idts)
    for (size_t i = 0; i < cpu::reginfo::IdtrInfo::kInterruptMaxCount; i++) {
      set_idt(&idts[i], (uint64_t)&interrupt_handlers, 8, 0x0, 0xE, 0, 1);
    }

    static auto idtr = cpu::reginfo::IdtrInfo::Idtr{};
    idtr.limit = cpu::reginfo::IdtrInfo::kInterruptMaxCount;
    idtr.base = idts;
    printf("idtr: 0x%p\n", &idtr);
    printf("idtr.limit: %d\n", idtr.limit);
    printf("idtr.base: 0x%p\n", idtr.base);
    cpu::kAllCr.idtr.Write(idtr);

    std::cout << cpu::kAllCr.idtr << std::endl;
    // while(1);

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

  std::cout << cpu::kAllCr.efer << std::endl;
  std::cout << cpu::kAllCr.gdtr << std::endl;
  std::cout << cpu::kAllCr.idtr << std::endl;
  std::cout << cpu::kAllCr.rflags << std::endl;
  std::cout << cpu::kAllCr.cr0 << std::endl;
  std::cout << cpu::kAllCr.cr2 << std::endl;
  std::cout << cpu::kAllCr.cr3 << std::endl;
  std::cout << cpu::kAllCr.cr4 << std::endl;
  std::cout << cpu::kAllCr.cr8 << std::endl;

  kInterrupt.GetInstance();

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
