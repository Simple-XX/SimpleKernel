
/**
 * @file cpu.hpp
 * @brief x86_64 cpu 相关定义
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2024-03-05
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2024-03-05<td>Zone.N (Zone.Niuzh@hotmail.com)<td>创建文件
 * </table>
 */

#ifndef SIMPLEKERNEL_SRC_KERNEL_ARCH_X86_64_INCLUDE_CPU_CPU_HPP_
#define SIMPLEKERNEL_SRC_KERNEL_ARCH_X86_64_INCLUDE_CPU_CPU_HPP_

#include <cstdint>
#include <cstdlib>
#include <type_traits>
#include <typeinfo>

#include "kernel_log.hpp"
#include "regs.hpp"
#include "sk_cstdio"
#include "sk_iostream"

/**
 * x86_64 cpu 相关定义
 * @note 寄存器读写设计见 arch/README.md
 * @see sdm.pdf
 * Intel® 64 and IA-32 Architectures Software Developer’s Manual
 * Volume 3 (3A, 3B, 3C, & 3D): System Programming Guide
 * Order Number: 325384-083US
 * https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html
 */
namespace cpu {
/**
 * @brief  读一个字节
 * @param  port           要读的端口
 * @return uint8_t         读取到的数据
 */
static __always_inline uint8_t InByte(const uint32_t port) {
  uint8_t data;
  __asm__ volatile("inb %1, %0" : "=a"(data) : "dN"(port));
  return data;
}

/**
 * @brief  读一个字
 * @param  port           要读的端口
 * @return uint16_t        读取到的数据
 */
static __always_inline uint16_t InWord(const uint32_t port) {
  uint16_t data;
  __asm__ volatile("inw %1, %0" : "=a"(data) : "dN"(port));
  return data;
}

/**
 * @brief  读一个双字
 * @param  port           要读的端口
 * @return uint32_t        读取到的数据
 */
static __always_inline uint32_t InLong(const uint32_t port) {
  uint32_t data;
  __asm__ volatile("inl %1, %0" : "=a"(data) : "dN"(port));
  return data;
}

/**
 * @brief  写一个字节
 * @param  port           要写的端口
 * @param  data           要写的数据
 */
static __always_inline void OutByte(const uint32_t port, const uint8_t data) {
  __asm__ volatile("outb %1, %0" : : "dN"(port), "a"(data));
}

/**
 * @brief  写一个字
 * @param  port           要写的端口
 * @param  data           要写的数据
 */
static __always_inline void OutWord(const uint32_t port, const uint16_t data) {
  __asm__ volatile("outw %1, %0" : : "dN"(port), "a"(data));
}

/**
 * @brief  写一个双字
 * @param  port           要写的端口
 * @param  data           要写的数据
 */
static __always_inline void OutLong(const uint32_t port, const uint32_t data) {
  __asm__ volatile("outl %1, %0" : : "dN"(port), "a"(data));
}

/// @name 端口
static constexpr const uint32_t kCom1 = 0x3F8;
/**
 * 串口定义
 */
class Serial {
 public:
  explicit Serial(uint32_t port) : port_(port) {
    // Disable all interrupts
    OutByte(port_ + 1, 0x00);
    // Enable DLAB (set baud rate divisor)
    OutByte(port_ + 3, 0x80);
    // Set divisor to 3 (lo byte) 38400 baud
    OutByte(port_ + 0, 0x03);
    // (hi byte)
    OutByte(port_ + 1, 0x00);
    // 8 bits, no parity, one stop bit
    OutByte(port_ + 3, 0x03);
    // Enable FIFO, clear them, with 14-byte threshold
    OutByte(port_ + 2, 0xC7);
    // IRQs enabled, RTS/DSR set
    OutByte(port_ + 4, 0x0B);
    // Set in loopback mode, test the serial chip
    OutByte(port_ + 4, 0x1E);
    // Test serial chip (send byte 0xAE and check if serial returns same byte)
    OutByte(port_ + 0, 0xAE);
    // Check if serial is faulty (i.e: not same byte as sent)
    if (InByte(port_ + 0) != 0xAE) {
      asm("hlt");
    }

    // If serial is not faulty set it in normal operation mode (not-loopback
    // with IRQs enabled and OUT#1 and OUT#2 bits enabled)
    OutByte(port_ + 4, 0x0F);
  }

