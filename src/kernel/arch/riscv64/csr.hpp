

#ifndef SIMPLEKERNEL_SRC_KERNEL_ARCH_RISCV64_CSR_HPP_
#define SIMPLEKERNEL_SRC_KERNEL_ARCH_RISCV64_CSR_HPP_

#include <cstdint>
#include <cstdlib>

#include "iostream"
#include "stdio.h"

class CSR {
 private:
  /** Immediate instructions use a 5 bit immediate field */
  static constexpr uint64_t kCsrImmOpMask = 0x01F;

  /** CSR: Read only, and Read-Write base class */
  template <class Reg>
  class ReadOnlyReg {
   public:
    using ReadDataType = typename Reg::DataType;

    /// @name 构造/析构函数
    /// @{
    ReadOnlyReg() = default;
    ReadOnlyReg(const ReadOnlyReg &) = delete;
    ReadOnlyReg(ReadOnlyReg &&) = delete;
    auto operator=(const ReadOnlyReg &) -> ReadOnlyReg & = delete;
    auto operator=(ReadOnlyReg &&) -> ReadOnlyReg & = delete;
    ~ReadOnlyReg() = default;
    /// @}

    /** Read the CSR value */
    static inline ReadDataType Read() { return Reg::Read(); }

    /** Operator alias to Read the CSR value */
    inline ReadDataType operator()() { return Reg::Read(); }
  };

  /** CSR: Write only, and Read-Write base class */
  template <class Reg>
  class WriteOnlyReg {
   public:
    using WriteDataType = typename Reg::DataType;

    /// @name 构造/析构函数
    /// @{
    WriteOnlyReg() = default;
    WriteOnlyReg(const WriteOnlyReg &) = delete;
    WriteOnlyReg(WriteOnlyReg &&) = delete;
    auto operator=(const WriteOnlyReg &) -> WriteOnlyReg & = delete;
    auto operator=(WriteOnlyReg &&) -> WriteOnlyReg & = delete;
    ~WriteOnlyReg() = default;
    /// @}

    /** Write a constant to the CSR. */
    template <WriteDataType VALUE>
    void WriteConst(void) {
      if constexpr ((VALUE & kCsrImmOpMask) == VALUE) {
        Reg::WriteImm(VALUE);
      } else {
        Reg::Write(VALUE);
      }
    }

    /** Write to the CSR. */
    inline void Write(const WriteDataType value) { Reg::Write(value); }

    /** Set a constant mask of bits in the CSR. */
    template <WriteDataType MASK>
    void SetConst(void) {
      if constexpr ((MASK & kCsrImmOpMask) == MASK) {
        Reg::SetBitsImm(MASK);
      } else {
        Reg::SetBits(MASK);
      }
    }

    /** Set a mask of bits in the CSR. */
    inline void Set(WriteDataType mask) { Reg::SetBits(mask); }

    /** Clear a constant mask of bits in the CSR. */
    template <WriteDataType MASK>
    void ClrConst() {
      if constexpr ((MASK & kCsrImmOpMask) == MASK) {
        Reg::ClrBitsImm(MASK);
      } else {
        Reg::ClrBits(MASK);
      }
    }

    /** Clear a mask of bits in the CSR. */
    inline void Clr(WriteDataType mask) { Reg::ClrBits(mask); }

    /** Operator alias to set mask of bits in the CSR. */
    inline void operator|=(WriteDataType mask) { Reg::SetBits(mask); }
  };

  /** CSR: Read-Write base class */
  template <class Reg>
  class ReadWriteReg : public ReadOnlyReg<Reg>, public WriteOnlyReg<Reg> {
   public:
    using DataType = typename Reg::DataType;

    ReadWriteReg() : ReadOnlyReg<Reg>(), WriteOnlyReg<Reg>() {}

    /// @name 构造/析构函数
    /// @{
    ReadWriteReg(const ReadWriteReg &) = delete;
    ReadWriteReg(ReadWriteReg &&) = delete;
    auto operator=(const ReadWriteReg &) -> ReadWriteReg & = delete;
    auto operator=(ReadWriteReg &&) -> ReadWriteReg & = delete;
    ~ReadWriteReg() = default;
    /// @}

