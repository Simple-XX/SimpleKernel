/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#pragma once

#include <etl/io_port.h>

#include <cstddef>
#include <cstdint>

/**
 * @brief GIC 中断控制器驱动
 * @see
 * https://developer.arm.com/documentation/100095/0003/Generic-Interrupt-Controller-CPU-Interface/GIC-programmers-model/CPU-interface-memory-mapped-register-descriptions
 */
class Gic {
 public:
  static constexpr const char* kCompatibleName = "arm,gic-v3";

  static constexpr size_t kSgiBase = 0;
  static constexpr size_t kSgiCount = 16;
  static constexpr size_t kPpiBase = 16;
  static constexpr size_t kPpiCount = 16;
  static constexpr size_t kSpiBase = 32;
  static constexpr size_t kSpiCount = 988;

  /**
   * @brief GIC Distributor 接口
   */
  class Gicd {
   public:
    /**
     * @brief 允许从 Distributor 转发到 redistributor
     * @param intid 中断号
     */
    auto Enable(uint32_t intid) const -> void;

    /**
     * @brief 允许 no-sec group1 中断
     */
    auto EnableGrp1Ns() const -> void;

    /**
     * @brief 禁止从 Distributor 转发到 redistributor
     * @param intid 中断号
     */
    auto Disable(uint32_t intid) const -> void;

    /**
     * @brief 清除 intid 的中断
     * @param intid 中断号
     */
    auto Clear(uint32_t intid) const -> void;

    /**
     * @brief 判断 intid 中断是否使能
     * @param intid 中断号
     * @return true 允许
     * @return false 未使能
     */
    [[nodiscard]] auto IsEnable(uint32_t intid) const -> bool;

    /**
     * @brief 设置 intid 的优先级
     * @param intid 中断号
     * @param prio 优先级
     */
    auto SetPrio(uint32_t intid, uint32_t prio) const -> void;

    /**
     * @brief 设置 intid 的属性
     * @param intid 中断号
     * @param config 属性
     */
    auto SetConfig(uint32_t intid, uint32_t config) const -> void;

    /**
     * @brief 设置 intid 的由指定 cpu 处理
     * @param intid 中断号
     * @param cpuid cpu 编号
     */
    auto SetTarget(uint32_t intid, uint32_t cpuid) const -> void;

    /**
     * @brief 设置指定 SPI 中断
     * SPI: shared peripheral interrupt,
     * 共享外设中断，该中断来源于外设，但是该中断可以对所有的 core 有效
     * @param intid 中断号
     * @param cpuid cpu 编号
     */
    auto SetupSpi(uint32_t intid, uint32_t cpuid) const -> void;

    /// @name 构造/析构函数
    /// @{

    /**
     * @brief 构造函数
     * @param _base_addr GICD 基地址
     */
    explicit Gicd(uint64_t _base_addr);
    Gicd() = default;
    Gicd(const Gicd&) = delete;
    Gicd(Gicd&&) = delete;
    auto operator=(const Gicd&) -> Gicd& = delete;
    auto operator=(Gicd&&) -> Gicd& = default;
    ~Gicd() = default;
    /// @}

   private:
    /// @see
    /// https://developer.arm.com/documentation/101206/0003/Programmers-model/Distributor-registers--GICD-GICDA--summary
    /// GICD Register offsets
    /// Configuration dependent Distributor Control Register, RW
    static constexpr uint32_t kCtlr = 0x0000;
    static constexpr uint32_t kCtlrEnableGrp1Ns = 0x2;

    /**
     * @brief GICD_CTLR, Distributor Control Register
     * @see
     * https://developer.arm.com/documentation/101206/0003/Programmers-model/Distributor-registers--GICD-GICDA--summary/GICD-CTLR--Distributor-Control-Register?lang=en
     */
    struct GicdCtlr {
      /// [31] Register Write Pending
      uint32_t rwp : 1;
      uint32_t reserved1 : 23;
      /// [7] E1NWF Enable 1 of N Wakeup Functionality
      uint32_t e1nwf : 1;
      /// [6] DS Disable Security
      uint32_t ds : 1;
      /// [5] ARE_NS Affinity Routing Enable, Non-secure state
      uint32_t are_ns : 1;
      /// [4] ARE_S Affinity Routing Enable, Secure state
      uint32_t are_s : 1;
      uint32_t reserved0 : 1;
      /// [2] EnableGrp1S Enable Secure Group 1 interrupts
      uint32_t enable_grp1_s : 1;
      /// [1] EnableGrp1NS Enable Non-secure Group 1 interrupts
      uint32_t enable_grp1_ns : 1;
      /// [0] EnableGrp0 Enable Group 0 interrupts
      uint32_t enable_grp0 : 1;
    };

