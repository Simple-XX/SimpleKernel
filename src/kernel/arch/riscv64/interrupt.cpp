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

Clint::InterruptFunc Clint::interrupt_handlers[Cpu::kInterruptMaxCount];

Clint::InterruptFunc Clint::exception_handlers[Cpu::kExceptionMaxCount];

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
  (void)all_regs;
  (void)sie;
  (void)sstatus;
  (void)satp;
  (void)sscratch;
  // 跳转到对应的处理函数
  interrupt.clint.Do(scause, nullptr);
}

Clint::Clint() {
  if (is_inited == false) {  // 注册默认中断处理函数
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
    is_inited = true;
  }
  // 开启内部中断
  Cpu::WriteSie(Cpu::ReadSie() | Cpu::kSieSsie);
  printf("Clint init.\n");
}

uint32_t Clint::Do(uint32_t scause, uint8_t *context) {
  (void)context;
  auto is_interrupt = scause & Cpu::kCauseInterruptMask;
  auto cause = scause & Cpu::kCauseCodeMask;
  if (is_interrupt) {
    // 中断
    Interrupt(cause, nullptr);
  } else {
    // 异常
    // 跳转到对应的处理函数
    Exception(cause, nullptr);
  }
  return 0;
}

void Clint::RegisterInterruptFunc(uint32_t intr_no, InterruptFunc func) {
  printf("[%s] %d, 0x%p\n", Cpu::kInterruptNames[intr_no], intr_no, func);
}

uint32_t Clint::Interrupt(uint32_t cause, uint8_t *context) {
  printf("[%s] %d, 0x%p\n", Cpu::kInterruptNames[cause], cause, context);
  return 0;
}

uint32_t Clint::Exception(uint32_t cause, uint8_t *context) {
  printf("[%s] %d, 0x%p\n", Cpu::kInterruptNames[cause], cause, context);
  return 0;
}

// /**
//  * @brief 外部中断处理
//  */
// static int32_t external_intr(int, char **) {
//   // 读取中断号
//   auto no = PLIC::get_instance().get();
//   // 根据中断号判断设备
//   printf("external_intr: 0x%X.\n", no);
//   return 0;
// }

// PLIC::PLIC() {
//   // 映射 plic
//   resource_t resource = BOOT_INFO::get_plic();
//   base_addr = resource.mem.addr;
//   PLIC_PRIORITY = base_addr + 0x0;
//   PLIC_PENDING = base_addr + 0x1000;
//   for (uintptr_t a = resource.mem.addr;
//        a < resource.mem.addr + resource.mem.len; a += COMMON::PAGE_SIZE) {
//     VMM::get_instance().mmap(VMM::get_instance().get_pgd(), a, a,
//                              VMM_PAGE_READABLE | VMM_PAGE_WRITABLE);
//   }
//   // TODO: 多核情况下设置所有 hart
//   // 将当前 hart 的 S 模式优先级阈值设置为 0
//   IO::get_instance().write32((void *)PLIC_SPRIORITY(CPU::get_curr_core_id()),
//                              0);
//   // 注册外部中断处理函数
//   INTR::get_instance().register_interrupt_handler(CPU::INTR_EXTERN_S,
//                                                   external_intr);
//   // 开启外部中断
//   CPU::WRITE_SIE(CPU::READ_SIE() | CPU::SIE_SEIE);
//   info("plic init.\n");
//   return 0;
// }

// void PLIC::set(uint8_t _no, bool _status) {
//   spinlock.lock();
//   // 设置 IRQ 的属性为非零，即启用 plic
//   IO::get_instance().write32((void *)(base_addr + _no * 4), _status);
//   // TODO: 多核情况下设置所有 hart
//   // 为当前 hart 的 S 模式设置 uart 的 enable
//   if (_status) {
//     IO::get_instance().write32((void *)PLIC_SENABLE(CPU::get_curr_core_id()),
//                                IO::get_instance().read32((void
//                                *)PLIC_SENABLE(
//                                    CPU::get_curr_core_id())) |
//                                    (1 << _no));
//   } else {
//     IO::get_instance().write32((void *)PLIC_SENABLE(CPU::get_curr_core_id()),
//                                IO::get_instance().read32((void
//                                *)PLIC_SENABLE(
//                                    CPU::get_curr_core_id())) &
//                                    ~(1 << _no));
//   }
//   spinlock.unlock();
//   return;
// }

// uint8_t PLIC::get(void) {
//   spinlock.lock();
//   uint8_t ret =
//       IO::get_instance().read32((void
//       *)PLIC_SCLAIM(CPU::get_curr_core_id()));
//   spinlock.unlock();
//   return ret;
// }

// void PLIC::done(uint8_t _no) {
//   spinlock.lock();
//   IO::get_instance().write32((void *)PLIC_SCLAIM(CPU::get_curr_core_id()),
//   _no); spinlock.unlock(); return;
// }

// void PLIC::register_externel_handler(
//     uint8_t _no, externel_interrupt_handler_t _interrupt_handler) {
//   externel_interrupt_handlers[_no] = _interrupt_handler;
//   return;
// }

// void PLIC::do_externel_interrupt(uint8_t _no) {
//   externel_interrupt_handlers[_no](_no);
//   return;
// }

Interrupt::Interrupt() {
  if (is_inited == false) {
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

uint32_t Interrupt::Do(uint32_t intr_no, uint8_t *context) {
  printf("[%s] %d, 0x%p\n", Cpu::kInterruptNames[intr_no], intr_no, context);
  return 0;
}

void Interrupt::RegisterInterruptFunc(uint32_t intr_no, InterruptFunc func) {
  printf("[%s] %d, 0x%p\n", Cpu::kInterruptNames[intr_no], intr_no, func);
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