    /** Read from, then Write a constant value to the CSR. */
    template <DataType VALUE>
    DataType ReadWriteConst() {
      if constexpr ((VALUE & kCsrImmOpMask) == VALUE) {
        return Reg::ReadWriteImm(VALUE);
      } else {
        return Reg::ReadWrite(VALUE);
      }
    }

    /** Read from, then Write to the CSR. */
    inline DataType ReadWrite(const DataType value) {
      return Reg::ReadWrite(value);
    }

    /** Read from, then set a constant bit mask to the CSR. */
    template <DataType MASK>
    DataType read_set_bits_const() {
      if constexpr ((MASK & kCsrImmOpMask) == MASK) {
        return Reg::ReadSetBitsImm(MASK);
      } else {
        return Reg::ReadSetBits(MASK);
      }
    }

    /** Read from, then set a bit mask to the CSR. */
    inline DataType ReadSetBits(const DataType mask) {
      return Reg::ReadSetBits(mask);
    }

    /** Read from, then clear a constant bit mask to the CSR. */
    template <DataType MASK>
    DataType read_clr_bits_const() {
      if constexpr ((MASK & kCsrImmOpMask) == MASK) {
        return Reg::ReadClrBitsImm(MASK);
      } else {
        return Reg::ReadClrBits(MASK);
      }
    }

    /** Read from, then clear a bit mask to the CSR. */
    inline DataType ReadClrBits(const DataType mask) {
      return Reg::ReadClrBits(mask);
    }
  };

  /** CSR Field: Read only, and Read-Write base class */
  template <class Reg, class Field>
  class ReadOnlyField {
   public:
    using ReadDataType = typename Field::DataType;

    /// @name 构造/析构函数
    /// @{
    ReadOnlyField() = default;
    ReadOnlyField(const ReadOnlyField &) = delete;
    ReadOnlyField(ReadOnlyField &&) = delete;
    auto operator=(const ReadOnlyField &) -> ReadOnlyField & = delete;
    auto operator=(ReadOnlyField &&) -> ReadOnlyField & = delete;
    ~ReadOnlyField() = default;
    /// @}

    /** Read a given field value from a CSR */
    ReadDataType Read() {
      return (ReadDataType)((Reg::Read() & Field::kBitMask) >>
                            Field::kBitOffset);
    }
  };

  /** CSR Field: Write only, and Read-Write base class */
  template <class Reg, class Field>
  class WriteOnlyField {
   public:
    using WriteDataType = typename Field::DataType;
    using reg_write_datatype_t = typename Reg::DataType;

    /// @name 构造/析构函数
    /// @{
    WriteOnlyField() = default;
    WriteOnlyField(const WriteOnlyField &) = delete;
    WriteOnlyField(WriteOnlyField &&) = delete;
    auto operator=(const WriteOnlyField &) -> WriteOnlyField & = delete;
    auto operator=(WriteOnlyField &&) -> WriteOnlyField & = delete;
    ~WriteOnlyField() = default;
    /// @}

    inline void Set() {
      if constexpr ((Field::kBitMask & kCsrImmOpMask) == Field::kBitMask) {
        Reg::SetBitsImm(Field::kBitMask);
      } else {
        Reg::SetBits(Field::kBitMask);
      }
    }
    inline void Clr() {
      if constexpr ((Field::kBitMask & kCsrImmOpMask) == Field::kBitMask) {
        Reg::ClrBitsImm(Field::kBitMask);
      } else {
        Reg::ClrBits(Field::kBitMask);
      }
    }
  };