    /// Configuration dependent Interrupt Controller Type Register, RO
    static constexpr uint32_t kTyper = 0x0004;
    static constexpr uint32_t kTyperItLinesNumberMask = 0x1F;

    /**
     * @brief GICD_TYPER, Interrupt Controller Type Register
     * @see
     * https://developer.arm.com/documentation/101206/0003/Programmers-model/Distributor-registers--GICD-GICDA--summary/GICD-TYPER--Interrupt-Controller-Type-Register?lang=en
     */
    struct GicdTyper {
      /// [31:26] Reserved, returns 0b000000
      uint32_t reserved1 : 6;
      /// [25] No1N 1 of N SPI
      uint32_t no1n : 1;
      /// [24] A3V Affinity level 3 values
      uint32_t a3v : 1;
      /// [23:19] IDbits Interrupt identifier bits
      uint32_t idbits : 5;
      /// [18] DVIS Direct virtual LPI injection support
      uint32_t dvis : 1;
      /// [17] LPIS Indicates whether the implementation supports LPIs
      uint32_t lpis : 1;
      /// [16] MBIS Message-based interrupt support
      uint32_t mbis : 1;
      /// [15:11] num_LPIs Returns 0b00000 because
      /// GICD_TYPER.IDbits indicates the number of LPIs that the GIC supports
      uint32_t num_lpis : 5;
      /// [10] SecurityExtn Security state support
      uint32_t security_extn : 1;
      /// [9:8] Reserved, returns 0b00000
      uint32_t reserved0 : 2;
      /// [7:5] CPUNumber Returns 0b000 because GICD_CTLR.ARE==1
      /// (ARE_NS & ARE_S)
      uint32_t cpu_number : 3;
      /// [4:0] ITLinesNumber Returns the maximum SPI INTID that this
      /// GIC-600AE implementation supports, given by
      /// 32×(ITLinesNumber + 1) − 1
      uint32_t it_lines_number : 5;
    };

    /// Configuration dependent Distributor Implementer Identification Register,
    /// RO
    static constexpr uint32_t kIidr = 0x0008;

    /**
     * @brief GICD_IIDR, Distributor Implementer Identification Register
     * @see
     * https://developer.arm.com/documentation/ddi0601/2024-12/External-Registers/GICD-IIDR--Distributor-Implementer-Identification-Register?lang=en
     */
    struct GicdIidr {
      /// [31:24] Product Identifier
      uint32_t product_id : 8;
      /// [23:20] Reserved, RES0
      uint32_t reserved0 : 4;
      /// [19:16] Variant number. Typically used to distinguish
      /// product variants or major revisions of a product
      uint32_t variant : 4;
      /// [15:12] Revision number. Typically used to distinguish
      /// minor revisions of a product
      uint32_t revision : 4;
      /// [11:0] Contains the JEP106 manufacturer's identification code
      /// of the designer of the Distributor
      uint32_t implementer : 12;
    };

    /// Function Control Register, RW
    static constexpr uint32_t kFctlr = 0x0020;
    /// Tie-off dependentb Secure Access Control register, RW
    static constexpr uint32_t kSac = 0x0024;
    /// Non-secure SPI Set Register, WO
    static constexpr uint32_t kSetSpiNsr = 0x0040;
    /// Non-secure SPI Clear Register, WO
    static constexpr uint32_t kClrSpiNsr = 0x0048;
    /// Secure SPI Set Register, WO
    static constexpr uint32_t kSetSpiSr = 0x0050;
    /// Secure SPI Clear Register, WO
    static constexpr uint32_t kClrSpiSr = 0x0058;