  ~Serial() = default;

  /// @name 不使用的构造函数
  /// @{
  Serial() = delete;
  Serial(const Serial &) = delete;
  Serial(Serial &&) = delete;
  auto operator=(const Serial &) -> Serial & = delete;
  auto operator=(Serial &&) -> Serial & = delete;
  /// @}

  /**
   * @brief  读一个字节
   * @return uint8_t         读取到的数据
   */
  [[nodiscard]] auto Read() const -> uint8_t {
    while (!SerialReceived()) {
      ;
    }
    return InByte(port_);
  }

  /**
   * @brief  写一个字节
   * @param  c              要写的数据
   */
  void Write(uint8_t c) const {
    while (!IsTransmitEmpty()) {
      ;
    }
    OutByte(port_, c);
  }

 private:
  uint32_t port_;

  /**
   * @brief 串口是否接收到数据
   * @return true
   * @return false
   */
  [[nodiscard]] auto SerialReceived() const -> bool {
    return InByte(port_ + 5) & 1;
  }

  /**
   * @brief 串口是否可以发送数据
   * @return true
   * @return false
   */
  [[nodiscard]] auto IsTransmitEmpty() const -> bool {
    return InByte(port_ + 5) & 0x20;
  }
};

/**
 * 中断控制器(8259A)
 * @see https://wiki.osdev.org/8259_PIC
 * @note master 处理 8 个中断，slave 处理八个中断
 * @note 工作在 8086 模式下，中断处理完后需要通知 pic 重置 ISR 寄存器
 */
class Pic {
 public:
  /**
   * 构造函数
   * @param offset1 主片中断偏移，共 8 个
   * @param offset2 从片中断偏移，共 8 个
   */
  explicit Pic(uint8_t offset1, uint8_t offset2)
      : offset1_(offset1), offset2_(offset2) {
    // 0001 0001
    OutByte(kMasterCommand, kIcw1Init | kIcw1Icw4);
    // 设置主片 IRQ 从 offset1_ 号中断开始
    OutByte(kMasterData, offset1_);
    // 设置主片 IR2 引脚连接从片
    // 4: 0000 0100
    OutByte(kMasterData, 4);
    // 设置主片按照 8086 的方式工作
    OutByte(kMasterData, kIcw48086);

    OutByte(kSlaveCommand, kIcw1Init | kIcw1Icw4);
    // 设置从片 IRQ 从 offset2_ 号中断开始
    OutByte(kPic2Data, offset2_);
    // 告诉从片输出引脚和主片 IR2 号相连
    // 2: 0000 0010
    OutByte(kPic2Data, 2);
    // 设置从片按照 8086 的方式工作
    OutByte(kPic2Data, kIcw48086);

    // 关闭所有中断
    OutByte(kMasterData, 0xFF);
    OutByte(kPic2Data, 0xFF);
  }

  /// @name 构造/析构函数
  /// @{
  Pic() = delete;
  Pic(const Pic &) = delete;
  Pic(Pic &&) = delete;
  auto operator=(const Pic &) -> Pic & = delete;
  auto operator=(Pic &&) -> Pic & = delete;
  ~Pic() = default;
  /// @}

  /**
   * 开启 pic 的 no 中断
   * @param no 中断号
   */
  void Enable(uint8_t no) {
    uint8_t mask = 0;
    if (no >= offset2_) {
      mask = ((InByte(kPic2Data)) & (~(1 << (no % 8))));
      OutByte(kPic2Data, mask);
    } else {
      mask = ((InByte(kMasterData)) & (~(1 << (no % 8))));
      OutByte(kMasterData, mask);
    }
  }

