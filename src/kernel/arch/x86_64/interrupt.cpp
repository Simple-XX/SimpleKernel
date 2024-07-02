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

// 声明中断处理程序，定义在 interrupt_s.S 中
extern "C" void IsrNoErrorCode_DE();
extern "C" void IsrNoErrorCode_DB();
extern "C" void IsrNoErrorCode_NMI();
extern "C" void IsrNoErrorCode_BP();
extern "C" void IsrNoErrorCode_OF();
extern "C" void IsrNoErrorCode_BR();
extern "C" void IsrNoErrorCode_UD();
extern "C" void IsrNoErrorCode_NM();
extern "C" void IsrErrorCode_DF();
extern "C" void IsrErrorCode_TS();
extern "C" void IsrErrorCode_NP();
extern "C" void IsrErrorCode_SS();
extern "C" void IsrErrorCode_GP();
extern "C" void IsrErrorCode_PF();
extern "C" void IsrNoErrorCode_MF();
extern "C" void IsrErrorCode_AC();
extern "C" void IsrNoErrorCode_MC();
extern "C" void IsrNoErrorCode_XM();
extern "C" void IsrNoErrorCode_VE();
extern "C" void IsrNoErrorCode_SYCALL();

/// IRQ:中断请求(Interrupt Request)
/// 电脑系统计时器
extern "C" void irq0(void);
/// 键盘
extern "C" void irq1(void);
/// 与 IRQ9 相接，MPU-401 MD 使用
extern "C" void irq2(void);
/// 串口设备
extern "C" void irq3(void);
/// 串口设备
extern "C" void irq4(void);
/// 建议声卡使用
extern "C" void irq5(void);
/// 软驱传输控制使用
extern "C" void irq6(void);
/// 打印机传输控制使用
extern "C" void irq7(void);
/// 即时时钟
extern "C" void irq8(void);
/// 与 IRQ2 相接，可设定给其他硬件
extern "C" void irq9(void);
/// 建议网卡使用
extern "C" void irq10(void);
/// 建议 AGP 显卡使用
extern "C" void irq11(void);
/// 接 PS/2 鼠标，也可设定给其他硬件
extern "C" void irq12(void);
/// 协处理器使用
extern "C" void irq13(void);
/// IDE0 传输控制使用
extern "C" void irq14(void);
/// IDE1 传输控制使用
extern "C" void irq15(void);

struct intr_context_t {
  uint64_t r15;
  uint64_t r14;
  uint64_t r13;
  uint64_t r12;
  uint64_t r11;
  uint64_t r10;
  uint64_t r9;
  uint64_t r8;
  uint64_t rbp;
  uint64_t rdi;
  uint64_t rsi;
  uint64_t rdx;
  uint64_t rcx;
  uint64_t rbx;
  uint64_t rax;
  uint64_t rip;
  uint64_t cs;
  uint64_t rflags;
  uint64_t rsp;
  uint64_t ss;
};

extern "C" void TarpEntry(uint8_t no, intr_context_t *intr_context) {
  kInterrupt.GetInstance().Do(no, nullptr);
}