    /// Interrupt Group Registers, n = 0-31, but n=0 is Reserved
    /// @see
    /// https://developer.arm.com/documentation/ddi0601/2024-12/External-Registers/GICD-IGROUPR-n---Interrupt-Group-Registers?lang=en
    static constexpr uint32_t kIgrouprn = 0x0080;

    /**
     * @brief 计算 IGROUPR 寄存器偏移
     * @param n 寄存器索引
     * @return uint64_t 寄存器偏移地址
     */
    [[nodiscard]] __always_inline auto Igrouprn(uint64_t n) const -> uint64_t {
      return kIgrouprn + n * 4;
    }

    /// Interrupt Set-Enable Registers, n = 0-31, but n=0 is Reserved
    /// @see
    /// https://developer.arm.com/documentation/ddi0601/2024-12/External-Registers/GICD-ISENABLER-n---Interrupt-Set-Enable-Registers?lang=en
    static constexpr uint32_t kIsEnablern = 0x0100;
    static constexpr uint32_t kIsEnablernSize = 32;

    /**
     * @brief 计算 ISENABLER 寄存器偏移
     * @param n 寄存器索引
     * @return uint64_t 寄存器偏移地址
     */
    [[nodiscard]] __always_inline auto Isenablern(uint64_t n) const
        -> uint64_t {
      return kIsEnablern + n * 4;
    }

    /// Interrupt Clear-Enable Registers, n = 0-31, but n=0 is Reserved
    /// @see
    /// https://developer.arm.com/documentation/ddi0601/2024-12/External-Registers/GICD-ICENABLER-n---Interrupt-Clear-Enable-Registers?lang=en
    static constexpr uint32_t kIcEnablern = 0x0180;
    static constexpr uint32_t kIcEnablernSize = 32;

    /**
     * @brief 计算 ICENABLER 寄存器偏移
     * @param n 寄存器索引
     * @return uint64_t 寄存器偏移地址
     */
    [[nodiscard]] __always_inline auto Icenablern(uint64_t n) const
        -> uint64_t {
      return kIcEnablern + n * 4;
    }

    /// Interrupt Set-Pending Registers, n = 0-31, but n=0 is Reserved
    static constexpr uint32_t kIsPendrn = 0x0200;

    /// Interrupt Clear-Pending Registers, n = 0-31, but n=0 is Reserved
    /// @see
    /// https://developer.arm.com/documentation/ddi0601/2024-12/External-Registers/GICD-ICPENDR-n---Interrupt-Clear-Pending-Registers?lang=en
    static constexpr uint32_t kIcPendrn = 0x0280;
    static constexpr uint32_t kIcPendrnSize = 32;

    /**
     * @brief 计算 ICPENDR 寄存器偏移
     * @param n 寄存器索引
     * @return uint64_t 寄存器偏移地址
     */
    [[nodiscard]] __always_inline auto Icpendrn(uint64_t n) const -> uint64_t {
      return kIcPendrn + n * 4;
    }

    /// Interrupt Set-Active Registers, n = 0-31, but n=0 is Reserved
    static constexpr uint32_t kIsActivern = 0x0300;
    /// Interrupt Clear-Active Registers, n = 0-31, but n=0 is Reserved
    static constexpr uint32_t kIcActivern = 0x0380;

    /// Interrupt Priority Registers, n = 0-255, but n=0-7 are Reserved when
    /// affinity routing is enabled
    /// @see
    /// https://developer.arm.com/documentation/ddi0601/2024-12/External-Registers/GICD-IPRIORITYR-n---Interrupt-Priority-Registers?lang=en
    static constexpr uint32_t kIpriorityrn = 0x0400;
    static constexpr uint32_t kIpriorityrnSize = 4;
    static constexpr uint32_t kIpriorityrnBits = 8;
    static constexpr uint32_t kIpriorityrnBitsMask = 0xFF;

    /**
     * @brief 计算 IPRIORITYR 寄存器偏移
     * @param n 寄存器索引
     * @return uint64_t 寄存器偏移地址
     */
    [[nodiscard]] __always_inline auto Ipriorityrn(uint64_t n) const
        -> uint64_t {
      return kIpriorityrn + n * 4;
    }

