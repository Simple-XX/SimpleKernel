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
extern "C" void Irq_0();
extern "C" void Irq_1();
extern "C" void Irq_2();
extern "C" void Irq_3();
extern "C" void Irq_4();
extern "C" void Irq_5();
extern "C" void Irq_6();
extern "C" void Irq_7();
extern "C" void Irq_8();
extern "C" void Irq_9();
extern "C" void Irq_10();
extern "C" void Irq_11();
extern "C" void Irq_12();
extern "C" void Irq_13();
extern "C" void Irq_14();
extern "C" void Irq_15();

/**
 * @brief 中断处理函数
 * @param no 中断号
 * @param interrupt_context 中断上下文
 */
extern "C" void TarpEntry(uint8_t no,
                          cpu::InterruptContext *interrupt_context) {
  std::cout << *interrupt_context << std::endl;
  kInterrupt.GetInstance().Do(no, (uint8_t *)interrupt_context);
}

void Interrupt::SetUpIdts() {
  // isr
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
          cpu::reginfo::IdtrInfo::Idt::DPL::kRing3,
          cpu::reginfo::IdtrInfo::Idt::P::kPresent);
  // irq
  idts[cpu::reginfo::IdtrInfo::kIrq0] = cpu::reginfo::IdtrInfo::Idt(
      (uint64_t)Irq_0, 8, 0x0,
      cpu::reginfo::IdtrInfo::Idt::Type::k64BitInterruptGate,
      cpu::reginfo::IdtrInfo::Idt::DPL::kRing0,
      cpu::reginfo::IdtrInfo::Idt::P::kPresent);
  idts[cpu::reginfo::IdtrInfo::kIrq1] = cpu::reginfo::IdtrInfo::Idt(
      (uint64_t)Irq_1, 8, 0x0,
      cpu::reginfo::IdtrInfo::Idt::Type::k64BitInterruptGate,
      cpu::reginfo::IdtrInfo::Idt::DPL::kRing0,
      cpu::reginfo::IdtrInfo::Idt::P::kPresent);
  idts[cpu::reginfo::IdtrInfo::kIrq2] = cpu::reginfo::IdtrInfo::Idt(
      (uint64_t)Irq_2, 8, 0x0,
      cpu::reginfo::IdtrInfo::Idt::Type::k64BitInterruptGate,
      cpu::reginfo::IdtrInfo::Idt::DPL::kRing0,
      cpu::reginfo::IdtrInfo::Idt::P::kPresent);
  idts[cpu::reginfo::IdtrInfo::kIrq3] = cpu::reginfo::IdtrInfo::Idt(
      (uint64_t)Irq_3, 8, 0x0,
      cpu::reginfo::IdtrInfo::Idt::Type::k64BitInterruptGate,
      cpu::reginfo::IdtrInfo::Idt::DPL::kRing0,
      cpu::reginfo::IdtrInfo::Idt::P::kPresent);
  idts[cpu::reginfo::IdtrInfo::kIrq4] = cpu::reginfo::IdtrInfo::Idt(
      (uint64_t)Irq_4, 8, 0x0,
      cpu::reginfo::IdtrInfo::Idt::Type::k64BitInterruptGate,
      cpu::reginfo::IdtrInfo::Idt::DPL::kRing0,
      cpu::reginfo::IdtrInfo::Idt::P::kPresent);
  idts[cpu::reginfo::IdtrInfo::kIrq5] = cpu::reginfo::IdtrInfo::Idt(
      (uint64_t)Irq_5, 8, 0x0,
      cpu::reginfo::IdtrInfo::Idt::Type::k64BitInterruptGate,
      cpu::reginfo::IdtrInfo::Idt::DPL::kRing0,
      cpu::reginfo::IdtrInfo::Idt::P::kPresent);
  idts[cpu::reginfo::IdtrInfo::kIrq6] = cpu::reginfo::IdtrInfo::Idt(
      (uint64_t)Irq_6, 8, 0x0,
      cpu::reginfo::IdtrInfo::Idt::Type::k64BitInterruptGate,
      cpu::reginfo::IdtrInfo::Idt::DPL::kRing0,
      cpu::reginfo::IdtrInfo::Idt::P::kPresent);
  idts[cpu::reginfo::IdtrInfo::kIrq7] = cpu::reginfo::IdtrInfo::Idt(
      (uint64_t)Irq_7, 8, 0x0,
      cpu::reginfo::IdtrInfo::Idt::Type::k64BitInterruptGate,
      cpu::reginfo::IdtrInfo::Idt::DPL::kRing0,
      cpu::reginfo::IdtrInfo::Idt::P::kPresent);
  idts[cpu::reginfo::IdtrInfo::kIrq8] = cpu::reginfo::IdtrInfo::Idt(
      (uint64_t)Irq_8, 8, 0x0,
      cpu::reginfo::IdtrInfo::Idt::Type::k64BitInterruptGate,
      cpu::reginfo::IdtrInfo::Idt::DPL::kRing0,
      cpu::reginfo::IdtrInfo::Idt::P::kPresent);
  idts[cpu::reginfo::IdtrInfo::kIrq9] = cpu::reginfo::IdtrInfo::Idt(
      (uint64_t)Irq_9, 8, 0x0,
      cpu::reginfo::IdtrInfo::Idt::Type::k64BitInterruptGate,
      cpu::reginfo::IdtrInfo::Idt::DPL::kRing0,
      cpu::reginfo::IdtrInfo::Idt::P::kPresent);
  idts[cpu::reginfo::IdtrInfo::kIrq10] = cpu::reginfo::IdtrInfo::Idt(
      (uint64_t)Irq_10, 8, 0x0,
      cpu::reginfo::IdtrInfo::Idt::Type::k64BitInterruptGate,
      cpu::reginfo::IdtrInfo::Idt::DPL::kRing0,
      cpu::reginfo::IdtrInfo::Idt::P::kPresent);
  idts[cpu::reginfo::IdtrInfo::kIrq11] = cpu::reginfo::IdtrInfo::Idt(
      (uint64_t)Irq_11, 8, 0x0,
      cpu::reginfo::IdtrInfo::Idt::Type::k64BitInterruptGate,
      cpu::reginfo::IdtrInfo::Idt::DPL::kRing0,
      cpu::reginfo::IdtrInfo::Idt::P::kPresent);
  idts[cpu::reginfo::IdtrInfo::kIrq12] = cpu::reginfo::IdtrInfo::Idt(
      (uint64_t)Irq_12, 8, 0x0,
      cpu::reginfo::IdtrInfo::Idt::Type::k64BitInterruptGate,
      cpu::reginfo::IdtrInfo::Idt::DPL::kRing0,
      cpu::reginfo::IdtrInfo::Idt::P::kPresent);
  idts[cpu::reginfo::IdtrInfo::kIrq13] = cpu::reginfo::IdtrInfo::Idt(
      (uint64_t)Irq_13, 8, 0x0,
      cpu::reginfo::IdtrInfo::Idt::Type::k64BitInterruptGate,
      cpu::reginfo::IdtrInfo::Idt::DPL::kRing0,
      cpu::reginfo::IdtrInfo::Idt::P::kPresent);
  idts[cpu::reginfo::IdtrInfo::kIrq14] = cpu::reginfo::IdtrInfo::Idt(
      (uint64_t)Irq_14, 8, 0x0,
      cpu::reginfo::IdtrInfo::Idt::Type::k64BitInterruptGate,
      cpu::reginfo::IdtrInfo::Idt::DPL::kRing0,
      cpu::reginfo::IdtrInfo::Idt::P::kPresent);
  idts[cpu::reginfo::IdtrInfo::kIrq15] = cpu::reginfo::IdtrInfo::Idt(
      (uint64_t)Irq_15, 8, 0x0,
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
        while (1);
      };
    }

    // 初始化 idt
    SetUpIdts();
    // 写入 idtr
    static auto idtr = cpu::reginfo::IdtrInfo::Idtr{
        .limit = sizeof(cpu::reginfo::IdtrInfo::Idtr) *
                     cpu::reginfo::IdtrInfo::kInterruptMaxCount -
                 1,
        .base = idts,
    };
    cpu::kAllCr.idtr.Write(idtr);

    // 输出 idt 信息
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