  /**
   * 关闭 8259A 芯片的所有中断
   */
  void Disable() {
    // 屏蔽所有中断
    OutByte(kMasterData, 0xFF);
    OutByte(kPic2Data, 0xFF);
  }

  /**
   * 关闭 pic 的 no 中断
   * @param no 中断号
   */
  void Disable(uint8_t no) {
    uint8_t mask = 0;
    if (no >= offset2_) {
      mask = ((InByte(kPic2Data)) | (1 << (no % 8)));
      OutByte(kPic2Data, mask);
    } else {
      mask = ((InByte(kMasterData)) | (1 << (no % 8)));
      OutByte(kMasterData, mask);
    }
  }

  /**
   * 通知 pic no 中断处理完毕
   * @param no 中断号
   */
  void Clear(uint8_t no) {
    // 按照我们的设置，从 offset1_ 号中断起为用户自定义中断
    // 因为单片的 Intel 8259A 芯片只能处理 8 级中断
    // 故大于等于 offset2_ 的中断号是由从片处理的
    if (no >= offset2_) {
      // 发送重设信号给从片
      OutByte(kSlaveCommand, kEoi);
    } else {
      // 发送重设信号给主片
      OutByte(kMasterCommand, kEoi);
    }
  }

  /**
   * Returns the combined value of the cascaded PICs irq request register
   * @return uint16_t 值
   */
  uint16_t GetIrr() { return GetIrqReg(kOcw3ReadIrr); }

  /**
   * Returns the combined value of the cascaded PICs in-service register
   * @return uint16_t 值
   */
  uint16_t GetIsr() { return GetIrqReg(kOcw3ReadIsr); }

 private:
  uint8_t offset1_;
  uint8_t offset2_;

  /// Master (IRQs 0-7)
  static constexpr const uint8_t kMaster = 0x20;
  /// Slave  (IRQs 8-15)
  static constexpr const uint8_t kSlave = 0xA0;
  static constexpr const uint8_t kMasterCommand = kMaster;
  static constexpr const uint8_t kMasterData = kMaster + 1;
  static constexpr const uint8_t kSlaveCommand = kSlave;
  static constexpr const uint8_t kPic2Data = kSlave + 1;
  /// End-of-interrupt command code
  static constexpr const uint8_t kEoi = 0x20;

  /// Indicates that ICW4 will be present
  static constexpr const uint8_t kIcw1Icw4 = 0x01;
  /// Single (cascade) mode
  static constexpr const uint8_t kIcw1Single = 0x02;
  /// Call address interval 4 (8)
  static constexpr const uint8_t kIcw1Interval4 = 0x04;
  /// Level triggered (edge) mode
  static constexpr const uint8_t kIcw1Level = 0x08;
  /// Initialization - required!
  static constexpr const uint8_t kIcw1Init = 0x10;

  /// OCW3 irq ready next CMD read
  static constexpr const uint8_t kOcw3ReadIrr = 0x0A;
  /// OCW3 irq service next CMD read
  static constexpr const uint8_t kOcw3ReadIsr = 0x0B;

  /// 8086/88 (MCS-80/85) mode
  static constexpr const uint8_t kIcw48086 = 0x01;
  /// Auto (normal) EOI
  static constexpr const uint8_t kIcw4Auto = 0x02;
  /// Buffered mode/slave
  static constexpr const uint8_t kIcw4BufferSlave = 0x08;
  /// Buffered mode/master
  static constexpr const uint8_t kIcw4BufferMaster = 0x0C;
  /// Special fully nested (not)
  static constexpr const uint8_t kIcw4Sfnm = 0x10;