  /** CSR Field: Read-Write base class */
  template <class Reg, class Field>
  class ReadWriteField : public ReadOnlyField<Reg, Field>,
                         public WriteOnlyField<Reg, Field> {
   public:
    using DataType = typename Field::DataType;
    using reg_datatype_t = typename Reg::DataType;

    ReadWriteField()
        : ReadOnlyField<Reg, Field>(), WriteOnlyField<Reg, Field>() {}

    /// @name 构造/析构函数
    /// @{
    ReadWriteField(const ReadWriteField &) = delete;
    ReadWriteField(ReadWriteField &&) = delete;
    auto operator=(const ReadWriteField &) -> ReadWriteField & = delete;
    auto operator=(ReadWriteField &&) -> ReadWriteField & = delete;
    ~ReadWriteField() = default;
    /// @}

    /* Read-modify-Write to Write a field.
       NOTE - not atomic.
     */
    inline void Write(const DataType value) {
      auto org_value = Reg::Read();
      auto new_value =
          (org_value & ~Field::kBitMask) |
          (((reg_datatype_t)value << Field::kBitOffset) & Field::kBitMask);
      Reg::Write(new_value);
    }

    /* Read-modify-Write to set a field, and return original value.
       NOTE - not atomic.
     */
    inline DataType ReadWrite(const DataType value) {
      auto org_value = Reg::Read();
      auto new_value =
          (org_value & ~Field::kBitMask) |
          (((reg_datatype_t)value << Field::kBitOffset) & Field::kBitMask);
      Reg::Write(new_value);
      return (DataType)((org_value & Field::kBitMask) >> Field::kBitOffset);
    }
  };

  /** CSR access context and Read/Write permission.
   */
  typedef enum {
    URW,
    URO,
    SRW,
    SRO,
    HRW,
    HRO,
    MRW,
    MRO,

    DRW,
    DRO,
  } priv_t;

  // ----------------------------------------------------------------
  // stval - SRW - Supervisor bad address or instruction.
  //
  /** Supervisor bad address or instruction. assembler operations */
  struct StvalOps {
    using DataType = uint64_t;
    static constexpr priv_t priv = SRW;

    /** Read stval */
    static uint64_t Read() {
      uint64_t value;
      __asm__ volatile("csrr %0, stval" : "=r"(value) : :);
      return value;
    }

    /** Write stval */
    static void Write(uint64_t value) {
      __asm__ volatile("csrw stval, %0" : : "r"(value) :);
    }

    /** Write immediate value to stval */
    static void WriteImm(uint64_t value) {
      __asm__ volatile("csrwi stval, %0" : : "i"(value) :);
    }

    /** Read and then Write to stval */
    static uint64_t ReadWrite(uint64_t new_value) {
      uint64_t prev_value;
      __asm__ volatile("csrrw %0, stval, %1"
                       : "=r"(prev_value)
                       : "r"(new_value)
                       :);
      return prev_value;
    }

    /** Read and then Write immediate value to stval */
    static uint64_t ReadWriteImm(const uint8_t new_value) {
      uint64_t prev_value;
      __asm__ volatile("csrrwi %0, stval, %1"
                       : "=r"(prev_value)
                       : "i"(new_value)
                       :);
      return prev_value;
    }

    // ------------------------------------------
    // Register CSR bit set and clear instructions

    /** Atomic modify and set bits for stval */
    static void SetBits(uint64_t mask) {
      __asm__ volatile("csrrs zero, stval, %0" : : "r"(mask) :);
    }

    /** Atomic Read and then and set bits for stval */
    static uint32_t ReadSetBits(uint64_t mask) {
      uint64_t value;
      __asm__ volatile("csrrs %0, stval, %1" : "=r"(value) : "r"(mask) :);
      return value;
    }

    /** Atomic modify and clear bits for stval */
    static void ClrBits(uint64_t mask) {
      __asm__ volatile("csrrc zero, stval, %0" : : "r"(mask) :);
    }

    /** Atomic Read and then and clear bits for stval */
    static uint32_t ReadClrBits(uint64_t mask) {
      uint64_t value;
      __asm__ volatile("csrrc %0, stval, %1" : "=r"(value) : "r"(mask) :);
      return value;
    }

    // ------------------------------------------
    // Immediate value CSR bit set and clear instructions (only up to 5 bits)

    /** Atomic modify and set bits from immediate for stval */
    static void SetBitsImm(const uint8_t mask) {
      __asm__ volatile("csrrsi zero, stval, %0" : : "i"(mask) :);
    }