    /// Interrupt Processor Targets Registers, n = 0 - 254
    /// @see
    /// https://developer.arm.com/documentation/ddi0601/2024-12/External-Registers/GICD-ITARGETSR-n---Interrupt-Processor-Targets-Registers?lang=en
    static constexpr uint32_t kItargetsrn = 0x0800;
    static constexpr uint32_t kItargetsrnSize = 4;
    static constexpr uint32_t kItargetsrnBits = 8;
    static constexpr uint32_t kItargetsrnBitsMask = 0xFF;

    /**
     * @brief 计算 ITARGETSR 寄存器偏移
     * @param n 寄存器索引
     * @return uint64_t 寄存器偏移地址
     */
    [[nodiscard]] __always_inline auto Itargetsrn(uint64_t n) const
        -> uint64_t {
      return kItargetsrn + n * 4;
    }

    /// Interrupt Configuration Registers, n = 0-63, but n=0-1 are Reserved
    /// @see
    /// https://developer.arm.com/documentation/ddi0601/2024-12/External-Registers/GICD-ICFGR-n---Interrupt-Configuration-Registers?lang=en
    static constexpr uint32_t kIcfgrn = 0x0C00;
    static constexpr uint32_t kIcfgrnSize = 16;
    static constexpr uint32_t kIcfgrnBits = 2;
    static constexpr uint32_t kIcfgrnBitsMask = 0x3;
    static constexpr uint32_t kIcfgrnLevelSensitive = 0;
    static constexpr uint32_t kIcfgrnEdgeTriggered = 1;

    /**
     * @brief 计算 ICFGR 寄存器偏移
     * @param n 寄存器索引
     * @return uint64_t 寄存器偏移地址
     */
    [[nodiscard]] __always_inline auto Icfgrn(uint64_t n) const -> uint64_t {
      return kIcfgrn + n * 4;
    }

    /// Interrupt Group Modifier Registers, n = 0-31, but n=0 is Reserved. If
    /// GICD_CTLR.DS == 1, then this register is RAZ/WI.
    static constexpr uint32_t kIgrpmodrn = 0x0D00;
    /// Non-secure Access Control Registers, n = 0-63, but n=0-1 are Reserved
    /// when affinity routing is enabled
    static constexpr uint32_t kNsacrn = 0x0E00;
    /// Interrupt Routing Registers, n = 0-991, but n=0-31 are Reserved when
    /// affinity routing is enabled.
    static constexpr uint32_t kIroutern = 0x6000;
    /// P-Channel dependent Chip Status Register, RW
    static constexpr uint32_t kChipsr = 0xC000;
    /// Default Chip Register, RW
    static constexpr uint32_t kDchipr = 0xC004;
    /// Chip Registers, n = 0-15. Reserved in single-chip configurations.
    static constexpr uint32_t kChiprn = 0xC008;
    /// Interrupt Class Registers, n = 0-63, but n=0-1 are Reserved
    static constexpr uint32_t kIclarn = 0xE000;
    /// Interrupt Clear Error Registers, n = 0-31, but n=0 is Reserved
    static constexpr uint32_t kIcerrrn = 0xE100;
    /// Configuration dependent Configuration ID Register, RO
    static constexpr uint64_t kCfgid = 0xF000;
    /// Peripheral ID4 register , RO
    static constexpr uint32_t kPidr4 = 0xFFD0;
    /// Peripheral ID 5 Register, RO
    static constexpr uint32_t kPidr5 = 0xFFD4;
    /// Peripheral ID 6 Register, RO
    static constexpr uint32_t kPidr6 = 0xFFD8;
    /// Peripheral ID 7 Register, RO
    static constexpr uint32_t kPidr7 = 0xFFDC;
    /// Peripheral ID0 register, RO
    static constexpr uint32_t kPidr0 = 0xFFE0;
    /// Peripheral ID1 register, RO
    static constexpr uint32_t kPidr1 = 0xFFE4;
    /// Peripheral ID2 register, RO
    static constexpr uint32_t kPidr2 = 0xFFE8;
    /// Peripheral ID3 register, RO
    static constexpr uint32_t kPidr3 = 0xFFEC;
    /// Component ID 0 Register, RO
    static constexpr uint32_t kCidr0 = 0xFFF0;
    /// Component ID 1 Register, RO
    static constexpr uint32_t kCidr1 = 0xFFF4;
    /// Component ID 2 Register, RO
    static constexpr uint32_t kCidr2 = 0xFFF8;
    /// Component ID 3 Register, RO
    static constexpr uint32_t kCidr3 = 0xFFFC;