  /**
   * 获取中断请求寄存器的值
   * @note OCW3 to PIC CMD to get the register values.  PIC2 is chained, and
   * represents IRQs 8-15.  PIC1 is IRQs 0-7, with 2 being the chain
   * @param ocw3 OCW3
   * @return uint16_t 值
   */
  uint16_t GetIrqReg(uint8_t ocw3) {
    OutByte(kMasterCommand, ocw3);
    OutByte(kSlaveCommand, ocw3);
    return (InByte(kSlaveCommand) << 8) | InByte(kMasterCommand);
  }
};

/**
 * 时钟控制器(8253/8254)
 * @see https://en.wikipedia.org/wiki/Intel_8253
 * @see https://wiki.osdev.org/Programmable_Interval_Timer
 */
class Pit {
 public:
  /**
   * 构造函数
   * @param frequency 每秒中断次数
   */
  explicit Pit(uint16_t frequency) {
    uint16_t divisor = kMaxFrequency / frequency;

    // 设置 8253/8254 芯片工作在模式 3 下
    OutByte(kCommand, (uint8_t)kChannel0 | (uint8_t)kHighAndLow |
                          (uint8_t)kSquareWaveGenerator);

    // 分别写入低字节和高字节
    OutByte(kChannel0Data, divisor & 0xFF);
    OutByte(kChannel0Data, divisor >> 8);
  }

  /// @name 构造/析构函数
  /// @{
  Pit() = delete;
  Pit(const Pit &) = delete;
  Pit(Pit &&) = delete;
  auto operator=(const Pit &) -> Pit & = delete;
  auto operator=(Pit &&) -> Pit & = delete;
  ~Pit() = default;
  /// @}

  /**
   * 计数器更新
   */
  void Ticks() { ticks_ += 1; }

  /**
   * 获取时钟中断次数
   * @return size_t 时钟中断次数
   */
  size_t GetTicks() const { return ticks_; }

 private:
  /// 最大频率
  static constexpr const size_t kMaxFrequency = 1193180;
  /// 通道 0 数据端口
  static constexpr const size_t kChannel0Data = 0x40;
  /// 模式/命令端口
  static constexpr const size_t kCommand = 0x43;

  /**
   * Bits         Usage
   * 6 and 7      Select channel :
   *                 0 0 = Channel 0
   *                 0 1 = Channel 1
   *                 1 0 = Channel 2
   *                 1 1 = Read-back command (8254 only)
   */
  enum Channel {
    kChannel0 = 0x0,
    kChannel1 = 0x40,
    kChannel2 = 0x80,
  };

  /**
   * Bits         Usage
   * 4 and 5      Access mode :
   *                 0 0 = Latch count value command
   *                 0 1 = Access mode: lobyte only
   *                 1 0 = Access mode: hibyte only
   *                 1 1 = Access mode: lobyte/hibyte
   */
  enum Access {
    kLatchCount = 0x0,
    kLowOnly = 0x10,
    kHighOnly = 0x20,
    kHighAndLow = 0x30,
  };

  /**
   * Bits         Usage
   * 1 to 3       Operating mode :
   *                 0 0 0 = Mode 0 (interrupt on terminal count)
   *                 0 0 1 = Mode 1 (hardware re-triggerable one-shot)
   *                 0 1 0 = Mode 2 (rate generator)
   *                 0 1 1 = Mode 3 (square wave generator)
   *                 1 0 0 = Mode 4 (software triggered strobe)
   *                 1 0 1 = Mode 5 (hardware triggered strobe)
   *                 1 1 0 = Mode 2 (rate generator, same as 010b)
   *                 1 1 1 = Mode 3 (square wave generator, same as 011b)
   * 0            BCD/Binary mode: 0 = 16-bit binary, 1 = four-digit BCD
   */
  enum Mode {
    kInterruptOnTerminalCount = 0x0,
    kHardwareRetriggerableOneShot = 0x2,
    kRateGenerator = 0x4,
    kSquareWaveGenerator = 0x6,
    kSoftwareTriggeredStrobe = 0x8,
    kHardwareTriggeredStrobe = 0xA,
  };

  /// 计数器
  volatile size_t ticks_ = 0;
};

/// 中断上下文，由 cpu 自动压入，无错误码
struct InterruptContext {
  uint64_t rip;
  uint64_t cs;
  uint64_t rflags;
  uint64_t rsp;
  uint64_t ss;

