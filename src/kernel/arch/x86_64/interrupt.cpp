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

#include "arch.h"
#include "cpu.hpp"
#include "kernel_log.hpp"
#include "sk_cstdio"
#include "sk_iostream"

Interrupt::InterruptFunc
    Interrupt::interrupt_handlers[cpu::reginfo::IdtrInfo::kInterruptMaxCount];

cpu::reginfo::IdtrInfo::Idt
    Interrupt::idts[cpu::reginfo::IdtrInfo::kInterruptMaxCount];

/**
 * @brief 中断处理函数
 * @tparam no 中断号
 * @param interrupt_context 中断上下文，根据中断不同可能是 InterruptContext 或
 * InterruptContextErrorCode
 */
template <uint8_t no>
__attribute__((target("general-regs-only")))
__attribute__((interrupt)) static void
TarpEntry(uint8_t *interrupt_context) {
  kInterrupt.GetInstance().Do(no, interrupt_context);
}

template <uint8_t no>
void Interrupt::SetUpIdtr() {
  if constexpr (no < cpu::reginfo::IdtrInfo::kInterruptMaxCount - 1) {
    idts[no] = cpu::reginfo::IdtrInfo::Idt(
        (uint64_t)TarpEntry<no>, 8, 0x0,
        cpu::reginfo::IdtrInfo::Idt::Type::k64BitInterruptGate,
        cpu::reginfo::IdtrInfo::Idt::DPL::kRing0,
        cpu::reginfo::IdtrInfo::Idt::P::kPresent);
    SetUpIdtr<no + 1>();
  } else {
    // 写入 idtr
    static auto idtr = cpu::reginfo::IdtrInfo::Idtr{
        .limit = sizeof(cpu::reginfo::IdtrInfo::Idtr) *
                     cpu::reginfo::IdtrInfo::kInterruptMaxCount -
                 1,
        .base = idts,
    };
    cpu::kAllCr.idtr.Write(idtr);

    // 输出 idtr 信息
    sk_std::cout << cpu::kAllCr.idtr << sk_std::endl;
    for (size_t i = 0; i < (cpu::kAllCr.idtr.Read().limit + 1) /
                               sizeof(cpu::reginfo::IdtrInfo::Idtr);
         i++) {
      printf("idtr[%d] 0x%p\n", i, cpu::kAllCr.idtr.Read().base + i);
      sk_std::cout << *(cpu::kAllCr.idtr.Read().base + i) << sk_std::endl;
    }
  }
}

Interrupt::Interrupt()
    : pic(cpu::reginfo::IdtrInfo::kIrq0, cpu::reginfo::IdtrInfo::kIrq8),
      pit(200) {
  if (is_inited == false) {
    // 注册默认中断处理函数
    for (auto &i : interrupt_handlers) {
      i = [](uint64_t cause, uint8_t *context) -> uint64_t {
        log::Info("Default Interrupt handler [%s] 0x%X, 0x%p\n",
                  cpu::reginfo::IdtrInfo::kInterruptNames[cause], cause,
                  context);
        DumpStack();
        while (1);
      };
    }

    // 初始化 idtr
    SetUpIdtr();

    // 初始化 loacl apic

    // 初始化 io apic

    is_inited = true;
  }

  log::Info("Interrupt init.\n");
}

void Interrupt::Do(uint64_t cause, uint8_t *context) {
  if (cause < cpu::reginfo::IdtrInfo::kInterruptMaxCount) {
    interrupt_handlers[cause](cause, context);
  }
}

void Interrupt::RegisterInterruptFunc(uint64_t cause, InterruptFunc func) {
  if (cause < cpu::reginfo::IdtrInfo::kInterruptMaxCount) {
    interrupt_handlers[cause] = func;
    printf("RegisterInterruptFunc [%s] 0x%X, 0x%p\n",
           cpu::reginfo::IdtrInfo::kInterruptNames[cause], cause, func);
  }
}

uint32_t InterruptInit(uint32_t, uint8_t *) {
  sk_std::cout << cpu::kAllCr.efer << sk_std::endl;
  sk_std::cout << cpu::kAllCr.gdtr << sk_std::endl;
  sk_std::cout << cpu::kAllCr.idtr << sk_std::endl;
  sk_std::cout << cpu::kAllCr.rflags << sk_std::endl;
  sk_std::cout << cpu::kAllCr.cr0 << sk_std::endl;
  sk_std::cout << cpu::kAllCr.cr2 << sk_std::endl;
  sk_std::cout << cpu::kAllCr.cr3 << sk_std::endl;
  sk_std::cout << cpu::kAllCr.cr4 << sk_std::endl;
  sk_std::cout << cpu::kAllCr.cr8 << sk_std::endl;

  // 初始化中断
  kInterrupt.GetInstance();

  // 注册时钟中断
  kInterrupt.GetInstance().RegisterInterruptFunc(
      cpu::reginfo::IdtrInfo::kIrq0,
      [](uint64_t exception_code, uint8_t *) -> uint64_t {
        kInterrupt.GetInstance().pit.Ticks();
        if (kInterrupt.GetInstance().pit.GetTicks() % 100 == 0) {
          printf("Handle %d %s\n", exception_code,
                 cpu::reginfo::IdtrInfo::kInterruptNames[exception_code]);
        }
        kInterrupt.GetInstance().pic.Clear(exception_code);
        return 0;
      });

  // 允许时钟中断
  kInterrupt.GetInstance().pic.Enable(cpu::reginfo::IdtrInfo::kIrq0);
  // 开启中断
  cpu::kAllCr.rflags.interrupt_enable_flag.Set();

  log::Info("Hello InterruptInit\n");

  return 0;
}