    /// GICD 基地址
    uint64_t base_addr_{0};

    [[nodiscard]] __always_inline auto Read(uint32_t off) const -> uint32_t {
      etl::io_port_ro<uint32_t> reg{reinterpret_cast<void*>(base_addr_ + off)};
      return reg.read();
    }

    __always_inline auto Write(uint32_t off, uint32_t val) const -> void {
      etl::io_port_wo<uint32_t> reg{reinterpret_cast<void*>(base_addr_ + off)};
      reg.write(val);
    }
  };

  /**
   * @brief GIC Redistributor 接口
   */
  class Gicr {
   public:
    /**
     * @brief 允许从 redistributor 转发到 CPU interface
     * @param intid 中断号
     * @param cpuid cpu 编号
     */
    auto Enable(uint32_t intid, uint32_t cpuid) const -> void;

    /**
     * @brief 禁止从 redistributor 转发到 CPU interface
     * @param intid 中断号
     * @param cpuid cpu 编号
     */
    auto Disable(uint32_t intid, uint32_t cpuid) const -> void;

    /**
     * @brief 清除指定 cpu intid 的中断
     * @param intid 中断号
     * @param cpuid cpu 编号
     */
    auto Clear(uint32_t intid, uint32_t cpuid) const -> void;

    /**
     * @brief 设置 intid 的优先级
     * @param intid 中断号
     * @param cpuid cpu 编号
     * @param prio 优先级
     */
    auto SetPrio(uint32_t intid, uint32_t cpuid, uint32_t prio) const -> void;

    /**
     * @brief 初始化 gicr，在多核场景使用
     */
    auto SetUp() const -> void;

    /**
     * @brief 设置指定 PPI 中断
     * PPI: private peripheral interrupt,
     * 私有外设中断，该中断来源于外设，但是该中断只对指定的 core 有效
     * @param intid 中断号
     * @param cpuid cpu 编号
     */
    auto SetupPpi(uint32_t intid, uint32_t cpuid) const -> void;

    /**
     * @brief 设置指定 SGI 中断
     * SGI: Software Generated Interrupt,
     * 软件生成中断，用于处理器间通信
     * @param intid 中断号 (0-15)
     * @param cpuid cpu 编号
     */
    auto SetupSgi(uint32_t intid, uint32_t cpuid) const -> void;

    /// @name 构造/析构函数
    /// @{

    /**
     * @brief 构造函数
     * @param _base_addr GICR 基地址
     */
    explicit Gicr(uint64_t _base_addr);
    Gicr() = default;
    Gicr(const Gicr&) = delete;
    Gicr(Gicr&&) = delete;
    auto operator=(const Gicr&) -> Gicr& = delete;
    auto operator=(Gicr&&) -> Gicr& = default;
    ~Gicr() = default;
    /// @}

   private:
    /// 每个 GICR 长度 2 * 64 * 1024
    static constexpr uint32_t kStride = 0x20000;

    /// Redistributor Control Register, RW
    /// @see
    /// https://developer.arm.com/documentation/ddi0601/2024-12/External-Registers/GICR-CTLR--Redistributor-Control-Register?lang=en
    static constexpr uint32_t kCtlr = 0x0000;
    /// Redistributor Implementation Identification Register, RO
    static constexpr uint32_t kIidr = 0x0004;
    /// Redistributor Type Register, RO
    static constexpr uint32_t kTyper = 0x0008;

    /// Power Management Control Register, RW1
    /// @see
    /// https://developer.arm.com/documentation/ddi0601/2024-12/External-Registers/GICR-WAKER--Redistributor-Wake-Register?lang=en
    static constexpr uint32_t kWaker = 0x0014;
    static constexpr uint32_t kWakerProcessorSleepMask = 2;
    static constexpr uint32_t kWakerChildrenAsleepMask = 4;