  friend sk_std::ostream &operator<<(
      sk_std::ostream &os, const InterruptContext &interrupt_context) {
    printf("rip: 0x%lX\n", interrupt_context.rip);
    printf("cs: 0x%lX\n", interrupt_context.cs);
    printf("rflags: 0x%lX\n", interrupt_context.rflags);
    printf("rsp: 0x%lX\n", interrupt_context.rsp);
    printf("ss: 0x%lX", interrupt_context.ss);
    return os;
  }
};

/// 中断上下文，由 cpu 自动压入，有错误码
struct InterruptContextErrorCode {
  register_info::IdtrInfo::ErrorCode error_code;
  uint32_t padding;
  uint64_t rip;
  uint64_t cs;
  uint64_t rflags;
  uint64_t rsp;
  uint64_t ss;

  friend sk_std::ostream &operator<<(
      sk_std::ostream &os,
      const InterruptContextErrorCode &interrupt_context_error_code) {
    sk_std::cout << sk_std::endl
                 << interrupt_context_error_code.error_code << sk_std::endl;
    printf("padding: 0x%X\n", interrupt_context_error_code.padding);
    printf("rip: 0x%lX\n", interrupt_context_error_code.rip);
    printf("cs: 0x%lX\n", interrupt_context_error_code.cs);
    printf("rflags: 0x%lX\n", interrupt_context_error_code.rflags);
    printf("rsp: 0x%lX\n", interrupt_context_error_code.rsp);
    printf("ss: 0x%lX", interrupt_context_error_code.ss);
    return os;
  }
};

namespace vmm {
/// P = 1 表示有效； P = 0 表示无效。
static constexpr const uint8_t VMM_PAGE_VALID = 1 << 0;
/// 如果为 0  表示页面只读或可执行。
static constexpr const uint8_t VMM_PAGE_READABLE = 0;
static constexpr const uint8_t VMM_PAGE_WRITABLE = 1 << 1;
static constexpr const uint8_t VMM_PAGE_EXECUTABLE = 0;
/// U/S-- 位 2 是用户 / 超级用户 (User/Supervisor) 标志。
/// 如果为 1 那么运行在任何特权级上的程序都可以访问该页面。
static constexpr const uint8_t VMM_PAGE_USER = 1 << 2;
/// 内核虚拟地址相对物理地址的偏移
static constexpr const size_t KERNEL_OFFSET = 0x0;
/// PTE 属性位数
static constexpr const size_t VMM_PTE_PROP_BITS = 12;
/// PTE 页内偏移位数
static constexpr const size_t VMM_PAGE_OFF_BITS = 12;
/// VPN 位数
static constexpr const size_t VMM_VPN_BITS = 9;
/// VPN 位数掩码，9 位 VPN
static constexpr const size_t VMM_VPN_BITS_MASK = 0x1FF;
/// x86_64 使用了四级页表
static constexpr const size_t VMM_PT_LEVEL = 4;

// 开启 PG
inline void EnablePage() { kAllCr.cr0.pg.Set(); }
inline void DisablePage() { kAllCr.cr0.pg.Clear(); }

/**
 * @brief 设置 页目录
 * @param  _pgd            要设置的页表
 * @return true            成功
 * @return false           失败
 */
inline void SetPageDirectory(uint64_t pgd) { kAllCr.cr3.Write(pgd); }

/**
 * @brief 获取页目录 CR3
 * @return uint64_t        CR3 值
 */
inline uint64_t GetPageDirectory() { return kAllCr.cr3.Read(); }

inline void FlushPage(uint64_t addr) {
  __asm__ volatile("invlpg (%0)" : : "r"(addr) : "memory");
}

};  // namespace vmm

};  // namespace cpu

#endif  // SIMPLEKERNEL_SRC_KERNEL_ARCH_X86_64_INCLUDE_CPU_CPU_HPP_
