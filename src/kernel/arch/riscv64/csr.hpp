

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
  // sscratch - SRW - Supervisor Mode Scratch Register
  //
  /** Supervisor Mode Scratch Register assembler operations */
  struct SscratchOps {
    using DataType = uint64_t;
    static constexpr priv_t priv = SRW;

    /** Read sscratch */
    static uint64_t Read() {
      uint64_t value;
      __asm__ volatile("csrr %0, sscratch" : "=r"(value) : :);
      return value;
    }

    /** Write sscratch */
    static void Write(uint64_t value) {
      __asm__ volatile("csrw sscratch, %0" : : "r"(value) :);
    }

    /** Write immediate value to sscratch */
    static void WriteImm(uint64_t value) {
      __asm__ volatile("csrwi sscratch, %0" : : "i"(value) :);
    }

    /** Read and then Write to sscratch */
    static uint64_t ReadWrite(uint64_t new_value) {
      uint64_t prev_value;
      __asm__ volatile("csrrw %0, sscratch, %1"
                       : "=r"(prev_value)
                       : "r"(new_value)
                       :);
      return prev_value;
    }

    /** Read and then Write immediate value to sscratch */
    static uint64_t ReadWriteImm(const uint8_t new_value) {
      uint64_t prev_value;
      __asm__ volatile("csrrwi %0, sscratch, %1"
                       : "=r"(prev_value)
                       : "i"(new_value)
                       :);
      return prev_value;
    }

    // ------------------------------------------
    // Register CSR bit set and clear instructions

    /** Atomic modify and set bits for sscratch */
    static void SetBits(uint64_t mask) {
      __asm__ volatile("csrrs zero, sscratch, %0" : : "r"(mask) :);
    }

    /** Atomic Read and then and set bits for sscratch */
    static uint32_t ReadSetBits(uint64_t mask) {
      uint64_t value;
      __asm__ volatile("csrrs %0, sscratch, %1" : "=r"(value) : "r"(mask) :);
      return value;
    }

    /** Atomic modify and clear bits for sscratch */
    static void ClrBits(uint64_t mask) {
      __asm__ volatile("csrrc zero, sscratch, %0" : : "r"(mask) :);
    }

    /** Atomic Read and then and clear bits for sscratch */
    static uint32_t ReadClrBits(uint64_t mask) {
      uint64_t value;
      __asm__ volatile("csrrc %0, sscratch, %1" : "=r"(value) : "r"(mask) :);
      return value;
    }

    // ------------------------------------------
    // Immediate value CSR bit set and clear instructions (only up to 5 bits)

    /** Atomic modify and set bits from immediate for sscratch */
    static void SetBitsImm(const uint8_t mask) {
      __asm__ volatile("csrrsi zero, sscratch, %0" : : "i"(mask) :);
    }

    /** Atomic Read and then and set bits from immediate for sscratch */
    static uint64_t ReadSetBitsImm(const uint8_t mask) {
      uint64_t value;
      __asm__ volatile("csrrsi %0, sscratch, %1" : "=r"(value) : "i"(mask) :);
      return value;
    }

    /** Atomic modify and clear bits from immediate for sscratch */
    static void ClrBitsImm(const uint8_t mask) {
      __asm__ volatile("csrrci zero, sscratch, %0" : : "i"(mask) :);
    }

    /** Atomic Read and then and clear bits from immediate for sscratch */
    static uint64_t ReadClrBitsImm(const uint8_t mask) {
      uint64_t value;
      __asm__ volatile("csrrci %0, sscratch, %1" : "=r"(value) : "i"(mask) :);
      return value;
    }

  }; /* SscratchOps */

  // ----------------------------------------------------------------
  // sepc - SRW - Supervisor Exception Program Counter
  //
  /** Supervisor Exception Program Counter assembler operations */
  struct SepcOps {
    using DataType = uint64_t;
    static constexpr priv_t priv = SRW;

    /** Read sepc */
    static uint64_t Read() {
      uint64_t value;
      __asm__ volatile("csrr %0, sepc" : "=r"(value) : :);
      return value;
    }

    /** Write sepc */
    static void Write(uint64_t value) {
      __asm__ volatile("csrw sepc, %0" : : "r"(value) :);
    }

    /** Write immediate value to sepc */
    static void WriteImm(uint64_t value) {
      __asm__ volatile("csrwi sepc, %0" : : "i"(value) :);
    }

    /** Read and then Write to sepc */
    static uint64_t ReadWrite(uint64_t new_value) {
      uint64_t prev_value;
      __asm__ volatile("csrrw %0, sepc, %1"
                       : "=r"(prev_value)
                       : "r"(new_value)
                       :);
      return prev_value;
    }

    /** Read and then Write immediate value to sepc */
    static uint64_t ReadWriteImm(const uint8_t new_value) {
      uint64_t prev_value;
      __asm__ volatile("csrrwi %0, sepc, %1"
                       : "=r"(prev_value)
                       : "i"(new_value)
                       :);
      return prev_value;
    }

    // ------------------------------------------
    // Register CSR bit set and clear instructions

    /** Atomic modify and set bits for sepc */
    static void SetBits(uint64_t mask) {
      __asm__ volatile("csrrs zero, sepc, %0" : : "r"(mask) :);
    }

    /** Atomic Read and then and set bits for sepc */
    static uint32_t ReadSetBits(uint64_t mask) {
      uint64_t value;
      __asm__ volatile("csrrs %0, sepc, %1" : "=r"(value) : "r"(mask) :);
      return value;
    }

    /** Atomic modify and clear bits for sepc */
    static void ClrBits(uint64_t mask) {
      __asm__ volatile("csrrc zero, sepc, %0" : : "r"(mask) :);
    }

    /** Atomic Read and then and clear bits for sepc */
    static uint32_t ReadClrBits(uint64_t mask) {
      uint64_t value;
      __asm__ volatile("csrrc %0, sepc, %1" : "=r"(value) : "r"(mask) :);
      return value;
    }

    // ------------------------------------------
    // Immediate value CSR bit set and clear instructions (only up to 5 bits)

    /** Atomic modify and set bits from immediate for sepc */
    static void SetBitsImm(const uint8_t mask) {
      __asm__ volatile("csrrsi zero, sepc, %0" : : "i"(mask) :);
    }

    /** Atomic Read and then and set bits from immediate for sepc */
    static uint64_t ReadSetBitsImm(const uint8_t mask) {
      uint64_t value;
      __asm__ volatile("csrrsi %0, sepc, %1" : "=r"(value) : "i"(mask) :);
      return value;
    }

    /** Atomic modify and clear bits from immediate for sepc */
    static void ClrBitsImm(const uint8_t mask) {
      __asm__ volatile("csrrci zero, sepc, %0" : : "i"(mask) :);
    }

    /** Atomic Read and then and clear bits from immediate for sepc */
    static uint64_t ReadClrBitsImm(const uint8_t mask) {
      uint64_t value;
      __asm__ volatile("csrrci %0, sepc, %1" : "=r"(value) : "i"(mask) :);
      return value;
    }

  }; /* SepcOps */

  // ----------------------------------------------------------------
  // scause - SRW - Supervisor Exception Cause
  //
  /** Supervisor Exception Cause assembler operations */
  struct ScauseOps {
    using DataType = uint64_t;
    static constexpr priv_t priv = SRW;

    /** Read scause */
    static uint64_t Read() {
      uint64_t value;
      __asm__ volatile("csrr %0, scause" : "=r"(value) : :);
      return value;
    }

    /** Write scause */
    static void Write(uint64_t value) {
      __asm__ volatile("csrw scause, %0" : : "r"(value) :);
    }

    /** Write immediate value to scause */
    static void WriteImm(uint64_t value) {
      __asm__ volatile("csrwi scause, %0" : : "i"(value) :);
    }

    /** Read and then Write to scause */
    static uint64_t ReadWrite(uint64_t new_value) {
      uint64_t prev_value;
      __asm__ volatile("csrrw %0, scause, %1"
                       : "=r"(prev_value)
                       : "r"(new_value)
                       :);
      return prev_value;
    }

    /** Read and then Write immediate value to scause */
    static uint64_t ReadWriteImm(const uint8_t new_value) {
      uint64_t prev_value;
      __asm__ volatile("csrrwi %0, scause, %1"
                       : "=r"(prev_value)
                       : "i"(new_value)
                       :);
      return prev_value;
    }

    // ------------------------------------------
    // Register CSR bit set and clear instructions

    /** Atomic modify and set bits for scause */
    static void SetBits(uint64_t mask) {
      __asm__ volatile("csrrs zero, scause, %0" : : "r"(mask) :);
    }

    /** Atomic Read and then and set bits for scause */
    static uint32_t ReadSetBits(uint64_t mask) {
      uint64_t value;
      __asm__ volatile("csrrs %0, scause, %1" : "=r"(value) : "r"(mask) :);
      return value;
    }

    /** Atomic modify and clear bits for scause */
    static void ClrBits(uint64_t mask) {
      __asm__ volatile("csrrc zero, scause, %0" : : "r"(mask) :);
    }

    /** Atomic Read and then and clear bits for scause */
    static uint32_t ReadClrBits(uint64_t mask) {
      uint64_t value;
      __asm__ volatile("csrrc %0, scause, %1" : "=r"(value) : "r"(mask) :);
      return value;
    }

    // ------------------------------------------
    // Immediate value CSR bit set and clear instructions (only up to 5 bits)

    /** Atomic modify and set bits from immediate for scause */
    static void SetBitsImm(const uint8_t mask) {
      __asm__ volatile("csrrsi zero, scause, %0" : : "i"(mask) :);
    }

    /** Atomic Read and then and set bits from immediate for scause */
    static uint64_t ReadSetBitsImm(const uint8_t mask) {
      uint64_t value;
      __asm__ volatile("csrrsi %0, scause, %1" : "=r"(value) : "i"(mask) :);
      return value;
    }

    /** Atomic modify and clear bits from immediate for scause */
    static void ClrBitsImm(const uint8_t mask) {
      __asm__ volatile("csrrci zero, scause, %0" : : "i"(mask) :);
    }

    /** Atomic Read and then and clear bits from immediate for scause */
    static uint64_t ReadClrBitsImm(const uint8_t mask) {
      uint64_t value;
      __asm__ volatile("csrrci %0, scause, %1" : "=r"(value) : "i"(mask) :);
      return value;
    }

  }; /* ScauseOps */
  /** Parameter data for fields in scause */
  struct ScauseData {
    /** Parameter data for interrupt */
    struct interrupt {
      using DataType = uint64_t;
      static constexpr uint64_t kBitOffset = (64 - 1);
      static constexpr uint64_t kBitWidth = 1;
      static constexpr uint64_t kBitMask = (0x1UL << ((64 - 1)));
      static constexpr uint64_t kAllSetMask = 0x1;
    };
    /** Parameter data for exception_code */
    struct exception_code {
      using DataType = uint64_t;
      static constexpr uint64_t kBitOffset = 0;
      static constexpr uint64_t kBitWidth = ((64 - 2) - (0) + 1);
      static constexpr uint64_t kBitMask =
          ((1UL << (((64 - 2) - (0) + 1) - 1)) << (0));
      static constexpr uint64_t kAllSetMask =
          ((1UL << (((64 - 2) - (0) + 1) - 1)) << (0));
    };
  };

  // ----------------------------------------------------------------
  // sstatus - SRW - Supervisor Status
  //
  /** Supervisor Status assembler operations */
  struct SstatusOps {
    using DataType = uint64_t;
    static constexpr priv_t priv = SRW;

    /** Read sstatus */
    static uint64_t Read() {
      uint64_t value;
      __asm__ volatile("csrr %0, sstatus" : "=r"(value) : :);
      return value;
    }

    /** Write sstatus */
    static void Write(uint64_t value) {
      __asm__ volatile("csrw sstatus, %0" : : "r"(value) :);
    }

    /** Write immediate value to sstatus */
    static void WriteImm(uint64_t value) {
      __asm__ volatile("csrwi sstatus, %0" : : "i"(value) :);
    }

    /** Read and then Write to sstatus */
    static uint64_t ReadWrite(uint64_t new_value) {
      uint64_t prev_value;
      __asm__ volatile("csrrw %0, sstatus, %1"
                       : "=r"(prev_value)
                       : "r"(new_value)
                       :);
      return prev_value;
    }

    /** Read and then Write immediate value to sstatus */
    static uint64_t ReadWriteImm(const uint8_t new_value) {
      uint64_t prev_value;
      __asm__ volatile("csrrwi %0, sstatus, %1"
                       : "=r"(prev_value)
                       : "i"(new_value)
                       :);
      return prev_value;
    }

    // ------------------------------------------
    // Register CSR bit set and clear instructions

    /** Atomic modify and set bits for sstatus */
    static void SetBits(uint64_t mask) {
      __asm__ volatile("csrrs zero, sstatus, %0" : : "r"(mask) :);
    }

    /** Atomic Read and then and set bits for sstatus */
    static uint32_t ReadSetBits(uint64_t mask) {
      uint64_t value;
      __asm__ volatile("csrrs %0, sstatus, %1" : "=r"(value) : "r"(mask) :);
      return value;
    }

    /** Atomic modify and clear bits for sstatus */
    static void ClrBits(uint64_t mask) {
      __asm__ volatile("csrrc zero, sstatus, %0" : : "r"(mask) :);
    }

    /** Atomic Read and then and clear bits for sstatus */
    static uint32_t ReadClrBits(uint64_t mask) {
      uint64_t value;
      __asm__ volatile("csrrc %0, sstatus, %1" : "=r"(value) : "r"(mask) :);
      return value;
    }

    // ------------------------------------------
    // Immediate value CSR bit set and clear instructions (only up to 5 bits)

    /** Atomic modify and set bits from immediate for sstatus */
    static void SetBitsImm(const uint8_t mask) {
      __asm__ volatile("csrrsi zero, sstatus, %0" : : "i"(mask) :);
    }

    /** Atomic Read and then and set bits from immediate for sstatus */
    static uint64_t ReadSetBitsImm(const uint8_t mask) {
      uint64_t value;
      __asm__ volatile("csrrsi %0, sstatus, %1" : "=r"(value) : "i"(mask) :);
      return value;
    }

    /** Atomic modify and clear bits from immediate for sstatus */
    static void ClrBitsImm(const uint8_t mask) {
      __asm__ volatile("csrrci zero, sstatus, %0" : : "i"(mask) :);
    }

    /** Atomic Read and then and clear bits from immediate for sstatus */
    static uint64_t ReadClrBitsImm(const uint8_t mask) {
      uint64_t value;
      __asm__ volatile("csrrci %0, sstatus, %1" : "=r"(value) : "i"(mask) :);
      return value;
    }

  }; /* SstatusOps */
  /** Parameter data for fields in sstatus */
  struct SstatusData {
    /** Parameter data for sie */
    struct sie {
      using DataType = uint64_t;
      static constexpr uint64_t kBitOffset = 2;
      static constexpr uint64_t kBitWidth = 1;
      static constexpr uint64_t kBitMask = 0x4;
      static constexpr uint64_t kAllSetMask = 0x1;
    };
    /** Parameter data for spie */
    struct spie {
      using DataType = uint64_t;
      static constexpr uint64_t kBitOffset = 5;
      static constexpr uint64_t kBitWidth = 1;
      static constexpr uint64_t kBitMask = 0x20;
      static constexpr uint64_t kAllSetMask = 0x1;
    };
    /** Parameter data for spp */
    struct spp {
      using DataType = uint64_t;
      static constexpr uint64_t kBitOffset = 8;
      static constexpr uint64_t kBitWidth = 1;
      static constexpr uint64_t kBitMask = 0x100;
      static constexpr uint64_t kAllSetMask = 0x1;
    };
  };

  // ----------------------------------------------------------------
  // stvec - SRW - Supervisor Trap Vector Base Address
  //
  /** Supervisor Trap Vector Base Address assembler operations */
  struct StvecOps {
    using DataType = uint64_t;
    static constexpr priv_t priv = SRW;

    /** Read stvec */
    static uint64_t Read() {
      uint64_t value;
      __asm__ volatile("csrr %0, stvec" : "=r"(value) : :);
      return value;
    }

    /** Write stvec */
    static void Write(uint64_t value) {
      __asm__ volatile("csrw stvec, %0" : : "r"(value) :);
    }

    /** Write immediate value to stvec */
    static void WriteImm(uint64_t value) {
      __asm__ volatile("csrwi stvec, %0" : : "i"(value) :);
    }

    /** Read and then Write to stvec */
    static uint64_t ReadWrite(uint64_t new_value) {
      uint64_t prev_value;
      __asm__ volatile("csrrw %0, stvec, %1"
                       : "=r"(prev_value)
                       : "r"(new_value)
                       :);
      return prev_value;
    }

    /** Read and then Write immediate value to stvec */
    static uint64_t ReadWriteImm(const uint8_t new_value) {
      uint64_t prev_value;
      __asm__ volatile("csrrwi %0, stvec, %1"
                       : "=r"(prev_value)
                       : "i"(new_value)
                       :);
      return prev_value;
    }

    // ------------------------------------------
    // Register CSR bit set and clear instructions

    /** Atomic modify and set bits for stvec */
    static void SetBits(uint64_t mask) {
      __asm__ volatile("csrrs zero, stvec, %0" : : "r"(mask) :);
    }

    /** Atomic Read and then and set bits for stvec */
    static uint32_t ReadSetBits(uint64_t mask) {
      uint64_t value;
      __asm__ volatile("csrrs %0, stvec, %1" : "=r"(value) : "r"(mask) :);
      return value;
    }

    /** Atomic modify and clear bits for stvec */
    static void ClrBits(uint64_t mask) {
      __asm__ volatile("csrrc zero, stvec, %0" : : "r"(mask) :);
    }

    /** Atomic Read and then and clear bits for stvec */
    static uint32_t ReadClrBits(uint64_t mask) {
      uint64_t value;
      __asm__ volatile("csrrc %0, stvec, %1" : "=r"(value) : "r"(mask) :);
      return value;
    }

    // ------------------------------------------
    // Immediate value CSR bit set and clear instructions (only up to 5 bits)

    /** Atomic modify and set bits from immediate for stvec */
    static void SetBitsImm(const uint8_t mask) {
      __asm__ volatile("csrrsi zero, stvec, %0" : : "i"(mask) :);
    }

    /** Atomic Read and then and set bits from immediate for stvec */
    static uint64_t ReadSetBitsImm(const uint8_t mask) {
      uint64_t value;
      __asm__ volatile("csrrsi %0, stvec, %1" : "=r"(value) : "i"(mask) :);
      return value;
    }

    /** Atomic modify and clear bits from immediate for stvec */
    static void ClrBitsImm(const uint8_t mask) {
      __asm__ volatile("csrrci zero, stvec, %0" : : "i"(mask) :);
    }

    /** Atomic Read and then and clear bits from immediate for stvec */
    static uint64_t ReadClrBitsImm(const uint8_t mask) {
      uint64_t value;
      __asm__ volatile("csrrci %0, stvec, %1" : "=r"(value) : "i"(mask) :);
      return value;
    }
  };

  /** Parameter data for fields in stvec */
  struct StvecData {
    /** Parameter data for base */
    struct base {
      using DataType = uint64_t;
      static constexpr uint64_t kBitOffset = 2;
      static constexpr uint64_t kBitWidth = ((64 - 1) - (2) + 1);
      static constexpr uint64_t kBitMask =
          ((1UL << (((64 - 1) - (2) + 1) - 1)) << (2));
      static constexpr uint64_t kAllSetMask =
          ((1UL << (((64 - 1) - (2) + 1) - 1)) << (0));
    };
    /** Parameter data for mode */
    struct mode {
      using DataType = uint64_t;
      static constexpr uint64_t kBitOffset = 0;
      static constexpr uint64_t kBitWidth = 2;
      static constexpr uint64_t kBitMask = 0x3;
      static constexpr uint64_t kAllSetMask = 0x3;
    };
  };

  // ----------------------------------------------------------------
  // sideleg - SRW - Supervisor Interrupt Delegation
  //
  /** Supervisor Interrupt Delegation assembler operations */
  struct SidelegOps {
    using DataType = uint64_t;
    static constexpr priv_t priv = SRW;

    /** Read sideleg */
    static uint64_t Read() {
      uint64_t value;
      __asm__ volatile("csrr %0, sideleg" : "=r"(value) : :);
      return value;
    }

    /** Write sideleg */
    static void Write(uint64_t value) {
      __asm__ volatile("csrw sideleg, %0" : : "r"(value) :);
    }

    /** Write immediate value to sideleg */
    static void WriteImm(uint64_t value) {
      __asm__ volatile("csrwi sideleg, %0" : : "i"(value) :);
    }

    /** Read and then Write to sideleg */
    static uint64_t ReadWrite(uint64_t new_value) {
      uint64_t prev_value;
      __asm__ volatile("csrrw %0, sideleg, %1"
                       : "=r"(prev_value)
                       : "r"(new_value)
                       :);
      return prev_value;
    }

    /** Read and then Write immediate value to sideleg */
    static uint64_t ReadWriteImm(const uint8_t new_value) {
      uint64_t prev_value;
      __asm__ volatile("csrrwi %0, sideleg, %1"
                       : "=r"(prev_value)
                       : "i"(new_value)
                       :);
      return prev_value;
    }

    // ------------------------------------------
    // Register CSR bit set and clear instructions

    /** Atomic modify and set bits for sideleg */
    static void SetBits(uint64_t mask) {
      __asm__ volatile("csrrs zero, sideleg, %0" : : "r"(mask) :);
    }

    /** Atomic Read and then and set bits for sideleg */
    static uint32_t ReadSetBits(uint64_t mask) {
      uint64_t value;
      __asm__ volatile("csrrs %0, sideleg, %1" : "=r"(value) : "r"(mask) :);
      return value;
    }

    /** Atomic modify and clear bits for sideleg */
    static void ClrBits(uint64_t mask) {
      __asm__ volatile("csrrc zero, sideleg, %0" : : "r"(mask) :);
    }

    /** Atomic Read and then and clear bits for sideleg */
    static uint32_t ReadClrBits(uint64_t mask) {
      uint64_t value;
      __asm__ volatile("csrrc %0, sideleg, %1" : "=r"(value) : "r"(mask) :);
      return value;
    }

    // ------------------------------------------
    // Immediate value CSR bit set and clear instructions (only up to 5 bits)

    /** Atomic modify and set bits from immediate for sideleg */
    static void SetBitsImm(const uint8_t mask) {
      __asm__ volatile("csrrsi zero, sideleg, %0" : : "i"(mask) :);
    }

    /** Atomic Read and then and set bits from immediate for sideleg */
    static uint64_t ReadSetBitsImm(const uint8_t mask) {
      uint64_t value;
      __asm__ volatile("csrrsi %0, sideleg, %1" : "=r"(value) : "i"(mask) :);
      return value;
    }

    /** Atomic modify and clear bits from immediate for sideleg */
    static void ClrBitsImm(const uint8_t mask) {
      __asm__ volatile("csrrci zero, sideleg, %0" : : "i"(mask) :);
    }

    /** Atomic Read and then and clear bits from immediate for sideleg */
    static uint64_t ReadClrBitsImm(const uint8_t mask) {
      uint64_t value;
      __asm__ volatile("csrrci %0, sideleg, %1" : "=r"(value) : "i"(mask) :);
      return value;
    }
  };

  // ----------------------------------------------------------------
  // sedeleg - SRW - Supervisor Exception Delegation
  //
  /** Supervisor Exception Delegation assembler operations */
  struct SedelegOps {
    using DataType = uint64_t;
    static constexpr priv_t priv = SRW;

    /** Read sedeleg */
    static uint64_t Read() {
      uint64_t value;
      __asm__ volatile("csrr %0, sedeleg" : "=r"(value) : :);
      return value;
    }

    /** Write sedeleg */
    static void Write(uint64_t value) {
      __asm__ volatile("csrw sedeleg, %0" : : "r"(value) :);
    }

    /** Write immediate value to sedeleg */
    static void WriteImm(uint64_t value) {
      __asm__ volatile("csrwi sedeleg, %0" : : "i"(value) :);
    }

    /** Read and then Write to sedeleg */
    static uint64_t ReadWrite(uint64_t new_value) {
      uint64_t prev_value;
      __asm__ volatile("csrrw %0, sedeleg, %1"
                       : "=r"(prev_value)
                       : "r"(new_value)
                       :);
      return prev_value;
    }

    /** Read and then Write immediate value to sedeleg */
    static uint64_t ReadWriteImm(const uint8_t new_value) {
      uint64_t prev_value;
      __asm__ volatile("csrrwi %0, sedeleg, %1"
                       : "=r"(prev_value)
                       : "i"(new_value)
                       :);
      return prev_value;
    }

    // ------------------------------------------
    // Register CSR bit set and clear instructions

    /** Atomic modify and set bits for sedeleg */
    static void SetBits(uint64_t mask) {
      __asm__ volatile("csrrs zero, sedeleg, %0" : : "r"(mask) :);
    }

    /** Atomic Read and then and set bits for sedeleg */
    static uint32_t ReadSetBits(uint64_t mask) {
      uint64_t value;
      __asm__ volatile("csrrs %0, sedeleg, %1" : "=r"(value) : "r"(mask) :);
      return value;
    }

    /** Atomic modify and clear bits for sedeleg */
    static void ClrBits(uint64_t mask) {
      __asm__ volatile("csrrc zero, sedeleg, %0" : : "r"(mask) :);
    }

    /** Atomic Read and then and clear bits for sedeleg */
    static uint32_t ReadClrBits(uint64_t mask) {
      uint64_t value;
      __asm__ volatile("csrrc %0, sedeleg, %1" : "=r"(value) : "r"(mask) :);
      return value;
    }

    // ------------------------------------------
    // Immediate value CSR bit set and clear instructions (only up to 5 bits)

    /** Atomic modify and set bits from immediate for sedeleg */
    static void SetBitsImm(const uint8_t mask) {
      __asm__ volatile("csrrsi zero, sedeleg, %0" : : "i"(mask) :);
    }

    /** Atomic Read and then and set bits from immediate for sedeleg */
    static uint64_t ReadSetBitsImm(const uint8_t mask) {
      uint64_t value;
      __asm__ volatile("csrrsi %0, sedeleg, %1" : "=r"(value) : "i"(mask) :);
      return value;
    }

    /** Atomic modify and clear bits from immediate for sedeleg */
    static void ClrBitsImm(const uint8_t mask) {
      __asm__ volatile("csrrci zero, sedeleg, %0" : : "i"(mask) :);
    }

    /** Atomic Read and then and clear bits from immediate for sedeleg */
    static uint64_t ReadClrBitsImm(const uint8_t mask) {
      uint64_t value;
      __asm__ volatile("csrrci %0, sedeleg, %1" : "=r"(value) : "i"(mask) :);
      return value;
    }
  };

  // ----------------------------------------------------------------
  // sip - SRW - Supervisor Interrupt Pending
  //
  /** Supervisor Interrupt Pending assembler operations */
  struct SipOps {
    using DataType = uint64_t;
    static constexpr priv_t priv = SRW;

    /** Read sip */
    static uint64_t Read() {
      uint64_t value;
      __asm__ volatile("csrr %0, sip" : "=r"(value) : :);
      return value;
    }

    /** Write sip */
    static void Write(uint64_t value) {
      __asm__ volatile("csrw sip, %0" : : "r"(value) :);
    }

    /** Write immediate value to sip */
    static void WriteImm(uint64_t value) {
      __asm__ volatile("csrwi sip, %0" : : "i"(value) :);
    }

    /** Read and then Write to sip */
    static uint64_t ReadWrite(uint64_t new_value) {
      uint64_t prev_value;
      __asm__ volatile("csrrw %0, sip, %1"
                       : "=r"(prev_value)
                       : "r"(new_value)
                       :);
      return prev_value;
    }

    /** Read and then Write immediate value to sip */
    static uint64_t ReadWriteImm(const uint8_t new_value) {
      uint64_t prev_value;
      __asm__ volatile("csrrwi %0, sip, %1"
                       : "=r"(prev_value)
                       : "i"(new_value)
                       :);
      return prev_value;
    }

    // ------------------------------------------
    // Register CSR bit set and clear instructions

    /** Atomic modify and set bits for sip */
    static void SetBits(uint64_t mask) {
      __asm__ volatile("csrrs zero, sip, %0" : : "r"(mask) :);
    }

    /** Atomic Read and then and set bits for sip */
    static uint32_t ReadSetBits(uint64_t mask) {
      uint64_t value;
      __asm__ volatile("csrrs %0, sip, %1" : "=r"(value) : "r"(mask) :);
      return value;
    }

    /** Atomic modify and clear bits for sip */
    static void ClrBits(uint64_t mask) {
      __asm__ volatile("csrrc zero, sip, %0" : : "r"(mask) :);
    }

    /** Atomic Read and then and clear bits for sip */
    static uint32_t ReadClrBits(uint64_t mask) {
      uint64_t value;
      __asm__ volatile("csrrc %0, sip, %1" : "=r"(value) : "r"(mask) :);
      return value;
    }

    // ------------------------------------------
    // Immediate value CSR bit set and clear instructions (only up to 5 bits)

    /** Atomic modify and set bits from immediate for sip */
    static void SetBitsImm(const uint8_t mask) {
      __asm__ volatile("csrrsi zero, sip, %0" : : "i"(mask) :);
    }

    /** Atomic Read and then and set bits from immediate for sip */
    static uint64_t ReadSetBitsImm(const uint8_t mask) {
      uint64_t value;
      __asm__ volatile("csrrsi %0, sip, %1" : "=r"(value) : "i"(mask) :);
      return value;
    }

    /** Atomic modify and clear bits from immediate for sip */
    static void ClrBitsImm(const uint8_t mask) {
      __asm__ volatile("csrrci zero, sip, %0" : : "i"(mask) :);
    }

    /** Atomic Read and then and clear bits from immediate for sip */
    static uint64_t ReadClrBitsImm(const uint8_t mask) {
      uint64_t value;
      __asm__ volatile("csrrci %0, sip, %1" : "=r"(value) : "i"(mask) :);
      return value;
    }
  };

  /** Parameter data for fields in sip */
  struct SipData {
    /** Parameter data for ssi */
    struct ssi {
      using DataType = uint64_t;
      static constexpr uint64_t kBitOffset = 1;
      static constexpr uint64_t kBitWidth = 1;
      static constexpr uint64_t kBitMask = 0x2;
      static constexpr uint64_t kAllSetMask = 0x1;
    };
    /** Parameter data for sti */
    struct sti {
      using DataType = uint64_t;
      static constexpr uint64_t kBitOffset = 5;
      static constexpr uint64_t kBitWidth = 1;
      static constexpr uint64_t kBitMask = 0x20;
      static constexpr uint64_t kAllSetMask = 0x1;
    };
    /** Parameter data for sei */
    struct sei {
      using DataType = uint64_t;
      static constexpr uint64_t kBitOffset = 9;
      static constexpr uint64_t kBitWidth = 1;
      static constexpr uint64_t kBitMask = 0x200;
      static constexpr uint64_t kAllSetMask = 0x1;
    };
    /** Parameter data for usi */
    struct usi {
      using DataType = uint64_t;
      static constexpr uint64_t kBitOffset = 0;
      static constexpr uint64_t kBitWidth = 1;
      static constexpr uint64_t kBitMask = 0x1;
      static constexpr uint64_t kAllSetMask = 0x1;
    };
    /** Parameter data for uti */
    struct uti {
      using DataType = uint64_t;
      static constexpr uint64_t kBitOffset = 4;
      static constexpr uint64_t kBitWidth = 1;
      static constexpr uint64_t kBitMask = 0x10;
      static constexpr uint64_t kAllSetMask = 0x1;
    };
    /** Parameter data for uei */
    struct uei {
      using DataType = uint64_t;
      static constexpr uint64_t kBitOffset = 8;
      static constexpr uint64_t kBitWidth = 1;
      static constexpr uint64_t kBitMask = 0x100;
      static constexpr uint64_t kAllSetMask = 0x1;
    };
  };

  // ----------------------------------------------------------------
  // sie - SRW - Supervisor Interrupt Enable
  //
  /** Supervisor Interrupt Enable assembler operations */
  struct SieOps {
    using DataType = uint64_t;
    static constexpr priv_t priv = SRW;

    /** Read sie */
    static uint64_t Read() {
      uint64_t value;
      __asm__ volatile("csrr %0, sie" : "=r"(value) : :);
      return value;
    }

    /** Write sie */
    static void Write(uint64_t value) {
      __asm__ volatile("csrw sie, %0" : : "r"(value) :);
    }

    /** Write immediate value to sie */
    static void WriteImm(uint64_t value) {
      __asm__ volatile("csrwi sie, %0" : : "i"(value) :);
    }

    /** Read and then Write to sie */
    static uint64_t ReadWrite(uint64_t new_value) {
      uint64_t prev_value;
      __asm__ volatile("csrrw %0, sie, %1"
                       : "=r"(prev_value)
                       : "r"(new_value)
                       :);
      return prev_value;
    }

    /** Read and then Write immediate value to sie */
    static uint64_t ReadWriteImm(const uint8_t new_value) {
      uint64_t prev_value;
      __asm__ volatile("csrrwi %0, sie, %1"
                       : "=r"(prev_value)
                       : "i"(new_value)
                       :);
      return prev_value;
    }

    // ------------------------------------------
    // Register CSR bit set and clear instructions

    /** Atomic modify and set bits for sie */
    static void SetBits(uint64_t mask) {
      __asm__ volatile("csrrs zero, sie, %0" : : "r"(mask) :);
    }

    /** Atomic Read and then and set bits for sie */
    static uint32_t ReadSetBits(uint64_t mask) {
      uint64_t value;
      __asm__ volatile("csrrs %0, sie, %1" : "=r"(value) : "r"(mask) :);
      return value;
    }

    /** Atomic modify and clear bits for sie */
    static void ClrBits(uint64_t mask) {
      __asm__ volatile("csrrc zero, sie, %0" : : "r"(mask) :);
    }

    /** Atomic Read and then and clear bits for sie */
    static uint32_t ReadClrBits(uint64_t mask) {
      uint64_t value;
      __asm__ volatile("csrrc %0, sie, %1" : "=r"(value) : "r"(mask) :);
      return value;
    }

    // ------------------------------------------
    // Immediate value CSR bit set and clear instructions (only up to 5 bits)

    /** Atomic modify and set bits from immediate for sie */
    static void SetBitsImm(const uint8_t mask) {
      __asm__ volatile("csrrsi zero, sie, %0" : : "i"(mask) :);
    }

    /** Atomic Read and then and set bits from immediate for sie */
    static uint64_t ReadSetBitsImm(const uint8_t mask) {
      uint64_t value;
      __asm__ volatile("csrrsi %0, sie, %1" : "=r"(value) : "i"(mask) :);
      return value;
    }

    /** Atomic modify and clear bits from immediate for sie */
    static void ClrBitsImm(const uint8_t mask) {
      __asm__ volatile("csrrci zero, sie, %0" : : "i"(mask) :);
    }

    /** Atomic Read and then and clear bits from immediate for sie */
    static uint64_t ReadClrBitsImm(const uint8_t mask) {
      uint64_t value;
      __asm__ volatile("csrrci %0, sie, %1" : "=r"(value) : "i"(mask) :);
      return value;
    }
  };

  /** Parameter data for fields in sie */
  struct SieData {
    /** Parameter data for ssi */
    struct ssi {
      using DataType = uint64_t;
      static constexpr uint64_t kBitOffset = 1;
      static constexpr uint64_t kBitWidth = 1;
      static constexpr uint64_t kBitMask = 0x2;
      static constexpr uint64_t kAllSetMask = 0x1;
    };
    /** Parameter data for sti */
    struct sti {
      using DataType = uint64_t;
      static constexpr uint64_t kBitOffset = 5;
      static constexpr uint64_t kBitWidth = 1;
      static constexpr uint64_t kBitMask = 0x20;
      static constexpr uint64_t kAllSetMask = 0x1;
    };
    /** Parameter data for sei */
    struct sei {
      using DataType = uint64_t;
      static constexpr uint64_t kBitOffset = 9;
      static constexpr uint64_t kBitWidth = 1;
      static constexpr uint64_t kBitMask = 0x200;
      static constexpr uint64_t kAllSetMask = 0x1;
    };
    /** Parameter data for usi */
    struct usi {
      using DataType = uint64_t;
      static constexpr uint64_t kBitOffset = 0;
      static constexpr uint64_t kBitWidth = 1;
      static constexpr uint64_t kBitMask = 0x1;
      static constexpr uint64_t kAllSetMask = 0x1;
    };
    /** Parameter data for uti */
    struct uti {
      using DataType = uint64_t;
      static constexpr uint64_t kBitOffset = 4;
      static constexpr uint64_t kBitWidth = 1;
      static constexpr uint64_t kBitMask = 0x10;
      static constexpr uint64_t kAllSetMask = 0x1;
    };
    /** Parameter data for uei */
    struct uei {
      using DataType = uint64_t;
      static constexpr uint64_t kBitOffset = 8;
      static constexpr uint64_t kBitWidth = 1;
      static constexpr uint64_t kBitMask = 0x100;
      static constexpr uint64_t kAllSetMask = 0x1;
    };
  };

  // ----------------------------------------------------------------
  // fflags - URW - Floating-Point Accrued Exceptions.
  //
  /** Floating-Point Accrued Exceptions. assembler operations */
  struct FflagsOps {
    using DataType = uint64_t;
    static constexpr priv_t priv = URW;

    /** Read fflags */
    static uint64_t Read() {
      uint64_t value;
      __asm__ volatile("csrr %0, fflags" : "=r"(value) : :);
      return value;
    }

    /** Write fflags */
    static void Write(uint64_t value) {
      __asm__ volatile("csrw fflags, %0" : : "r"(value) :);
    }

    /** Write immediate value to fflags */
    static void WriteImm(uint64_t value) {
      __asm__ volatile("csrwi fflags, %0" : : "i"(value) :);
    }

    /** Read and then Write to fflags */
    static uint64_t ReadWrite(uint64_t new_value) {
      uint64_t prev_value;
      __asm__ volatile("csrrw %0, fflags, %1"
                       : "=r"(prev_value)
                       : "r"(new_value)
                       :);
      return prev_value;
    }

    /** Read and then Write immediate value to fflags */
    static uint64_t ReadWriteImm(const uint8_t new_value) {
      uint64_t prev_value;
      __asm__ volatile("csrrwi %0, fflags, %1"
                       : "=r"(prev_value)
                       : "i"(new_value)
                       :);
      return prev_value;
    }

    // ------------------------------------------
    // Register CSR bit set and clear instructions

    /** Atomic modify and set bits for fflags */
    static void SetBits(uint64_t mask) {
      __asm__ volatile("csrrs zero, fflags, %0" : : "r"(mask) :);
    }

    /** Atomic Read and then and set bits for fflags */
    static uint32_t ReadSetBits(uint64_t mask) {
      uint64_t value;
      __asm__ volatile("csrrs %0, fflags, %1" : "=r"(value) : "r"(mask) :);
      return value;
    }

    /** Atomic modify and clear bits for fflags */
    static void ClrBits(uint64_t mask) {
      __asm__ volatile("csrrc zero, fflags, %0" : : "r"(mask) :);
    }

    /** Atomic Read and then and clear bits for fflags */
    static uint32_t ReadClrBits(uint64_t mask) {
      uint64_t value;
      __asm__ volatile("csrrc %0, fflags, %1" : "=r"(value) : "r"(mask) :);
      return value;
    }

    // ------------------------------------------
    // Immediate value CSR bit set and clear instructions (only up to 5 bits)

    /** Atomic modify and set bits from immediate for fflags */
    static void SetBitsImm(const uint8_t mask) {
      __asm__ volatile("csrrsi zero, fflags, %0" : : "i"(mask) :);
    }

    /** Atomic Read and then and set bits from immediate for fflags */
    static uint64_t ReadSetBitsImm(const uint8_t mask) {
      uint64_t value;
      __asm__ volatile("csrrsi %0, fflags, %1" : "=r"(value) : "i"(mask) :);
      return value;
    }

    /** Atomic modify and clear bits from immediate for fflags */
    static void ClrBitsImm(const uint8_t mask) {
      __asm__ volatile("csrrci zero, fflags, %0" : : "i"(mask) :);
    }

    /** Atomic Read and then and clear bits from immediate for fflags */
    static uint64_t ReadClrBitsImm(const uint8_t mask) {
      uint64_t value;
      __asm__ volatile("csrrci %0, fflags, %1" : "=r"(value) : "i"(mask) :);
      return value;
    }
  };

  // ----------------------------------------------------------------
  // frm - URW - Floating-Point Dynamic Rounding Mode.
  //
  /** Floating-Point Dynamic Rounding Mode. assembler operations */
  struct FrmOps {
    using DataType = uint64_t;
    static constexpr priv_t priv = URW;

    /** Read frm */
    static uint64_t Read() {
      uint64_t value;
      __asm__ volatile("csrr %0, frm" : "=r"(value) : :);
      return value;
    }

    /** Write frm */
    static void Write(uint64_t value) {
      __asm__ volatile("csrw frm, %0" : : "r"(value) :);
    }

    /** Write immediate value to frm */
    static void WriteImm(uint64_t value) {
      __asm__ volatile("csrwi frm, %0" : : "i"(value) :);
    }

    /** Read and then Write to frm */
    static uint64_t ReadWrite(uint64_t new_value) {
      uint64_t prev_value;
      __asm__ volatile("csrrw %0, frm, %1"
                       : "=r"(prev_value)
                       : "r"(new_value)
                       :);
      return prev_value;
    }

    /** Read and then Write immediate value to frm */
    static uint64_t ReadWriteImm(const uint8_t new_value) {
      uint64_t prev_value;
      __asm__ volatile("csrrwi %0, frm, %1"
                       : "=r"(prev_value)
                       : "i"(new_value)
                       :);
      return prev_value;
    }

    // ------------------------------------------
    // Register CSR bit set and clear instructions

    /** Atomic modify and set bits for frm */
    static void SetBits(uint64_t mask) {
      __asm__ volatile("csrrs zero, frm, %0" : : "r"(mask) :);
    }

    /** Atomic Read and then and set bits for frm */
    static uint32_t ReadSetBits(uint64_t mask) {
      uint64_t value;
      __asm__ volatile("csrrs %0, frm, %1" : "=r"(value) : "r"(mask) :);
      return value;
    }

    /** Atomic modify and clear bits for frm */
    static void ClrBits(uint64_t mask) {
      __asm__ volatile("csrrc zero, frm, %0" : : "r"(mask) :);
    }

    /** Atomic Read and then and clear bits for frm */
    static uint32_t ReadClrBits(uint64_t mask) {
      uint64_t value;
      __asm__ volatile("csrrc %0, frm, %1" : "=r"(value) : "r"(mask) :);
      return value;
    }

    // ------------------------------------------
    // Immediate value CSR bit set and clear instructions (only up to 5 bits)

    /** Atomic modify and set bits from immediate for frm */
    static void SetBitsImm(const uint8_t mask) {
      __asm__ volatile("csrrsi zero, frm, %0" : : "i"(mask) :);
    }

    /** Atomic Read and then and set bits from immediate for frm */
    static uint64_t ReadSetBitsImm(const uint8_t mask) {
      uint64_t value;
      __asm__ volatile("csrrsi %0, frm, %1" : "=r"(value) : "i"(mask) :);
      return value;
    }

    /** Atomic modify and clear bits from immediate for frm */
    static void ClrBitsImm(const uint8_t mask) {
      __asm__ volatile("csrrci zero, frm, %0" : : "i"(mask) :);
    }

    /** Atomic Read and then and clear bits from immediate for frm */
    static uint64_t ReadClrBitsImm(const uint8_t mask) {
      uint64_t value;
      __asm__ volatile("csrrci %0, frm, %1" : "=r"(value) : "i"(mask) :);
      return value;
    }
  };

  // ----------------------------------------------------------------
  // fcsr - URW - Floating-Point Control and Status
  //
  /** Floating-Point Control and Status assembler operations */
  struct FcsrOps {
    using DataType = uint64_t;
    static constexpr priv_t priv = URW;

    /** Read fcsr */
    static uint64_t Read() {
      uint64_t value;
      __asm__ volatile("csrr %0, fcsr" : "=r"(value) : :);
      return value;
    }

    /** Write fcsr */
    static void Write(uint64_t value) {
      __asm__ volatile("csrw fcsr, %0" : : "r"(value) :);
    }

    /** Write immediate value to fcsr */
    static void WriteImm(uint64_t value) {
      __asm__ volatile("csrwi fcsr, %0" : : "i"(value) :);
    }

    /** Read and then Write to fcsr */
    static uint64_t ReadWrite(uint64_t new_value) {
      uint64_t prev_value;
      __asm__ volatile("csrrw %0, fcsr, %1"
                       : "=r"(prev_value)
                       : "r"(new_value)
                       :);
      return prev_value;
    }

    /** Read and then Write immediate value to fcsr */
    static uint64_t ReadWriteImm(const uint8_t new_value) {
      uint64_t prev_value;
      __asm__ volatile("csrrwi %0, fcsr, %1"
                       : "=r"(prev_value)
                       : "i"(new_value)
                       :);
      return prev_value;
    }

    // ------------------------------------------
    // Register CSR bit set and clear instructions

    /** Atomic modify and set bits for fcsr */
    static void SetBits(uint64_t mask) {
      __asm__ volatile("csrrs zero, fcsr, %0" : : "r"(mask) :);
    }

    /** Atomic Read and then and set bits for fcsr */
    static uint32_t ReadSetBits(uint64_t mask) {
      uint64_t value;
      __asm__ volatile("csrrs %0, fcsr, %1" : "=r"(value) : "r"(mask) :);
      return value;
    }

    /** Atomic modify and clear bits for fcsr */
    static void ClrBits(uint64_t mask) {
      __asm__ volatile("csrrc zero, fcsr, %0" : : "r"(mask) :);
    }

    /** Atomic Read and then and clear bits for fcsr */
    static uint32_t ReadClrBits(uint64_t mask) {
      uint64_t value;
      __asm__ volatile("csrrc %0, fcsr, %1" : "=r"(value) : "r"(mask) :);
      return value;
    }

    // ------------------------------------------
    // Immediate value CSR bit set and clear instructions (only up to 5 bits)

    /** Atomic modify and set bits from immediate for fcsr */
    static void SetBitsImm(const uint8_t mask) {
      __asm__ volatile("csrrsi zero, fcsr, %0" : : "i"(mask) :);
    }

    /** Atomic Read and then and set bits from immediate for fcsr */
    static uint64_t ReadSetBitsImm(const uint8_t mask) {
      uint64_t value;
      __asm__ volatile("csrrsi %0, fcsr, %1" : "=r"(value) : "i"(mask) :);
      return value;
    }

    /** Atomic modify and clear bits from immediate for fcsr */
    static void ClrBitsImm(const uint8_t mask) {
      __asm__ volatile("csrrci zero, fcsr, %0" : : "i"(mask) :);
    }

    /** Atomic Read and then and clear bits from immediate for fcsr */
    static uint64_t ReadClrBitsImm(const uint8_t mask) {
      uint64_t value;
      __asm__ volatile("csrrci %0, fcsr, %1" : "=r"(value) : "i"(mask) :);
      return value;
    }
  };

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

//   /* Supervisor Mode Scratch Register */
//   template <class RegOps>
//   class sscratch_reg : public ReadWriteReg<RegOps> {};
//   using sscratch = sscratch_reg<SscratchOps>;
//   /* Supervisor Exception Program Counter */
//   template <class RegOps>
//   class sepc_reg : public ReadWriteReg<RegOps> {};
//   using sepc = sepc_reg<SepcOps>;
//   /* Supervisor Exception Cause */
//   template <class RegOps>
//   class scause_reg : public ReadWriteReg<RegOps> {
//    public:
//     ReadWriteField<RegOps, ScauseData::interrupt> interrupt;
//     ReadWriteField<RegOps, ScauseData::exception_code> exception_code;
//   };
//   using scause = scause_reg<ScauseOps>;
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
