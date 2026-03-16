/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#include <cpu_io.h>
#include <opensbi_interface.h>

#include "arch.h"
#include "basic_info.hpp"
#include "interrupt.h"
#include "kernel.h"
#include "kernel_fdt.hpp"
#include "kernel_log.hpp"
#include "kstd_cstdio"
#include "ns16550a/ns16550a.hpp"
#include "syscall.hpp"
#include "task_manager.hpp"
#include "virtio/virtio_driver.hpp"
#include "virtual_memory.hpp"

namespace {
using Ns16550aSingleton = etl::singleton<ns16550a::Ns16550a>;
using InterruptDelegate = InterruptBase::InterruptDelegate;

// 外部中断分发器：CPU 外部中断 -> PLIC -> 设备 handler
auto ExternalInterruptHandler(uint64_t /*cause*/, cpu_io::TrapContext* context)
    -> uint64_t {
  auto& plic = InterruptSingleton::instance().plic();
  auto source_id = plic.Which();
  plic.Do(source_id, context);
  plic.Done(source_id);
  return 0;
}

// ebreak 中断处理
auto EbreakHandler(uint64_t exception_code, cpu_io::TrapContext* context)
    -> uint64_t {
  // 读取 sepc 处的指令
  auto instruction = *reinterpret_cast<uint8_t*>(context->sepc);

  // 判断是否为压缩指令 (低 2 位不为 11)
  if ((instruction & 0x3) != 0x3) {
    // 2 字节指令
    context->sepc += 2;
  } else {
    // 4 字节指令
    context->sepc += 4;
  }
  klog::Info("Handle {}", cpu_io::ScauseInfo::kExceptionNames[exception_code]);
  return 0;
}

auto PageFaultHandler(uint64_t exception_code, cpu_io::TrapContext* context)
    -> uint64_t {
  auto addr = cpu_io::Stval::Read();
  klog::Err("PageFault: {}({:#x}), addr: {:#x}",
            cpu_io::ScauseInfo::kExceptionNames[exception_code], exception_code,
            addr);
  klog::Err("sepc: {:#x}", context->sepc);
  DumpStack();
  while (true) {
    cpu_io::Pause();
  }
  return 0;
}

// 系统调用处理
auto SyscallHandler(uint64_t /*cause*/, cpu_io::TrapContext* context)
    -> uint64_t {
  Syscall(0, context);
  return 0;
}

// 软中断 (IPI) 处理
auto IpiHandler(uint64_t /*cause*/, cpu_io::TrapContext* /*context*/)
    -> uint64_t {
  // 清软中断 pending 位
  cpu_io::Sip::Ssip::Clear();
  klog::Debug("Core {} received IPI", cpu_io::GetCurrentCoreId());
  return 0;
}

// 串口外部中断处理
auto SerialIrqHandler(uint64_t /*cause*/, cpu_io::TrapContext* /*context*/)
    -> uint64_t {
  while (Ns16550aSingleton::instance().HasData()) {
    uint8_t ch = Ns16550aSingleton::instance().GetChar();
    etl_putchar(ch);
  }
  return 0;
}

// VirtIO-blk 外部中断处理
auto VirtioBlkIrqHandler(uint64_t /*cause*/, cpu_io::TrapContext* /*context*/)
    -> uint64_t {
  VirtioDriverSingleton::instance().HandleInterrupt(
      [](void* /*token*/, ErrorCode status) {
        if (status != ErrorCode::kSuccess) {
          klog::Err("VirtIO blk IO error: {}", static_cast<int>(status));
        }
      });
  return 0;
}

auto RegisterInterrupts() -> void {
  // 注册外部中断分发器：CPU 外部中断 -> PLIC -> 设备 handler
  InterruptSingleton::instance().RegisterInterruptFunc(
      cpu_io::ScauseInfo::kSupervisorExternalInterrupt,
      InterruptDelegate::create<ExternalInterruptHandler>());

  auto [base, size, irq] = KernelFdtSingleton::instance().GetSerial().value();
  auto uart_result = ns16550a::Ns16550a::Create(base);
  if (uart_result) {
    Ns16550aSingleton::create(std::move(*uart_result));
  } else {
    klog::Err("Failed to create Ns16550a: {}",
              static_cast<int>(uart_result.error().code));
  }

  // 注册 ebreak 中断
  InterruptSingleton::instance().RegisterInterruptFunc(
      cpu_io::ScauseInfo::kBreakpoint,
      InterruptDelegate::create<EbreakHandler>());

  // 注册缺页中断处理
  InterruptSingleton::instance().RegisterInterruptFunc(
      cpu_io::ScauseInfo::kInstructionPageFault,
      InterruptDelegate::create<PageFaultHandler>());
  InterruptSingleton::instance().RegisterInterruptFunc(
      cpu_io::ScauseInfo::kLoadPageFault,
      InterruptDelegate::create<PageFaultHandler>());
  InterruptSingleton::instance().RegisterInterruptFunc(
      cpu_io::ScauseInfo::kStoreAmoPageFault,
      InterruptDelegate::create<PageFaultHandler>());

  // 注册系统调用
  InterruptSingleton::instance().RegisterInterruptFunc(
      cpu_io::ScauseInfo::kEcallUserMode,
      InterruptDelegate::create<SyscallHandler>());

  // 注册软中断 (IPI)
  InterruptSingleton::instance().RegisterInterruptFunc(
      cpu_io::ScauseInfo::kSupervisorSoftwareInterrupt,
      InterruptDelegate::create<IpiHandler>());
}

}  // namespace