void Interrupt::SetUpIdts() {
  idts[cpu::reginfo::IdtrInfo::kDivideError] = cpu::reginfo::IdtrInfo::Idt(
      (uint64_t)IsrNoErrorCode_DE, 8, 0x0,
      cpu::reginfo::IdtrInfo::Idt::Type::k64BitInterruptGate,
      cpu::reginfo::IdtrInfo::Idt::DPL::kRing0,
      cpu::reginfo::IdtrInfo::Idt::P::kPresent);
  idts[cpu::reginfo::IdtrInfo::kDebugException] = cpu::reginfo::IdtrInfo::Idt(
      (uint64_t)IsrNoErrorCode_DB, 8, 0x0,
      cpu::reginfo::IdtrInfo::Idt::Type::k64BitInterruptGate,
      cpu::reginfo::IdtrInfo::Idt::DPL::kRing0,
      cpu::reginfo::IdtrInfo::Idt::P::kPresent);
  idts[cpu::reginfo::IdtrInfo::kNmiInterrupt] = cpu::reginfo::IdtrInfo::Idt(
      (uint64_t)IsrNoErrorCode_NMI, 8, 0x0,
      cpu::reginfo::IdtrInfo::Idt::Type::k64BitInterruptGate,
      cpu::reginfo::IdtrInfo::Idt::DPL::kRing0,
      cpu::reginfo::IdtrInfo::Idt::P::kPresent);
  idts[cpu::reginfo::IdtrInfo::kBreakpoint] = cpu::reginfo::IdtrInfo::Idt(
      (uint64_t)IsrNoErrorCode_BP, 8, 0x0,
      cpu::reginfo::IdtrInfo::Idt::Type::k64BitInterruptGate,
      cpu::reginfo::IdtrInfo::Idt::DPL::kRing0,
      cpu::reginfo::IdtrInfo::Idt::P::kPresent);
  idts[cpu::reginfo::IdtrInfo::kOverflow] = cpu::reginfo::IdtrInfo::Idt(
      (uint64_t)IsrNoErrorCode_OF, 8, 0x0,
      cpu::reginfo::IdtrInfo::Idt::Type::k64BitInterruptGate,
      cpu::reginfo::IdtrInfo::Idt::DPL::kRing0,
      cpu::reginfo::IdtrInfo::Idt::P::kPresent);
  idts[cpu::reginfo::IdtrInfo::kBoundRangeExceeded] =
      cpu::reginfo::IdtrInfo::Idt(
          (uint64_t)IsrNoErrorCode_BR, 8, 0x0,
          cpu::reginfo::IdtrInfo::Idt::Type::k64BitInterruptGate,
          cpu::reginfo::IdtrInfo::Idt::DPL::kRing0,
          cpu::reginfo::IdtrInfo::Idt::P::kPresent);
  idts[cpu::reginfo::IdtrInfo::kInvalidOpcode] = cpu::reginfo::IdtrInfo::Idt(
      (uint64_t)IsrNoErrorCode_UD, 8, 0x0,
      cpu::reginfo::IdtrInfo::Idt::Type::k64BitInterruptGate,
      cpu::reginfo::IdtrInfo::Idt::DPL::kRing0,
      cpu::reginfo::IdtrInfo::Idt::P::kPresent);
  idts[cpu::reginfo::IdtrInfo::kDeviceNotAvailable] =
      cpu::reginfo::IdtrInfo::Idt(
          (uint64_t)IsrNoErrorCode_NM, 8, 0x0,
          cpu::reginfo::IdtrInfo::Idt::Type::k64BitInterruptGate,
          cpu::reginfo::IdtrInfo::Idt::DPL::kRing0,
          cpu::reginfo::IdtrInfo::Idt::P::kPresent);
  idts[cpu::reginfo::IdtrInfo::kDoubleFault] = cpu::reginfo::IdtrInfo::Idt(
      (uint64_t)IsrErrorCode_DF, 8, 0x0,
      cpu::reginfo::IdtrInfo::Idt::Type::k64BitInterruptGate,
      cpu::reginfo::IdtrInfo::Idt::DPL::kRing0,
      cpu::reginfo::IdtrInfo::Idt::P::kPresent);
  idts[cpu::reginfo::IdtrInfo::kInvalidTss] = cpu::reginfo::IdtrInfo::Idt(
      (uint64_t)IsrErrorCode_TS, 8, 0x0,
      cpu::reginfo::IdtrInfo::Idt::Type::k64BitInterruptGate,
      cpu::reginfo::IdtrInfo::Idt::DPL::kRing0,
      cpu::reginfo::IdtrInfo::Idt::P::kPresent);
  idts[cpu::reginfo::IdtrInfo::kSegmentNotPresent] =
      cpu::reginfo::IdtrInfo::Idt(
          (uint64_t)IsrErrorCode_NP, 8, 0x0,
          cpu::reginfo::IdtrInfo::Idt::Type::k64BitInterruptGate,
          cpu::reginfo::IdtrInfo::Idt::DPL::kRing0,
          cpu::reginfo::IdtrInfo::Idt::P::kPresent);
  idts[cpu::reginfo::IdtrInfo::kStackSegmentFault] =
      cpu::reginfo::IdtrInfo::Idt(
          (uint64_t)IsrErrorCode_SS, 8, 0x0,
          cpu::reginfo::IdtrInfo::Idt::Type::k64BitInterruptGate,
          cpu::reginfo::IdtrInfo::Idt::DPL::kRing0,
          cpu::reginfo::IdtrInfo::Idt::P::kPresent);
  idts[cpu::reginfo::IdtrInfo::kGeneralProtection] =
      cpu::reginfo::IdtrInfo::Idt(
          (uint64_t)IsrErrorCode_GP, 8, 0x0,
          cpu::reginfo::IdtrInfo::Idt::Type::k64BitInterruptGate,
          cpu::reginfo::IdtrInfo::Idt::DPL::kRing0,
          cpu::reginfo::IdtrInfo::Idt::P::kPresent);
  idts[cpu::reginfo::IdtrInfo::kPageFault] = cpu::reginfo::IdtrInfo::Idt(
      (uint64_t)IsrErrorCode_PF, 8, 0x0,
      cpu::reginfo::IdtrInfo::Idt::Type::k64BitInterruptGate,
      cpu::reginfo::IdtrInfo::Idt::DPL::kRing0,
      cpu::reginfo::IdtrInfo::Idt::P::kPresent);
  idts[cpu::reginfo::IdtrInfo::kX87FpuFloatingPointError] =
      cpu::reginfo::IdtrInfo::Idt(
          (uint64_t)IsrNoErrorCode_MF, 8, 0x0,
          cpu::reginfo::IdtrInfo::Idt::Type::k64BitInterruptGate,
          cpu::reginfo::IdtrInfo::Idt::DPL::kRing0,
          cpu::reginfo::IdtrInfo::Idt::P::kPresent);
  idts[cpu::reginfo::IdtrInfo::kAlignmentCheck] = cpu::reginfo::IdtrInfo::Idt(
      (uint64_t)IsrErrorCode_AC, 8, 0x0,
      cpu::reginfo::IdtrInfo::Idt::Type::k64BitInterruptGate,
      cpu::reginfo::IdtrInfo::Idt::DPL::kRing0,
      cpu::reginfo::IdtrInfo::Idt::P::kPresent);
  idts[cpu::reginfo::IdtrInfo::kMachineCheck] = cpu::reginfo::IdtrInfo::Idt(
      (uint64_t)IsrNoErrorCode_MC, 8, 0x0,
      cpu::reginfo::IdtrInfo::Idt::Type::k64BitInterruptGate,
      cpu::reginfo::IdtrInfo::Idt::DPL::kRing0,
      cpu::reginfo::IdtrInfo::Idt::P::kPresent);
  idts[cpu::reginfo::IdtrInfo::kSIMDFloatingPointException] =
      cpu::reginfo::IdtrInfo::Idt(
          (uint64_t)IsrNoErrorCode_XM, 8, 0x0,
          cpu::reginfo::IdtrInfo::Idt::Type::k64BitInterruptGate,
          cpu::reginfo::IdtrInfo::Idt::DPL::kRing0,
          cpu::reginfo::IdtrInfo::Idt::P::kPresent);
  idts[cpu::reginfo::IdtrInfo::kVirtualizationException] =
      cpu::reginfo::IdtrInfo::Idt(
          (uint64_t)IsrNoErrorCode_VE, 8, 0x0,
          cpu::reginfo::IdtrInfo::Idt::Type::k64BitInterruptGate,
          cpu::reginfo::IdtrInfo::Idt::DPL::kRing0,
          cpu::reginfo::IdtrInfo::Idt::P::kPresent);
  idts[cpu::reginfo::IdtrInfo::ControlProtectionException] =
      cpu::reginfo::IdtrInfo::Idt(
          (uint64_t)IsrNoErrorCode_SYCALL, 8, 0x0,
          cpu::reginfo::IdtrInfo::Idt::Type::k64BitInterruptGate,
          cpu::reginfo::IdtrInfo::Idt::DPL::kRing0,
          cpu::reginfo::IdtrInfo::Idt::P::kPresent);
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

    SetUpIdts();

    static auto idtr = cpu::reginfo::IdtrInfo::Idtr{
        .limit = sizeof(cpu::reginfo::IdtrInfo::Idtr) *
                     cpu::reginfo::IdtrInfo::kInterruptMaxCount -
                 1,
        .base = idts,
    };
    cpu::kAllCr.idtr.Write(idtr);

    std::cout << cpu::kAllCr.idtr << std::endl;
    for (size_t i = 0; i < (cpu::kAllCr.idtr.Read().limit + 1) /
                               sizeof(cpu::reginfo::IdtrInfo::Idtr);
         i++) {
      printf("idtr[%d] 0x%p\n", i, cpu::kAllCr.idtr.Read().base + i);
      std::cout << *(cpu::kAllCr.idtr.Read().base + i) << std::endl;
    }

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