    /** Atomic Read and then and set bits from immediate for stval */
    static uint64_t ReadSetBitsImm(const uint8_t mask) {
      uint64_t value;
      __asm__ volatile("csrrsi %0, stval, %1" : "=r"(value) : "i"(mask) :);
      return value;
    }

    /** Atomic modify and clear bits from immediate for stval */
    static void ClrBitsImm(const uint8_t mask) {
      __asm__ volatile("csrrci zero, stval, %0" : : "i"(mask) :);
    }

    /** Atomic Read and then and clear bits from immediate for stval */
    static uint64_t ReadClrBitsImm(const uint8_t mask) {
      uint64_t value;
      __asm__ volatile("csrrci %0, stval, %1" : "=r"(value) : "i"(mask) :);
      return value;
    }
  };

  // ----------------------------------------------------------------
  // satp - SRW - Supervisor address translation and protection.
  //
  /** Supervisor address translation and protection. assembler operations */
  struct SatpOps {
    using DataType = uint64_t;
    static constexpr priv_t priv = SRW;

    /** Read satp */
    static uint64_t Read() {
      uint64_t value;
      __asm__ volatile("csrr %0, satp" : "=r"(value) : :);
      return value;
    }

    /** Write satp */
    static void Write(uint64_t value) {
      __asm__ volatile("csrw satp, %0" : : "r"(value) :);
    }

    /** Write immediate value to satp */
    static void WriteImm(uint64_t value) {
      __asm__ volatile("csrwi satp, %0" : : "i"(value) :);
    }

    /** Read and then Write to satp */
    static uint64_t ReadWrite(uint64_t new_value) {
      uint64_t prev_value;
      __asm__ volatile("csrrw %0, satp, %1"
                       : "=r"(prev_value)
                       : "r"(new_value)
                       :);
      return prev_value;
    }

    /** Read and then Write immediate value to satp */
    static uint64_t ReadWriteImm(const uint8_t new_value) {
      uint64_t prev_value;
      __asm__ volatile("csrrwi %0, satp, %1"
                       : "=r"(prev_value)
                       : "i"(new_value)
                       :);
      return prev_value;
    }

    // ------------------------------------------
    // Register CSR bit set and clear instructions

    /** Atomic modify and set bits for satp */
    static void SetBits(uint64_t mask) {
      __asm__ volatile("csrrs zero, satp, %0" : : "r"(mask) :);
    }

    /** Atomic Read and then and set bits for satp */
    static uint32_t ReadSetBits(uint64_t mask) {
      uint64_t value;
      __asm__ volatile("csrrs %0, satp, %1" : "=r"(value) : "r"(mask) :);
      return value;
    }

    /** Atomic modify and clear bits for satp */
    static void ClrBits(uint64_t mask) {
      __asm__ volatile("csrrc zero, satp, %0" : : "r"(mask) :);
    }

    /** Atomic Read and then and clear bits for satp */
    static uint32_t ReadClrBits(uint64_t mask) {
      uint64_t value;
      __asm__ volatile("csrrc %0, satp, %1" : "=r"(value) : "r"(mask) :);
      return value;
    }

    // ------------------------------------------
    // Immediate value CSR bit set and clear instructions (only up to 5 bits)

    /** Atomic modify and set bits from immediate for satp */
    static void SetBitsImm(const uint8_t mask) {
      __asm__ volatile("csrrsi zero, satp, %0" : : "i"(mask) :);
    }

    /** Atomic Read and then and set bits from immediate for satp */
    static uint64_t ReadSetBitsImm(const uint8_t mask) {
      uint64_t value;
      __asm__ volatile("csrrsi %0, satp, %1" : "=r"(value) : "i"(mask) :);
      return value;
    }

    /** Atomic modify and clear bits from immediate for satp */
    static void ClrBitsImm(const uint8_t mask) {
      __asm__ volatile("csrrci zero, satp, %0" : : "i"(mask) :);
    }