    /// Function Control Register, RW
    static constexpr uint32_t kFctlr = 0x0020;
    /// Power Register, RW
    static constexpr uint32_t kPwrr = 0x0024;
    /// Class Register, RW
    static constexpr uint32_t kClassr = 0x0028;
    /// Redistributor Properties Base Address Register, RW
    static constexpr uint32_t kPropbaser = 0x0070;
    /// Redistributor LPI Pending Table Base Address Register, RW
    static constexpr uint32_t kPendbaser = 0x0078;
    /// Peripheral ID 4 Register, RO
    static constexpr uint32_t kPidr4 = 0xFFD0;
    /// Peripheral ID 5 Register, RO
    static constexpr uint32_t kPidr5 = 0xFFD4;
    /// Peripheral ID 6 Register, RO
    static constexpr uint32_t kPidr6 = 0xFFD8;
    /// Peripheral ID 7 Register, RO
    static constexpr uint32_t kPidr7 = 0xFFDC;
    /// Peripheral ID 0 Register, RO
    static constexpr uint32_t kPidr0 = 0xFFE0;
    /// Peripheral ID 1 Register, RO
    static constexpr uint32_t kPidr1 = 0xFFE4;
    /// Peripheral ID 2 Register, RO
    static constexpr uint32_t kPidr2 = 0xFFE8;
    /// Peripheral ID 3 Register, RO
    static constexpr uint32_t kPidr3 = 0xFFEC;
    /// Component ID 0 Register, RO
    static constexpr uint32_t kCidr0 = 0xFFF0;
    /// Component ID 1 Register, RO
    static constexpr uint32_t kCidr1 = 0xFFF4;
    /// Component ID 2 Register, RO
    static constexpr uint32_t kCidr2 = 0xFFF8;
    /// Component ID 3 Register, RO
    static constexpr uint32_t kCidr3 = 0xFFFC;

    /// SGI 基地址 64 * 1024
    static constexpr uint32_t kSgiBase = 0x10000;

    /// Interrupt Group Register, RW
    /// @see
    /// https://developer.arm.com/documentation/ddi0601/2024-12/External-Registers/GICR-IGROUPR0--Interrupt-Group-Register-0?lang=en
    static constexpr uint32_t kIgroupr0 = kSgiBase + 0x0080;
    static constexpr uint32_t kIgroupr0Clear = 0;
    static constexpr uint32_t kIgroupr0Set = UINT32_MAX;

    /// Interrupt Set-Enable Register, RW
    /// @see
    /// https://developer.arm.com/documentation/ddi0601/2024-12/External-Registers/GICR-ISENABLER0--Interrupt-Set-Enable-Register-0?lang=en
    static constexpr uint32_t kIsEnabler0 = kSgiBase + 0x0100;
    static constexpr uint32_t kIsEnabler0Size = 32;

    /// Interrupt Clear-Enable Register, RW
    /// @see
    /// https://developer.arm.com/documentation/ddi0601/2024-12/External-Registers/GICR-ICENABLER0--Interrupt-Clear-Enable-Register-0?lang=en
    static constexpr uint32_t kIcEnabler0 = kSgiBase + 0x0180;
    static constexpr uint32_t kIcEnabler0Size = 32;

    /// Interrupt Set-Pending Register, RW
    static constexpr uint32_t kIsPendr0 = kSgiBase + 0x0200;

    /// Peripheral Clear Pending Register, RW
    /// @see
    /// https://developer.arm.com/documentation/ddi0601/2024-12/External-Registers/GICR-ICPENDR0--Interrupt-Clear-Pending-Register-0?lang=en
    static constexpr uint32_t kIcPendr0 = kSgiBase + 0x0280;
    static constexpr uint32_t kIcPendr0Size = 32;

    /// Interrupt Set-Active Register, RW
    static constexpr uint32_t kIsActiver0 = kSgiBase + 0x0300;
    /// Interrupt Clear-Active Register, RW
    static constexpr uint32_t kIcActiver0 = kSgiBase + 0x0380;
    /// Interrupt Priority Registers, RW
    /// @see
    /// https://developer.arm.com/documentation/ddi0601/2024-12/External-Registers/GICR-IPRIORITYR-n---Interrupt-Priority-Registers?lang=en
    static constexpr uint32_t kIpriorityrn = kSgiBase + 0x0400;
    static constexpr uint32_t kIpriorityrnSize = 4;
    static constexpr uint32_t kIpriorityrnBits = 8;
    static constexpr uint32_t kIpriorityrnBitsMask = 0xFF;