extern "C" auto HandleTrap(cpu_io::TrapContext* context)
    -> cpu_io::TrapContext* {
  InterruptSingleton::instance().Do(context->scause, context);
  return context;
}

auto InterruptInit(int, const char**) -> void {
  InterruptSingleton::create();

  // 注册中断处理函数
  RegisterInterrupts();

  // 初始化 plic
  auto [plic_addr, plic_size, ndev, context_count] =
      KernelFdtSingleton::instance().GetPlic().value();
  VirtualMemorySingleton::instance()
      .MapMMIO(plic_addr, plic_size)
      .or_else([](Error err) -> Expected<void*> {
        klog::Err("Failed to map PLIC MMIO: {}", err.message());
        while (true) {
          cpu_io::Pause();
        }
        return std::unexpected(err);
      });
  InterruptSingleton::instance().InitPlic(plic_addr, ndev, context_count);

  // 设置 trap vector
  auto success =
      cpu_io::Stvec::SetDirect(reinterpret_cast<uint64_t>(trap_entry));
  if (!success) {
    klog::Err("Failed to set trap vector");
  }

  // 开启 Supervisor 中断
  cpu_io::Sstatus::Sie::Set();

  // 开启内部中断
  cpu_io::Sie::Ssie::Set();

  // 开启外部中断
  cpu_io::Sie::Seie::Set();

  // 通过统一接口注册串口外部中断（先注册 handler，再启用 PLIC）
  auto serial_irq =
      std::get<2>(KernelFdtSingleton::instance().GetSerial().value());
  InterruptSingleton::instance()
      .RegisterExternalInterrupt(serial_irq, cpu_io::GetCurrentCoreId(), 1,
                                 InterruptDelegate::create<SerialIrqHandler>())
      .or_else([](Error err) -> Expected<void> {
        klog::Err("Failed to register serial IRQ: {}", err.message());
        return std::unexpected(err);
      });

  // 通过统一接口注册 virtio-blk 外部中断
  auto& blk_driver = VirtioDriverSingleton::instance();
  auto blk_irq = blk_driver.GetIrq();
  if (blk_irq != 0) {
    InterruptSingleton::instance()
        .RegisterExternalInterrupt(
            blk_irq, cpu_io::GetCurrentCoreId(), 1,
            InterruptDelegate::create<VirtioBlkIrqHandler>())
        .or_else([blk_irq](Error err) -> Expected<void> {
          klog::Err("Failed to register virtio-blk IRQ {}: {}", blk_irq,
                    err.message());
          return std::unexpected(err);
        });
  }

  klog::Info("Hello InterruptInit");
}

auto InterruptInitSMP(int, const char**) -> void {
  // 设置 trap vector
  auto success =
      cpu_io::Stvec::SetDirect(reinterpret_cast<uint64_t>(trap_entry));
  if (!success) {
    klog::Err("Failed to set trap vector");
  }

  // 开启 Supervisor 中断
  cpu_io::Sstatus::Sie::Set();

  // 开启内部中断
  cpu_io::Sie::Ssie::Set();

  // 开启外部中断
  cpu_io::Sie::Seie::Set();

  klog::Info("Hello InterruptInitSMP");
}