    /** Atomic Read and then and clear bits from immediate for satp */
    static uint64_t ReadClrBitsImm(const uint8_t mask) {
      uint64_t value;
      __asm__ volatile("csrrci %0, satp, %1" : "=r"(value) : "i"(mask) :);
      return value;
    }
  };

//   /* Supervisor Status */
//   template <class RegOps>
//   class sstatus_reg : public ReadWriteReg<RegOps> {
//    public:
//     ReadWriteField<RegOps, SstatusData::sie> sie;
//     ReadWriteField<RegOps, SstatusData::spie> spie;
//     ReadWriteField<RegOps, SstatusData::spp> spp;
//   };
//   using sstatus = sstatus_reg<SstatusOps>;
//   /* Supervisor Trap Vector Base Address */
//   template <class RegOps>
//   class stvec_reg : public ReadWriteReg<RegOps> {
//    public:
//     ReadWriteField<RegOps, StvecData::base> base;
//     ReadWriteField<RegOps, StvecData::mode> mode;
//   };
//   using stvec = stvec_reg<StvecOps>;
//   /* Supervisor Interrupt Delegation */
//   template <class RegOps>
//   class sideleg_reg : public ReadWriteReg<RegOps> {};
//   using sideleg = sideleg_reg<SidelegOps>;
//   /* Supervisor Exception Delegation */
//   template <class RegOps>
//   class sedeleg_reg : public ReadWriteReg<RegOps> {};
//   using sedeleg = sedeleg_reg<SedelegOps>;
//   /* Supervisor Interrupt Pending */
//   template <class RegOps>
//   class sip_reg : public ReadWriteReg<RegOps> {
//    public:
//     ReadWriteField<RegOps, SipData::ssi> ssi;
//     ReadWriteField<RegOps, SipData::sti> sti;
//     ReadWriteField<RegOps, SipData::sei> sei;
//     ReadWriteField<RegOps, SipData::usi> usi;
//     ReadWriteField<RegOps, SipData::uti> uti;
//     ReadWriteField<RegOps, SipData::uei> uei;
//   };
//   using sip = sip_reg<SipOps>;
//   /* Supervisor Interrupt Enable */
//   template <class RegOps>
//   class sie_reg : public ReadWriteReg<RegOps> {
//    public:
//     ReadWriteField<RegOps, SieData::ssi> ssi;
//     ReadWriteField<RegOps, SieData::sti> sti;
//     ReadWriteField<RegOps, SieData::sei> sei;
//     ReadWriteField<RegOps, SieData::usi> usi;
//     ReadWriteField<RegOps, SieData::uti> uti;
//     ReadWriteField<RegOps, SieData::uei> uei;
//   };
//   using sie = sie_reg<SieOps>;

//   /* Supervisor bad address or instruction. */
//   template <class RegOps>
//   class stval_reg : public ReadWriteReg<RegOps> {};
//   using stval = stval_reg<StvalOps>;
//   /* Supervisor address translation and protection. */
//   template <class RegOps>
//   class satp_reg : public ReadWriteReg<RegOps> {};
//   using satp = satp_reg<SatpOps>;

//   /** Encapsulate all CSRs in a single structure.
//      - No storage is required by this class.
//    */
//   struct all {
//     /* Supervisor Mode Scratch Register */
//     sscratch sscratch;
//     /* Supervisor Exception Program Counter */
//     sepc sepc;
//     /* Supervisor Exception Cause */
//     scause scause;
//     /* Supervisor Status */
//     sstatus sstatus;
//     /* Supervisor Trap Vector Base Address */
//     stvec stvec;
//     /* Supervisor Interrupt Delegation */
//     sideleg sideleg;
//     /* Supervisor Exception Delegation */
//     sedeleg sedeleg;
//     /* Supervisor Interrupt Pending */
//     sip sip;
//     /* Supervisor Interrupt Enable */
//     sie sie;
//     /* Supervisor bad address or instruction. */
//     stval stval;
//     /* Supervisor address translation and protection. */
//     satp satp;
//   };

//   static all csrs;
};

#endif  // SIMPLEKERNEL_SRC_KERNEL_ARCH_RISCV64_CSR_HPP_