    /**
     * @brief 计算 IPRIORITYR 寄存器偏移
     * @param n 寄存器索引
     * @return uint64_t 寄存器偏移地址
     */
    [[nodiscard]] __always_inline auto Ipriorityrn(uint64_t n) const
        -> uint64_t {
      return kIpriorityrn + n * 4;
    }

    /// Interrupt Configuration Registers, RW
    static constexpr uint32_t kIcfgrn = 0x0C00;

    /// Interrupt Group Modifier Register, RW
    /// @see
    /// https://developer.arm.com/documentation/ddi0601/2024-12/External-Registers/GICR-IGRPMODR0--Interrupt-Group-Modifier-Register-0?lang=en
    static constexpr uint32_t kIgrpmodr0 = 0x0D00;
    // kIgrpmodr0 kIgroupr0 Definition
    // 0b0        0b0       Secure Group 0 G0S
    // 0b0        0b1       Non-secure Group 1 G1NS
    // 0b1        0b0       Secure Group 1 G1S
    static constexpr uint32_t kIgrpmodr0Clear = 0;
    static constexpr uint32_t kIgrpmodr0Set = UINT32_MAX;

    /// Non-secure Access Control Register, RW
    static constexpr uint32_t kNsacr = 0x0E00;
    /// Miscellaneous Status Register, RO
    static constexpr uint32_t kMiscstatusr = 0xC000;
    /// Interrupt Error Valid Register, RW
    static constexpr uint32_t kIerrvr = 0xC008;
    /// SGI Default Register, RW
    static constexpr uint32_t kSgidr = 0xC010;
    /// Configuration ID0 Register, RO
    static constexpr uint32_t kCfgid0 = 0xF000;
    /// Configuration ID1 Register, RO
    static constexpr uint32_t kCfgid1 = 0xF004;

    /// GICR 基地址
    uint64_t base_addr_{0};

    [[nodiscard]] __always_inline auto Read(uint32_t cpuid, uint32_t off) const
        -> uint32_t {
      etl::io_port_ro<uint32_t> reg{
          reinterpret_cast<void*>(base_addr_ + cpuid * kStride + off)};
      return reg.read();
    }

    __always_inline auto Write(uint32_t cpuid, uint32_t off, uint32_t val) const
        -> void {
      etl::io_port_wo<uint32_t> reg{
          reinterpret_cast<void*>(base_addr_ + cpuid * kStride + off)};
      reg.write(val);
    }
  };

  /**
   * @brief 初始化当前 CPU 的 GIC 配置
   */
  auto SetUp() const -> void;

  /**
   * @brief 配置共享外设中断 (SPI)
   * @param intid 中断号
   * @param cpuid CPU 编号
   */
  auto Spi(uint32_t intid, uint32_t cpuid) const -> void;

  /**
   * @brief 配置私有外设中断 (PPI)
   * @param intid 中断号
   * @param cpuid CPU 编号
   */
  auto Ppi(uint32_t intid, uint32_t cpuid) const -> void;

  /**
   * @brief 配置软件生成中断 (SGI)
   * @param intid 中断号
   * @param cpuid CPU 编号
   */
  auto Sgi(uint32_t intid, uint32_t cpuid) const -> void;

  /// @name 构造/析构函数
  /// @{

  /**
   * @brief 构造函数
   * @param gicd_base_addr GIC Distributor 地址
   * @param gicr_base_addr GIC Redistributor 地址
   */
  explicit Gic(uint64_t gicd_base_addr, uint64_t gicr_base_addr);
  Gic() = default;
  Gic(const Gic&) = delete;
  Gic(Gic&&) = delete;
  auto operator=(const Gic&) -> Gic& = delete;
  auto operator=(Gic&&) -> Gic& = default;
  ~Gic() = default;
  /// @}

 private:
  /// Distributor 实例
  Gicd gicd_{};
  /// Redistributor 实例
  Gicr gicr_{};
};
