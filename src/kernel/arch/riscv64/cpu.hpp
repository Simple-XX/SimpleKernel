
/**
 * @file cpu.hpp
 * @brief riscv64 cpu 相关定义
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

#ifndef SIMPLEKERNEL_SRC_KERNEL_ARCH_RISCV64_CPU_HPP_
#define SIMPLEKERNEL_SRC_KERNEL_ARCH_RISCV64_CPU_HPP_

#include <cstdint>
#include <cstdlib>

#include "iostream"
#include "stdio.h"

// ------------------------------------------------------------------------
// Base and common classes

namespace riscv {
namespace csr {

/** Immediate instructions use a 5 bit immediate field */
static constexpr uint64_t kCsrImmOpMask = 0x01F;

/** CSR: Read only, and read-Write base class */
template <class C>
class ReadOnlyReg {
 public:
  using ReadDataType = typename C::DataType;

  ReadOnlyReg(void) {}
  ReadOnlyReg(const ReadOnlyReg &) = delete;
  ReadOnlyReg &operator=(const ReadOnlyReg &) = delete;

  /** Read the CSR value */
  static inline ReadDataType read(void) { return C::read(); }
  /** Operator alias to read the CSR value */
  inline ReadDataType operator()(void) { return C::read(); }
};

/** CSR: Write only, and read-Write base class */
template <class C>
class WriteOnlyReg {
 public:
  using WriteDataType = typename C::DataType;

  WriteOnlyReg(void) {}
  WriteOnlyReg(const WriteOnlyReg &) = delete;
  WriteOnlyReg &operator=(const WriteOnlyReg &) = delete;

  /** Write a constant to the CSR. */
  template <WriteDataType VALUE>
  void write_const(void) {
    if constexpr ((VALUE & kCsrImmOpMask) == VALUE) {
      C::WriteImm(VALUE);
    } else {
      C::Write(VALUE);
    }
  }
  /** Write to the CSR. */
  inline void Write(const WriteDataType value) { C::Write(value); }
  /** Set a constant mask of bits in the CSR. */
  template <WriteDataType MASK>
  void set_const(void) {
    if constexpr ((MASK & kCsrImmOpMask) == MASK) {
      C::SetBitsImm(MASK);
    } else {
      C::SetBits(MASK);
    }
  }
  /** Set a mask of bits in the CSR. */
  inline void set(WriteDataType mask) { C::SetBits(mask); }
  /** Clear a constant mask of bits in the CSR. */
  template <WriteDataType MASK>
  void ClrConst(void) {
    if constexpr ((MASK & kCsrImmOpMask) == MASK) {
      C::ClrBitsImm(MASK);
    } else {
      C::ClrBits(MASK);
    }
  }
  /** Clear a mask of bits in the CSR. */
  inline void Clr(WriteDataType mask) { C::ClrBits(mask); }
  /** Operator alias to set mask of bits in the CSR. */
  inline void operator|=(WriteDataType mask) { C::SetBits(mask); }
};

/** CSR: Read-Write base class */
template <class C>
class ReadWriteReg : public ReadOnlyReg<C>, public WriteOnlyReg<C> {
 public:
  using datatype_t = typename C::DataType;

  ReadWriteReg(void) : ReadOnlyReg<C>(), WriteOnlyReg<C>() {}
  ReadWriteReg(const ReadWriteReg &) = delete;
  ReadWriteReg &operator=(const ReadWriteReg &) = delete;

  /** Read from, then Write a constant value to the CSR. */
  template <datatype_t VALUE>
  datatype_t ReadWriteConst(void) {
    if constexpr ((VALUE & kCsrImmOpMask) == VALUE) {
      return C::ReadWriteImm(VALUE);
    } else {
      return C::ReadWrite(VALUE);
    }
  }
  /** Read from, then Write to the CSR. */
  inline datatype_t ReadWrite(const datatype_t value) {
    return C::ReadWrite(value);
  }
  /** Read from, then set a constant bit mask to the CSR. */
  template <datatype_t MASK>
  datatype_t read_set_bits_const(void) {
    if constexpr ((MASK & kCsrImmOpMask) == MASK) {
      return C::ReadSetBitsImm(MASK);
    } else {
      return C::ReadSetBits(MASK);
    }
  }
  /** Read from, then set a bit mask to the CSR. */
  inline datatype_t ReadSetBits(const datatype_t mask) {
    return C::ReadSetBits(mask);
  }
  /** Read from, then clear a constant bit mask to the CSR. */
  template <datatype_t MASK>
  datatype_t read_clr_bits_const(void) {
    if constexpr ((MASK & kCsrImmOpMask) == MASK) {
      return C::ReadClrBitsImm(MASK);
    } else {
      return C::ReadClrBits(MASK);
    }
  }
  /** Read from, then clear a bit mask to the CSR. */
  inline datatype_t ReadClrBits(const datatype_t mask) {
    return C::ReadClrBits(mask);
  }
};

/** CSR Field: Read only, and read-Write base class */
template <class C, class F>
class ReadOnlyField {
 public:
  using ReadDataType = typename F::DataType;

  ReadOnlyField(void) {}
  ReadOnlyField(const ReadOnlyField &) = delete;
  ReadOnlyField &operator=(const ReadOnlyField &) = delete;

  /** Read a given field value from a CSR */
  ReadDataType Read(void) {
    return (ReadDataType)((C::Read() & F::BIT_MASK) >> F::BIT_OFFSET);
  }
};

/** CSR Field: Write only, and Read-Write base class */
template <class C, class F>
class WriteOnlyField {
 public:
  using WriteDataType = typename F::DataType;
  using reg_write_datatype_t = typename C::DataType;

  WriteOnlyField(void) {}
  WriteOnlyField(const WriteOnlyField &) = delete;
  WriteOnlyField &operator=(const WriteOnlyField &) = delete;

  inline void set(void) {
    if constexpr ((F::BIT_MASK & kCsrImmOpMask) == F::BIT_MASK) {
      C::SetBitsImm(F::BIT_MASK);
    } else {
      C::SetBits(F::BIT_MASK);
    }
  }
  inline void Clr(void) {
    if constexpr ((F::BIT_MASK & kCsrImmOpMask) == F::BIT_MASK) {
      C::ClrBitsImm(F::BIT_MASK);
    } else {
      C::ClrBits(F::BIT_MASK);
    }
  }
};

/** CSR Field: Read-Write base class */
template <class C, class F>
class ReadWriteField : public ReadOnlyField<C, F>, public WriteOnlyField<C, F> {
 public:
  using datatype_t = typename F::DataType;
  using reg_datatype_t = typename C::DataType;

  ReadWriteField(void) : ReadOnlyField<C, F>(), WriteOnlyField<C, F>() {}
  ReadWriteField(const ReadWriteField &) = delete;
  ReadWriteField &operator=(const ReadWriteField &) = delete;

  /* Read-modify-Write to Write a field.
     NOTE - not atomic.
   */
  inline void Write(const datatype_t value) {
    auto org_value = C::Read();
    auto new_value = (org_value & ~F::BIT_MASK) |
                     (((reg_datatype_t)value << F::BIT_OFFSET) & F::BIT_MASK);
    C::Write(new_value);
  }
  /* Read-modify-Write to set a field, and return original value.
     NOTE - not atomic.
   */
  inline datatype_t ReadWrite(const datatype_t value) {
    auto org_value = C::Read();
    auto new_value = (org_value & ~F::BIT_MASK) |
                     (((reg_datatype_t)value << F::BIT_OFFSET) & F::BIT_MASK);
    C::Write(new_value);
    return (datatype_t)((org_value & F::BIT_MASK) >> F::BIT_OFFSET);
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

}  // namespace csr
}  // namespace riscv

// ------------------------------------------------------------------------
// Assembler operations and bit field definitions

namespace riscv {
namespace csr {
// ----------------------------------------------------------------
// sscratch - SRW - Supervisor Mode Scratch Register
//
/** Supervisor Mode Scratch Register assembler operations */
struct sscratch_ops {
  using DataType = uint64_t;
  static constexpr priv_t priv = SRW;

  /** Read sscratch */
  static uint64_t Read(void) {
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

}; /* sscratch_ops */

// ----------------------------------------------------------------
// sepc - SRW - Supervisor Exception Program Counter
//
/** Supervisor Exception Program Counter assembler operations */
struct sepc_ops {
  using DataType = uint64_t;
  static constexpr priv_t priv = SRW;

  /** Read sepc */
  static uint64_t Read(void) {
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

}; /* sepc_ops */

// ----------------------------------------------------------------
// scause - SRW - Supervisor Exception Cause
//
/** Supervisor Exception Cause assembler operations */
struct scause_ops {
  using DataType = uint64_t;
  static constexpr priv_t priv = SRW;

  /** Read scause */
  static uint64_t Read(void) {
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

}; /* scause_ops */
/** Parameter data for fields in scause */
namespace scause_data {
/** Parameter data for interrupt */
struct interrupt {
  using DataType = uint64_t;
  static constexpr uint64_t BIT_OFFSET = (64 - 1);
  static constexpr uint64_t BIT_WIDTH = 1;
  static constexpr uint64_t BIT_MASK = (0x1UL << ((64 - 1)));
  static constexpr uint64_t ALL_SET_MASK = 0x1;
};
/** Parameter data for exception_code */
struct exception_code {
  using DataType = uint64_t;
  static constexpr uint64_t BIT_OFFSET = 0;
  static constexpr uint64_t BIT_WIDTH = ((64 - 2) - (0) + 1);
  static constexpr uint64_t BIT_MASK =
      ((1UL << (((64 - 2) - (0) + 1) - 1)) << (0));
  static constexpr uint64_t ALL_SET_MASK =
      ((1UL << (((64 - 2) - (0) + 1) - 1)) << (0));
};
}  // namespace scause_data

// ----------------------------------------------------------------
// sstatus - SRW - Supervisor Status
//
/** Supervisor Status assembler operations */
struct sstatus_ops {
  using DataType = uint64_t;
  static constexpr priv_t priv = SRW;

  /** Read sstatus */
  static uint64_t Read(void) {
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

}; /* sstatus_ops */
/** Parameter data for fields in sstatus */
namespace sstatus_data {
/** Parameter data for sie */
struct sie {
  using DataType = uint64_t;
  static constexpr uint64_t BIT_OFFSET = 2;
  static constexpr uint64_t BIT_WIDTH = 1;
  static constexpr uint64_t BIT_MASK = 0x4;
  static constexpr uint64_t ALL_SET_MASK = 0x1;
};
/** Parameter data for spie */
struct spie {
  using DataType = uint64_t;
  static constexpr uint64_t BIT_OFFSET = 5;
  static constexpr uint64_t BIT_WIDTH = 1;
  static constexpr uint64_t BIT_MASK = 0x20;
  static constexpr uint64_t ALL_SET_MASK = 0x1;
};
/** Parameter data for spp */
struct spp {
  using DataType = uint64_t;
  static constexpr uint64_t BIT_OFFSET = 8;
  static constexpr uint64_t BIT_WIDTH = 1;
  static constexpr uint64_t BIT_MASK = 0x100;
  static constexpr uint64_t ALL_SET_MASK = 0x1;
};
}  // namespace sstatus_data

// ----------------------------------------------------------------
// stvec - SRW - Supervisor Trap Vector Base Address
//
/** Supervisor Trap Vector Base Address assembler operations */
struct stvec_ops {
  using DataType = uint64_t;
  static constexpr priv_t priv = SRW;

  /** Read stvec */
  static uint64_t Read(void) {
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

}; /* stvec_ops */
/** Parameter data for fields in stvec */
namespace stvec_data {
/** Parameter data for base */
struct base {
  using DataType = uint64_t;
  static constexpr uint64_t BIT_OFFSET = 2;
  static constexpr uint64_t BIT_WIDTH = ((64 - 1) - (2) + 1);
  static constexpr uint64_t BIT_MASK =
      ((1UL << (((64 - 1) - (2) + 1) - 1)) << (2));
  static constexpr uint64_t ALL_SET_MASK =
      ((1UL << (((64 - 1) - (2) + 1) - 1)) << (0));
};
/** Parameter data for mode */
struct mode {
  using DataType = uint64_t;
  static constexpr uint64_t BIT_OFFSET = 0;
  static constexpr uint64_t BIT_WIDTH = 2;
  static constexpr uint64_t BIT_MASK = 0x3;
  static constexpr uint64_t ALL_SET_MASK = 0x3;
};
}  // namespace stvec_data

// ----------------------------------------------------------------
// sideleg - SRW - Supervisor Interrupt Delegation
//
/** Supervisor Interrupt Delegation assembler operations */
struct sideleg_ops {
  using DataType = uint64_t;
  static constexpr priv_t priv = SRW;

  /** Read sideleg */
  static uint64_t Read(void) {
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

}; /* sideleg_ops */

// ----------------------------------------------------------------
// sedeleg - SRW - Supervisor Exception Delegation
//
/** Supervisor Exception Delegation assembler operations */
struct sedeleg_ops {
  using DataType = uint64_t;
  static constexpr priv_t priv = SRW;

  /** Read sedeleg */
  static uint64_t Read(void) {
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

}; /* sedeleg_ops */

// ----------------------------------------------------------------
// sip - SRW - Supervisor Interrupt Pending
//
/** Supervisor Interrupt Pending assembler operations */
struct sip_ops {
  using DataType = uint64_t;
  static constexpr priv_t priv = SRW;

  /** Read sip */
  static uint64_t Read(void) {
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
    __asm__ volatile("csrrw %0, sip, %1" : "=r"(prev_value) : "r"(new_value) :);
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

}; /* sip_ops */
/** Parameter data for fields in sip */
namespace sip_data {
/** Parameter data for ssi */
struct ssi {
  using DataType = uint64_t;
  static constexpr uint64_t BIT_OFFSET = 1;
  static constexpr uint64_t BIT_WIDTH = 1;
  static constexpr uint64_t BIT_MASK = 0x2;
  static constexpr uint64_t ALL_SET_MASK = 0x1;
};
/** Parameter data for sti */
struct sti {
  using DataType = uint64_t;
  static constexpr uint64_t BIT_OFFSET = 5;
  static constexpr uint64_t BIT_WIDTH = 1;
  static constexpr uint64_t BIT_MASK = 0x20;
  static constexpr uint64_t ALL_SET_MASK = 0x1;
};
/** Parameter data for sei */
struct sei {
  using DataType = uint64_t;
  static constexpr uint64_t BIT_OFFSET = 9;
  static constexpr uint64_t BIT_WIDTH = 1;
  static constexpr uint64_t BIT_MASK = 0x200;
  static constexpr uint64_t ALL_SET_MASK = 0x1;
};
/** Parameter data for usi */
struct usi {
  using DataType = uint64_t;
  static constexpr uint64_t BIT_OFFSET = 0;
  static constexpr uint64_t BIT_WIDTH = 1;
  static constexpr uint64_t BIT_MASK = 0x1;
  static constexpr uint64_t ALL_SET_MASK = 0x1;
};
/** Parameter data for uti */
struct uti {
  using DataType = uint64_t;
  static constexpr uint64_t BIT_OFFSET = 4;
  static constexpr uint64_t BIT_WIDTH = 1;
  static constexpr uint64_t BIT_MASK = 0x10;
  static constexpr uint64_t ALL_SET_MASK = 0x1;
};
/** Parameter data for uei */
struct uei {
  using DataType = uint64_t;
  static constexpr uint64_t BIT_OFFSET = 8;
  static constexpr uint64_t BIT_WIDTH = 1;
  static constexpr uint64_t BIT_MASK = 0x100;
  static constexpr uint64_t ALL_SET_MASK = 0x1;
};
}  // namespace sip_data

// ----------------------------------------------------------------
// sie - SRW - Supervisor Interrupt Enable
//
/** Supervisor Interrupt Enable assembler operations */
struct sie_ops {
  using DataType = uint64_t;
  static constexpr priv_t priv = SRW;

  /** Read sie */
  static uint64_t Read(void) {
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
    __asm__ volatile("csrrw %0, sie, %1" : "=r"(prev_value) : "r"(new_value) :);
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

}; /* sie_ops */
/** Parameter data for fields in sie */
namespace sie_data {
/** Parameter data for ssi */
struct ssi {
  using DataType = uint64_t;
  static constexpr uint64_t BIT_OFFSET = 1;
  static constexpr uint64_t BIT_WIDTH = 1;
  static constexpr uint64_t BIT_MASK = 0x2;
  static constexpr uint64_t ALL_SET_MASK = 0x1;
};
/** Parameter data for sti */
struct sti {
  using DataType = uint64_t;
  static constexpr uint64_t BIT_OFFSET = 5;
  static constexpr uint64_t BIT_WIDTH = 1;
  static constexpr uint64_t BIT_MASK = 0x20;
  static constexpr uint64_t ALL_SET_MASK = 0x1;
};
/** Parameter data for sei */
struct sei {
  using DataType = uint64_t;
  static constexpr uint64_t BIT_OFFSET = 9;
  static constexpr uint64_t BIT_WIDTH = 1;
  static constexpr uint64_t BIT_MASK = 0x200;
  static constexpr uint64_t ALL_SET_MASK = 0x1;
};
/** Parameter data for usi */
struct usi {
  using DataType = uint64_t;
  static constexpr uint64_t BIT_OFFSET = 0;
  static constexpr uint64_t BIT_WIDTH = 1;
  static constexpr uint64_t BIT_MASK = 0x1;
  static constexpr uint64_t ALL_SET_MASK = 0x1;
};
/** Parameter data for uti */
struct uti {
  using DataType = uint64_t;
  static constexpr uint64_t BIT_OFFSET = 4;
  static constexpr uint64_t BIT_WIDTH = 1;
  static constexpr uint64_t BIT_MASK = 0x10;
  static constexpr uint64_t ALL_SET_MASK = 0x1;
};
/** Parameter data for uei */
struct uei {
  using DataType = uint64_t;
  static constexpr uint64_t BIT_OFFSET = 8;
  static constexpr uint64_t BIT_WIDTH = 1;
  static constexpr uint64_t BIT_MASK = 0x100;
  static constexpr uint64_t ALL_SET_MASK = 0x1;
};
}  // namespace sie_data

// ----------------------------------------------------------------
// fflags - URW - Floating-Point Accrued Exceptions.
//
/** Floating-Point Accrued Exceptions. assembler operations */
struct fflags_ops {
  using DataType = uint64_t;
  static constexpr priv_t priv = URW;

  /** Read fflags */
  static uint64_t Read(void) {
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

}; /* fflags_ops */

// ----------------------------------------------------------------
// frm - URW - Floating-Point Dynamic Rounding Mode.
//
/** Floating-Point Dynamic Rounding Mode. assembler operations */
struct frm_ops {
  using DataType = uint64_t;
  static constexpr priv_t priv = URW;

  /** Read frm */
  static uint64_t Read(void) {
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
    __asm__ volatile("csrrw %0, frm, %1" : "=r"(prev_value) : "r"(new_value) :);
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

}; /* frm_ops */

// ----------------------------------------------------------------
// fcsr - URW - Floating-Point Control and Status
//
/** Floating-Point Control and Status assembler operations */
struct fcsr_ops {
  using DataType = uint64_t;
  static constexpr priv_t priv = URW;

  /** Read fcsr */
  static uint64_t Read(void) {
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

}; /* fcsr_ops */

// ----------------------------------------------------------------
// cycle - URO - Cycle counter for RDCYCLE instruction.
//
/** Cycle counter for RDCYCLE instruction. assembler operations */
struct cycle_ops {
  using DataType = uint64_t;
  static constexpr priv_t priv = URO;

  /** Read cycle */
  static uint64_t Read(void) {
    uint64_t value;
    __asm__ volatile("csrr %0, cycle" : "=r"(value) : :);
    return value;
  }

}; /* cycle_ops */

// ----------------------------------------------------------------
// time - URO - Timer for RDTIME instruction.
//
/** Timer for RDTIME instruction. assembler operations */
struct time_ops {
  using DataType = uint64_t;
  static constexpr priv_t priv = URO;

  /** Read time */
  static uint64_t Read(void) {
    uint64_t value;
    __asm__ volatile("csrr %0, time" : "=r"(value) : :);
    return value;
  }

}; /* time_ops */

// ----------------------------------------------------------------
// instret - URO - Instructions-retired counter for RDINSTRET instruction.
//
/** Instructions-retired counter for RDINSTRET instruction. assembler operations
 */
struct instret_ops {
  using DataType = uint64_t;
  static constexpr priv_t priv = URO;

  /** Read instret */
  static uint64_t Read(void) {
    uint64_t value;
    __asm__ volatile("csrr %0, instret" : "=r"(value) : :);
    return value;
  }

}; /* instret_ops */

// ----------------------------------------------------------------
// stval - SRW - Supervisor bad address or instruction.
//
/** Supervisor bad address or instruction. assembler operations */
struct stval_ops {
  using DataType = uint64_t;
  static constexpr priv_t priv = SRW;

  /** Read stval */
  static uint64_t Read(void) {
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

}; /* stval_ops */

// ----------------------------------------------------------------
// satp - SRW - Supervisor address translation and protection.
//
/** Supervisor address translation and protection. assembler operations */
struct satp_ops {
  using DataType = uint64_t;
  static constexpr priv_t priv = SRW;

  /** Read satp */
  static uint64_t Read(void) {
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

}; /* satp_ops */
}  // namespace csr
}  // namespace riscv

// ------------------------------------------------------------------------
// Register and field interface classes.
namespace riscv {
namespace csr {
/* Supervisor Mode Scratch Register */
template <class OPS>
class sscratch_reg : public ReadWriteReg<OPS> {};
using sscratch = sscratch_reg<riscv::csr::sscratch_ops>;
/* Supervisor Exception Program Counter */
template <class OPS>
class sepc_reg : public ReadWriteReg<OPS> {};
using sepc = sepc_reg<riscv::csr::sepc_ops>;
/* Supervisor Exception Cause */
template <class OPS>
class scause_reg : public ReadWriteReg<OPS> {
 public:
  ReadWriteField<OPS, riscv::csr::scause_data::interrupt> interrupt;
  ReadWriteField<OPS, riscv::csr::scause_data::exception_code> exception_code;
};
using scause = scause_reg<riscv::csr::scause_ops>;
/* Supervisor Status */
template <class OPS>
class sstatus_reg : public ReadWriteReg<OPS> {
 public:
  ReadWriteField<OPS, riscv::csr::sstatus_data::sie> sie;
  ReadWriteField<OPS, riscv::csr::sstatus_data::spie> spie;
  ReadWriteField<OPS, riscv::csr::sstatus_data::spp> spp;
};
using sstatus = sstatus_reg<riscv::csr::sstatus_ops>;
/* Supervisor Trap Vector Base Address */
template <class OPS>
class stvec_reg : public ReadWriteReg<OPS> {
 public:
  ReadWriteField<OPS, riscv::csr::stvec_data::base> base;
  ReadWriteField<OPS, riscv::csr::stvec_data::mode> mode;
};
using stvec = stvec_reg<riscv::csr::stvec_ops>;
/* Supervisor Interrupt Delegation */
template <class OPS>
class sideleg_reg : public ReadWriteReg<OPS> {};
using sideleg = sideleg_reg<riscv::csr::sideleg_ops>;
/* Supervisor Exception Delegation */
template <class OPS>
class sedeleg_reg : public ReadWriteReg<OPS> {};
using sedeleg = sedeleg_reg<riscv::csr::sedeleg_ops>;
/* Supervisor Interrupt Pending */
template <class OPS>
class sip_reg : public ReadWriteReg<OPS> {
 public:
  ReadWriteField<OPS, riscv::csr::sip_data::ssi> ssi;
  ReadWriteField<OPS, riscv::csr::sip_data::sti> sti;
  ReadWriteField<OPS, riscv::csr::sip_data::sei> sei;
  ReadWriteField<OPS, riscv::csr::sip_data::usi> usi;
  ReadWriteField<OPS, riscv::csr::sip_data::uti> uti;
  ReadWriteField<OPS, riscv::csr::sip_data::uei> uei;
};
using sip = sip_reg<riscv::csr::sip_ops>;
/* Supervisor Interrupt Enable */
template <class OPS>
class sie_reg : public ReadWriteReg<OPS> {
 public:
  ReadWriteField<OPS, riscv::csr::sie_data::ssi> ssi;
  ReadWriteField<OPS, riscv::csr::sie_data::sti> sti;
  ReadWriteField<OPS, riscv::csr::sie_data::sei> sei;
  ReadWriteField<OPS, riscv::csr::sie_data::usi> usi;
  ReadWriteField<OPS, riscv::csr::sie_data::uti> uti;
  ReadWriteField<OPS, riscv::csr::sie_data::uei> uei;
};
using sie = sie_reg<riscv::csr::sie_ops>;

/* Supervisor bad address or instruction. */
template <class OPS>
class stval_reg : public ReadWriteReg<OPS> {};
using stval = stval_reg<riscv::csr::stval_ops>;
/* Supervisor address translation and protection. */
template <class OPS>
class satp_reg : public ReadWriteReg<OPS> {};
using satp = satp_reg<riscv::csr::satp_ops>;

/** Encapsulate all CSRs in a single structure.
   - No storage is required by this class.
 */
struct all {
  /* Supervisor Mode Scratch Register */
  riscv::csr::sscratch sscratch;
  /* Supervisor Exception Program Counter */
  riscv::csr::sepc sepc;
  /* Supervisor Exception Cause */
  riscv::csr::scause scause;
  /* Supervisor Status */
  riscv::csr::sstatus sstatus;
  /* Supervisor Trap Vector Base Address */
  riscv::csr::stvec stvec;
  /* Supervisor Interrupt Delegation */
  riscv::csr::sideleg sideleg;
  /* Supervisor Exception Delegation */
  riscv::csr::sedeleg sedeleg;
  /* Supervisor Interrupt Pending */
  riscv::csr::sip sip;
  /* Supervisor Interrupt Enable */
  riscv::csr::sie sie;
  /* Supervisor bad address or instruction. */
  riscv::csr::stval stval;
  /* Supervisor address translation and protection. */
  riscv::csr::satp satp;
};

}  // namespace csr

static csr::all csrs;

}  // namespace riscv

/**
 * riscv64 cpu 相关定义
 * @see priv-isa-asciidoc_20240411.pdf
 * https://github.com/riscv/riscv-isa-manual/releases/tag/20240411/priv-isa-asciidoc.pdf
 * @see riscv-abi.pdf
 * https://github.com/riscv-non-isa/riscv-elf-psabi-doc/releases/tag/v1.0
 */
class Cpu {
 public:
  /**
   * Privilege Levels
   * @see priv-isa-asciidoc_20240411.pdf#1.2
   */
  enum {
    kPrivilegeLevelUser = 0,
    kPrivilegeLevelSuper = 1,
    kPrivilegeLevelMachine = 3,
  };

  /**
   * @brief Xstatus 寄存器定义
   * @see priv-isa-asciidoc_20240411.pdf#3.1.6
   */
  class Xstatus {
   public:
    /// User Interrupt Enable
    static constexpr const uint64_t kXstatusUie = 1 << 0;
    /// Supervisor Interrupt Enable
    static constexpr const uint64_t kXstatusSie = 1 << 1;
    /// User Previous Interrupt Enable
    static constexpr const uint64_t kXstatusUpie = 1 << 4;
    /// Supervisor Previous Interrupt Enable
    static constexpr const uint64_t kXstatusSpie = 1 << 5;
    /// Previous mode, 1=Supervisor, 0=User
    static constexpr const uint64_t kXstatusSpp = 1 << 8;

    union {
      struct {
        // Reserved Writes Preserve Values, Reads Ignore Values 1
        uint64_t wpri1 : 1;
        // The SIE bit enables or disables all interrupts in supervisor mode.
        uint64_t sie : 1;
        uint64_t wpri2 : 1;
        // The MIE bit enables or disables all interrupts in machine mode.
        uint64_t mie : 1;
        uint64_t wpri3 : 1;
        // The SPIE bit indicates whether supervisor interrupts were enabled
        // prior to trapping into supervisor mode.
        uint64_t spie : 1;
        // The MBE, SBE, and UBE bits in mstatus and mstatush are WARL fields
        // that control the endianness of memory accesses other than instruction
        // fetches. Instruction fetches are always little-endian.
        uint64_t ube : 1;
        uint64_t mpie : 1;
        // The SPP bit indicates the privilege level at which a hart was
        // executing before entering supervisor mode.
        // xPP holds the previous privilege mode.
        uint64_t spp : 1;
        uint64_t vs : 2;
        uint64_t mpp : 2;
        // The FS field encodes the status of the floating-point unit, including
        // the CSR fcsr and floating-point data registers f0–f31.
        uint64_t fs : 2;
        // The XS field encodes the status of additional user-mode extensions
        // and associated state.
        uint64_t xs : 2;
        // Modify PRiVilege
        uint64_t mprv : 1;
        // permit Supervisor User Memory access
        uint64_t sum : 1;
        // Make eXecutable Readable
        uint64_t mxr : 1;
        // Trap Virtual Memory
        uint64_t tvm : 1;
        // Timeout Wait
        uint64_t tw : 1;
        // Trap SRET
        uint64_t tsr : 1;
        uint64_t wpri4 : 9;
        // U-mode XLEN
        uint64_t uxl : 2;
        // S-mode XLEN
        uint64_t sxl : 2;
        uint64_t sbe : 1;
        uint64_t mbe : 1;
        uint64_t wpri5 : 25;
        // The SD bit is read-only and is set when either the FS or XS bits
        // encode a Dirty state (i.e., SD=((FS==11) OR (XS==11))).
        uint64_t sd : 1;
      } xstatus_;
      uint64_t val_;
    };

    explicit Xstatus(uint64_t val) : val_(val) {}

    /// @name 构造/析构函数
    /// @{
    Xstatus() = default;
    Xstatus(const Xstatus &) = default;
    Xstatus(Xstatus &&) = default;
    auto operator=(const Xstatus &) -> Xstatus & = default;
    auto operator=(Xstatus &&) -> Xstatus & = default;
    ~Xstatus() = default;
    /// @}

    friend std::ostream &operator<<(std::ostream &os, const Xstatus &xstatus) {
      printf("val: 0x%p, sie: %s, spie: %s, spp: %s", xstatus.val_,
             (xstatus.xstatus_.sie == true ? "Enable" : "Disable"),
             (xstatus.xstatus_.spie == true ? "Enable" : "Disable"),
             (xstatus.xstatus_.spp == true ? "S Mode" : "U Mode")

      );
      return os;
    }
  };

  /**
   * @brief Xtvec 寄存器定义
   * @see priv-isa-asciidoc_20240411.pdf#3.1.7
   */
  class Xtvec {
   public:
    /// 中断模式 直接
    static constexpr const uint64_t kDirect = 0x0;
    /// 中断模式 向量
    static constexpr const uint64_t kVectored = 0x1;

    union {
      struct {
        uint64_t mode : 2;
        uint64_t base : 62;
      } xtvec_;
      uint64_t val_;
    };

    explicit Xtvec(uint64_t val) : val_(val) {}

    /// @name 构造/析构函数
    /// @{
    Xtvec() = default;
    Xtvec(const Xtvec &) = default;
    Xtvec(Xtvec &&) = default;
    auto operator=(const Xtvec &) -> Xtvec & = default;
    auto operator=(Xtvec &&) -> Xtvec & = default;
    ~Xtvec() = default;
    /// @}

    friend std::ostream &operator<<(std::ostream &os, const Xtvec &xtvec) {
      printf("val: 0x%p, mode: %s, base: 0x%p", xtvec.val_,
             (xtvec.xtvec_.mode == kDirect ? "Direct" : "Vectored"),
             xtvec.xtvec_.base);
      return os;
    }
  };

  /**
   * @brief Xip 寄存器定义
   * @see priv-isa-asciidoc_20240411.pdf#3.1.9
   */
  class Xip {
   public:
    // Supervisor Interrupt Pending
    /// software
    static constexpr const uint64_t kSsip = 1 << 1;
    /// timer
    static constexpr const uint64_t kStip = 1 << 5;
    /// external
    static constexpr const uint64_t kSeip = 1 << 9;

    union {
      struct {
        uint64_t zero0 : 1;
        /// Supervisor Software Interrupt Pending
        uint64_t ssip : 1;
        uint64_t zero2 : 1;
        uint64_t msip : 1;
        uint64_t zero4 : 1;
        /// Supervisor Timer Interrupt Pending
        uint64_t stip : 1;
        uint64_t zero6 : 1;
        /// Machine Timer Interrupt Pending
        uint64_t mtip : 1;
        /// User External Interrupt Pending
        uint64_t zero8 : 1;
        /// Supervisor External Interrupt Pending
        uint64_t seip : 1;
        uint64_t zero10 : 1;
        /// Machine External Interrupt Pending
        uint64_t meip : 1;
        uint64_t zero12 : 1;
        uint64_t lcofip : 1;
        uint64_t zero14_15 : 1;
        uint64_t warl : 48;
      } xip_;
      uint64_t val_;
    };

    explicit Xip(uint64_t val) : val_(val) {}

    /// @name 构造/析构函数
    /// @{
    Xip() = default;
    Xip(const Xip &) = default;
    Xip(Xip &&) = default;
    auto operator=(const Xip &) -> Xip & = default;
    auto operator=(Xip &&) -> Xip & = default;
    ~Xip() = default;
    /// @}

    friend std::ostream &operator<<(std::ostream &os, const Xip &xip) {
      printf("val: 0x%p, ssie: %s, stie: %s, seie: %s", xip.val_,
             (xip.xip_.ssip == true ? "Enable" : "Disable"),
             (xip.xip_.stip == true ? "Enable" : "Disable"),
             (xip.xip_.seip == true ? "Enable" : "Disable"));
      return os;
    }
  };

  /**
   * @brief Xie 寄存器定义
   * @see priv-isa-asciidoc_20240411.pdf#3.1.9
   */
  class Xie {
   public:
    // Supervisor Interrupt Enable
    /// software
    static constexpr const uint64_t kSsie = 1 << 1;
    /// timer
    static constexpr const uint64_t kStie = 1 << 5;
    /// external
    static constexpr const uint64_t kSeie = 1 << 9;

    union {
      struct {
        uint64_t zero0 : 1;
        /// Supervisor Software Interrupt Enable
        uint64_t ssie : 1;
        uint64_t zero2 : 1;
        uint64_t msie : 1;
        uint64_t zero4 : 1;
        /// Supervisor Timer Interrupt Enable
        uint64_t stie : 1;
        uint64_t zero6 : 1;
        /// Machine Timer Interrupt Enable
        uint64_t mtie : 1;
        uint64_t zero8 : 1;
        /// Supervisor External Interrupt Enable
        uint64_t seie : 1;
        uint64_t zero10 : 1;
        /// Machine External Interrupt Enable
        uint64_t meie : 1;
        uint64_t zero12 : 1;
        uint64_t lcofie : 1;
        uint64_t zero14_15 : 1;
        /// warl
        uint64_t wpri4 : 48;
      } xie_;
      uint64_t val_;
    };

    explicit Xie(uint64_t val) : val_(val) {}

    /// @name 构造/析构函数
    /// @{
    Xie() = default;
    Xie(const Xie &) = default;
    Xie(Xie &&) = default;
    auto operator=(const Xie &) -> Xie & = default;
    auto operator=(Xie &&) -> Xie & = default;
    ~Xie() = default;
    /// @}

    friend std::ostream &operator<<(std::ostream &os, const Xie &xie) {
      printf("val: 0x%p, ssie: %s, stie: %s, seie: %s", xie.val_,
             (xie.xie_.ssie == true ? "Enable" : "Disable"),
             (xie.xie_.stie == true ? "Enable" : "Disable"),
             (xie.xie_.seie == true ? "Enable" : "Disable"));
      return os;
    }
  };

  /**
   * @brief Xcause 寄存器定义
   * @see priv-isa-asciidoc_20240411.pdf#3.1.15
   */
  class Xcause {
   public:
    enum {
      // 中断
      kInterrupt = 1ULL << 63,
      kUserSoftwareInterrupt = kInterrupt + 0,
      kSupervisorSoftwareInterrupt = kInterrupt + 1,
      kReserved1 = kInterrupt + 2,
      kMachineSoftwareInterrupt = kInterrupt + 3,
      kUserTimerInterrupt = kInterrupt + 4,
      kSupervisorTimerInterrupt = kInterrupt + 5,
      kReserved2 = kInterrupt + 6,
      kMachineTimerInterrupt = kInterrupt + 7,
      kUserExternalInterrupt = kInterrupt + 8,
      kSupervisorExternalInterrupt = kInterrupt + 9,
      kReserved3 = kInterrupt + 10,
      kMachineExternalInterrupt = kInterrupt + 11,

      // 异常
      kInstructionAddressMisaligned = 0,
      kInstructionAccessFault = 1,
      kIllegalInstruction = 2,
      kBreakpoint = 3,
      kLoadAddressMisaligned = 4,
      kLoadAccessFault = 5,
      kStoreAmoAddressMisaligned = 6,
      kStoreAmoAccessFault = 7,
      kEcallUserMode = 8,
      kEcallSuperMode = 9,
      kReserved4 = 10,
      kEcallMachineMode = 11,
      kInstructionPageFault = 12,
      kLoadPageFault = 13,
      kReserved5 = 14,
      kStoreAmoPageFault = 15,
    };

    /// 最大中断数
    static constexpr const uint32_t kInterruptMaxCount = 12;

    /// 中断名
    static constexpr const char *const kInterruptNames[kInterruptMaxCount] = {
        "User Software Interrupt",
        "Supervisor Software Interrupt",
        "Reserved",
        "Machine Software Interrupt",
        "User Timer Interrupt",
        "Supervisor Timer Interrupt",
        "Reserved",
        "Machine Timer Interrupt",
        "User External Interrupt",
        "Supervisor External Interrupt",
        "Reserved",
        "Machine External Interrupt",
    };

    /// 最大异常数
    static constexpr const uint32_t kExceptionMaxCount = 16;

    /// 异常名
    static constexpr const char *const kExceptionNames[kExceptionMaxCount] = {
        "Instruction Address Misaligned",
        "Instruction Access Fault",
        "Illegal Instruction",
        "Breakpoint",
        "Load Address Misaligned",
        "Load Access Fault",
        "Store/AMO Address Misaligned",
        "Store/AMO Access Fault",
        "Environment Call from U-mode",
        "Environment Call from S-mode",
        "Reserved",
        "Environment Call from M-mode",
        "Instruction Page Fault",
        "Load Page Fault",
        "Reserved",
        "Store/AMO Page Fault",
    };

    union {
      struct {
        uint64_t exception_code : 63;
        uint64_t interrupt : 1;
      } xcause_;
      uint64_t val_;
    };

    explicit Xcause(uint64_t val) : val_(val) {}

    /// @name 构造/析构函数
    /// @{
    Xcause() = default;
    Xcause(const Xcause &) = default;
    Xcause(Xcause &&) = default;
    auto operator=(const Xcause &) -> Xcause & = default;
    auto operator=(Xcause &&) -> Xcause & = default;
    ~Xcause() = default;
    /// @}

    friend std::ostream &operator<<(std::ostream &os, const Xcause &xcause) {
      printf("val: 0x%p, exception_code: 0x%p, interrupt: %s, name: %s",
             xcause.val_, xcause.xcause_.exception_code,
             xcause.xcause_.interrupt ? "Yes" : "No",
             xcause.xcause_.interrupt
                 ? kInterruptNames[xcause.xcause_.exception_code]
                 : kExceptionNames[xcause.xcause_.exception_code]);
      return os;
    }
  };

  /**
   * @brief satp 寄存器定义
   * @see priv-isa-asciidoc_20240411.pdf#10.1.11
   */
  class Satp {
   public:
    enum {
      kBare = 0,
      kSv39 = 8,
      kSv48 = 9,
      kSv57 = 10,
      kSv64 = 11,
    };

    static constexpr const char *kModeNames[] = {
        "Bare",     "Reserved", "Reserved", "Reserved", "Reserved", "Reserved",
        "Reserved", "Reserved", "SV39",     "SV48",     "SV57",     "SV64",
    };

    union {
      struct {
        uint64_t ppn : 44;
        uint64_t asid : 16;
        uint64_t mode : 4;
      } satp_;
      uint64_t val_;
    };

    explicit Satp(uint64_t val) : val_(val) {}

    /// @name 构造/析构函数
    /// @{
    Satp() = default;
    Satp(const Satp &) = default;
    Satp(Satp &&) = default;
    auto operator=(const Satp &) -> Satp & = default;
    auto operator=(Satp &&) -> Satp & = default;
    ~Satp() = default;
    /// @}

    friend std::ostream &operator<<(std::ostream &os, const Satp &satp) {
      printf("val: 0x%p, ppn: 0x%p, asid: 0x%p, mode: %s", satp.val_,
             satp.satp_.ppn, satp.satp_.asid, kModeNames[satp.satp_.mode]);
      return os;
    }
  };

  /**
   * @brief 通用寄存器
   */
  struct Xregs {
    uint64_t zero;
    uint64_t ra;
    uint64_t sp;
    uint64_t gp;
    uint64_t tp;
    uint64_t t0;
    uint64_t t1;
    uint64_t t2;
    uint64_t s0;
    uint64_t s1;
    uint64_t a0;
    uint64_t a1;
    uint64_t a2;
    uint64_t a3;
    uint64_t a4;
    uint64_t a5;
    uint64_t a6;
    uint64_t a7;
    uint64_t s2;
    uint64_t s3;
    uint64_t s4;
    uint64_t s5;
    uint64_t s6;
    uint64_t s7;
    uint64_t s8;
    uint64_t s9;
    uint64_t s10;
    uint64_t s11;
    uint64_t t3;
    uint64_t t4;
    uint64_t t5;
    uint64_t t6;
    friend std::ostream &operator<<(std::ostream &os, const Xregs &xregs) {
      printf("zero: 0x%p, ", xregs.zero);
      printf("ra: 0x%p, ", xregs.ra);
      printf("sp: 0x%p, ", xregs.sp);
      printf("gp: 0x%p\n", xregs.gp);
      printf("tp: 0x%p, ", xregs.tp);
      printf("t0: 0x%p, ", xregs.t0);
      printf("t1: 0x%p, ", xregs.t1);
      printf("t2: 0x%p\n", xregs.t2);
      printf("s0: 0x%p, ", xregs.s0);
      printf("s1: 0x%p, ", xregs.s1);
      printf("a0: 0x%p, ", xregs.a0);
      printf("a1: 0x%p\n", xregs.a1);
      printf("a2: 0x%p, ", xregs.a2);
      printf("a3: 0x%p, ", xregs.a3);
      printf("a4: 0x%p, ", xregs.a4);
      printf("a5: 0x%p\n", xregs.a5);
      printf("a6: 0x%p, ", xregs.a6);
      printf("a7: 0x%p, ", xregs.a7);
      printf("s2: 0x%p, ", xregs.s2);
      printf("s3: 0x%p\n", xregs.s3);
      printf("s4: 0x%p, ", xregs.s4);
      printf("s5: 0x%p, ", xregs.s5);
      printf("s6: 0x%p, ", xregs.s6);
      printf("s7: 0x%p\n", xregs.s7);
      printf("s8: 0x%p, ", xregs.s8);
      printf("s9: 0x%p, ", xregs.s9);
      printf("s10: 0x%p, ", xregs.s10);
      printf("s11: 0x%p\n", xregs.s11);
      printf("t3: 0x%p, ", xregs.t3);
      printf("t4: 0x%p, ", xregs.t4);
      printf("t5: 0x%p, ", xregs.t5);
      printf("t6: 0x%p", xregs.t6);
      return os;
    }
  };

  /**
   * @brief 浮点寄存器
   */
  struct Fregs {
    uintptr_t ft0;
    uintptr_t ft1;
    uintptr_t ft2;
    uintptr_t ft3;
    uintptr_t ft4;
    uintptr_t ft5;
    uintptr_t ft6;
    uintptr_t ft7;
    uintptr_t fs0;
    uintptr_t fs1;
    uintptr_t fa0;
    uintptr_t fa1;
    uintptr_t fa2;
    uintptr_t fa3;
    uintptr_t fa4;
    uintptr_t fa5;
    uintptr_t fa6;
    uintptr_t fa7;
    uintptr_t fs2;
    uintptr_t fs3;
    uintptr_t fs4;
    uintptr_t fs5;
    uintptr_t fs6;
    uintptr_t fs7;
    uintptr_t fs8;
    uintptr_t fs9;
    uintptr_t fs10;
    uintptr_t fs11;
    uintptr_t ft8;
    uintptr_t ft9;
    uintptr_t ft10;
    uintptr_t ft11;
    friend std::ostream &operator<<(std::ostream &os, const Fregs &fregs) {
      printf("ft0: 0x%p, ", fregs.ft0);
      printf("ft1: 0x%p, ", fregs.ft1);
      printf("ft2: 0x%p, ", fregs.ft2);
      printf("ft3: 0x%p\n", fregs.ft3);
      printf("ft4: 0x%p, ", fregs.ft4);
      printf("ft5: 0x%p, ", fregs.ft5);
      printf("ft6: 0x%p, ", fregs.ft6);
      printf("ft7: 0x%p\n", fregs.ft7);
      printf("fs0: 0x%p, ", fregs.fs0);
      printf("fs1: 0x%p, ", fregs.fs1);
      printf("fa0: 0x%p, ", fregs.fa0);
      printf("fa1: 0x%p\n", fregs.fa1);
      printf("fa2: 0x%p, ", fregs.fa2);
      printf("fa3: 0x%p, ", fregs.fa3);
      printf("fa4: 0x%p, ", fregs.fa4);
      printf("fa5: 0x%p\n", fregs.fa5);
      printf("fa6: 0x%p, ", fregs.fa6);
      printf("fa7: 0x%p, ", fregs.fa7);
      printf("fs2: 0x%p, ", fregs.fs2);
      printf("fs3: 0x%p\n", fregs.fs3);
      printf("fs4: 0x%p, ", fregs.fs4);
      printf("fs5: 0x%p, ", fregs.fs5);
      printf("fs6: 0x%p, ", fregs.fs6);
      printf("fs7: 0x%p\n", fregs.fs7);
      printf("fs8: 0x%p, ", fregs.fs8);
      printf("fs9: 0x%p, ", fregs.fs9);
      printf("fs10: 0x%p, ", fregs.fs10);
      printf("fs11: 0x%p\n", fregs.fs11);
      printf("ft8: 0x%p, ", fregs.ft8);
      printf("ft9: 0x%p, ", fregs.ft9);
      printf("ft10: 0x%p, ", fregs.ft10);
      printf("ft11: 0x%p", fregs.ft11);
      return os;
    }
  };

  /**
   * @brief 调用者保存寄存器
   * @see riscv-abi.pdf#1.1
   * @see riscv-abi.pdf#1.2
   */
  struct CallerRegs {
    uintptr_t ra;
    uintptr_t t0;
    uintptr_t t1;
    uintptr_t t2;
    uintptr_t a0;
    uintptr_t a1;
    uintptr_t a2;
    uintptr_t a3;
    uintptr_t a4;
    uintptr_t a5;
    uintptr_t a6;
    uintptr_t a7;
    uintptr_t t3;
    uintptr_t t4;
    uintptr_t t5;
    uintptr_t t6;
    uintptr_t ft0;
    uintptr_t ft1;
    uintptr_t ft2;
    uintptr_t ft3;
    uintptr_t ft4;
    uintptr_t ft5;
    uintptr_t ft6;
    uintptr_t ft7;
    uintptr_t fa0;
    uintptr_t fa1;
    uintptr_t fa2;
    uintptr_t fa3;
    uintptr_t fa4;
    uintptr_t fa5;
    uintptr_t fa6;
    uintptr_t fa7;
    uintptr_t ft8;
    uintptr_t ft9;
    uintptr_t ft10;
    uintptr_t ft11;
    friend std::ostream &operator<<(std::ostream &os,
                                    const CallerRegs &caller_regs) {
      printf("ra: 0x%p, ", caller_regs.ra);
      printf("t0: 0x%p, ", caller_regs.t0);
      printf("t1: 0x%p, ", caller_regs.t1);
      printf("t2: 0x%p, ", caller_regs.t2);
      printf("a0: 0x%p\n", caller_regs.a0);
      printf("a1: 0x%p, ", caller_regs.a1);
      printf("a2: 0x%p, ", caller_regs.a2);
      printf("a3: 0x%p, ", caller_regs.a3);
      printf("a4: 0x%p\n", caller_regs.a4);
      printf("a5: 0x%p, ", caller_regs.a5);
      printf("a6: 0x%p, ", caller_regs.a6);
      printf("a7: 0x%p, ", caller_regs.a7);
      printf("t3: 0x%p\n", caller_regs.t3);
      printf("t4: 0x%p, ", caller_regs.t4);
      printf("t5: 0x%p, ", caller_regs.t5);
      printf("t6: 0x%p, ", caller_regs.t6);
      printf("ft0: 0x%p\n", caller_regs.ft0);
      printf("ft1: 0x%p, ", caller_regs.ft1);
      printf("ft2: 0x%p, ", caller_regs.ft2);
      printf("ft3: 0x%p, ", caller_regs.ft3);
      printf("ft4: 0x%p\n", caller_regs.ft4);
      printf("ft5: 0x%p, ", caller_regs.ft5);
      printf("ft6: 0x%p, ", caller_regs.ft6);
      printf("ft7: 0x%p, ", caller_regs.ft7);
      printf("fa0: 0x%p\n", caller_regs.fa0);
      printf("fa1: 0x%p", caller_regs.fa1);
      printf("fa2: 0x%p", caller_regs.fa2);
      printf("fa3: 0x%p", caller_regs.fa3);
      printf("fa4: 0x%p", caller_regs.fa4);
      printf("fa5: 0x%p", caller_regs.fa5);
      printf("fa6: 0x%p", caller_regs.fa6);
      printf("fa7: 0x%p", caller_regs.fa7);
      printf("ft8: 0x%p", caller_regs.ft8);
      printf("ft9: 0x%p", caller_regs.ft9);
      printf("ft10: 0x%p", caller_regs.ft10);
      printf("ft11: 0x%p", caller_regs.ft11);
      return os;
    }
  };

  /**
   * @brief 被调用者保存寄存器
   * @see riscv-abi.pdf#1.1
   * @see riscv-abi.pdf#1.2
   */
  struct CalleeRegs {
    uintptr_t sp;
    uintptr_t s0;
    uintptr_t s1;
    uintptr_t s2;
    uintptr_t s3;
    uintptr_t s4;
    uintptr_t s5;
    uintptr_t s6;
    uintptr_t s7;
    uintptr_t s8;
    uintptr_t s9;
    uintptr_t s10;
    uintptr_t s11;
    uintptr_t fs0;
    uintptr_t fs1;
    uintptr_t fs2;
    uintptr_t fs3;
    uintptr_t fs4;
    uintptr_t fs5;
    uintptr_t fs6;
    uintptr_t fs7;
    uintptr_t fs8;
    uintptr_t fs9;
    uintptr_t fs10;
    uintptr_t fs11;
    friend std::ostream &operator<<(std::ostream &os,
                                    const CalleeRegs &callee_regs) {
      printf("sp: 0x%p, ", callee_regs.sp);
      printf("s0: 0x%p, ", callee_regs.s0);
      printf("s1: 0x%p, ", callee_regs.s1);
      printf("s2: 0x%p\n", callee_regs.s2);
      printf("s3: 0x%p, ", callee_regs.s3);
      printf("s4: 0x%p, ", callee_regs.s4);
      printf("s5: 0x%p, ", callee_regs.s5);
      printf("s6: 0x%p\n", callee_regs.s6);
      printf("s7: 0x%p, ", callee_regs.s7);
      printf("s8: 0x%p, ", callee_regs.s8);
      printf("s9: 0x%p, ", callee_regs.s9);
      printf("s10: 0x%p\n", callee_regs.s10);
      printf("s11: 0x%p, ", callee_regs.s11);
      printf("fs0: 0x%p, ", callee_regs.fs0);
      printf("fs1: 0x%p, ", callee_regs.fs1);
      printf("fs2: 0x%p\n", callee_regs.fs2);
      printf("fs3: 0x%p, ", callee_regs.fs3);
      printf("fs4: 0x%p, ", callee_regs.fs4);
      printf("fs5: 0x%p, ", callee_regs.fs5);
      printf("fs6: 0x%p\n", callee_regs.fs6);
      printf("fs7: 0x%p, ", callee_regs.fs7);
      printf("fs8: 0x%p, ", callee_regs.fs8);
      printf("fs9: 0x%p, ", callee_regs.fs9);
      printf("fs10: 0x%p\n", callee_regs.fs10);
      printf("fs11: 0x%p", callee_regs.fs11);
      return os;
    }
  };

  /**
   * @brief 所有寄存器，在中断时使用，共 32+32+7=71 个
   */
  struct AllRegs {
    Xregs xregs;
    Fregs fregs;
    uintptr_t sepc;
    uintptr_t stval;
    Xcause xcause;
    Xie xie;
    Xstatus xstatus;
    Satp satp;
    uintptr_t sscratch;
    // friend std::ostream &operator<<(std::ostream &_os,
    //                                 const AllRegs &all_regs) {
    //   (void)all_regs.fregs;
    //   _os << all_regs.xregs << std::endl;
    //   printf(
    //       "sepc: 0x%p, stval: 0x%p, scause: 0x%p, sie: 0x%p, xstatus: "
    //       "0x%p, satp: 0x%p, sscratch: 0x%p",
    //       all_regs.sepc, all_regs.stval, all_regs.scause, all_regs.sie,
    //       all_regs.xstatus_.val, all_regs.satp.val, all_regs.sscratch);
    //   return _os;
    // }
  };

  /**
   * @brief 读取所有寄存器
   * @param  all_regs        要保存读取到的的值
   */
  static inline void ReadAllRegs(AllRegs &all_regs) {
    asm("mv %0, zero" : "=r"(all_regs.xregs.zero));
    asm("mv %0, ra" : "=r"(all_regs.xregs.ra));
    asm("mv %0, sp" : "=r"(all_regs.xregs.sp));
    asm("mv %0, gp" : "=r"(all_regs.xregs.gp));
    asm("mv %0, tp" : "=r"(all_regs.xregs.tp));
    asm("mv %0, t0" : "=r"(all_regs.xregs.t0));
    asm("mv %0, t1" : "=r"(all_regs.xregs.t1));
    asm("mv %0, t2" : "=r"(all_regs.xregs.t2));
    asm("mv %0, s0" : "=r"(all_regs.xregs.s0));
    asm("mv %0, s1" : "=r"(all_regs.xregs.s1));
    asm("mv %0, a0" : "=r"(all_regs.xregs.a0));
    asm("mv %0, a1" : "=r"(all_regs.xregs.a1));
    asm("mv %0, a2" : "=r"(all_regs.xregs.a2));
    asm("mv %0, a3" : "=r"(all_regs.xregs.a3));
    asm("mv %0, a4" : "=r"(all_regs.xregs.a4));
    asm("mv %0, a5" : "=r"(all_regs.xregs.a5));
    asm("mv %0, a6" : "=r"(all_regs.xregs.a6));
    asm("mv %0, a7" : "=r"(all_regs.xregs.a7));
    asm("mv %0, s2" : "=r"(all_regs.xregs.s2));
    asm("mv %0, s3" : "=r"(all_regs.xregs.s3));
    asm("mv %0, s4" : "=r"(all_regs.xregs.s4));
    asm("mv %0, s5" : "=r"(all_regs.xregs.s5));
    asm("mv %0, s6" : "=r"(all_regs.xregs.s6));
    asm("mv %0, s7" : "=r"(all_regs.xregs.s7));
    asm("mv %0, s8" : "=r"(all_regs.xregs.s8));
    asm("mv %0, s9" : "=r"(all_regs.xregs.s9));
    asm("mv %0, s10" : "=r"(all_regs.xregs.s10));
    asm("mv %0, s11" : "=r"(all_regs.xregs.s11));
    asm("mv %0, t3" : "=r"(all_regs.xregs.t3));
    asm("mv %0, t4" : "=r"(all_regs.xregs.t4));
    asm("mv %0, t5" : "=r"(all_regs.xregs.t5));
    asm("mv %0, t6" : "=r"(all_regs.xregs.t6));

    //    asm("mv %0, ft0" : "=r"(all_regs.fregs.ft0));
    //    asm("mv %0, ft1" : "=r"(all_regs.fregs.ft1));
    //    asm("mv %0, ft2" : "=r"(all_regs.fregs.ft2));
    //    asm("mv %0, ft3" : "=r"(all_regs.fregs.ft3));
    //    asm("mv %0, ft4" : "=r"(all_regs.fregs.ft4));
    //    asm("mv %0, ft5" : "=r"(all_regs.fregs.ft5));
    //    asm("mv %0, ft6" : "=r"(all_regs.fregs.ft6));
    //    asm("mv %0, ft7" : "=r"(all_regs.fregs.ft7));
    //    asm("mv %0, fs0" : "=r"(all_regs.fregs.fs0));
    //    asm("mv %0, fs1" : "=r"(all_regs.fregs.fs1));
    //    asm("mv %0, fa0" : "=r"(all_regs.fregs.fa0));
    //    asm("mv %0, fa1" : "=r"(all_regs.fregs.fa1));
    //    asm("mv %0, fa2" : "=r"(all_regs.fregs.fa2));
    //    asm("mv %0, fa3" : "=r"(all_regs.fregs.fa3));
    //    asm("mv %0, fa4" : "=r"(all_regs.fregs.fa4));
    //    asm("mv %0, fa5" : "=r"(all_regs.fregs.fa5));
    //    asm("mv %0, fa6" : "=r"(all_regs.fregs.fa6));
    //    asm("mv %0, fa7" : "=r"(all_regs.fregs.fa7));
    //    asm("mv %0, fs2" : "=r"(all_regs.fregs.fs2));
    //    asm("mv %0, fs3" : "=r"(all_regs.fregs.fs3));
    //    asm("mv %0, fs4" : "=r"(all_regs.fregs.fs4));
    //    asm("mv %0, fs5" : "=r"(all_regs.fregs.fs5));
    //    asm("mv %0, fs6" : "=r"(all_regs.fregs.fs6));
    //    asm("mv %0, fs7" : "=r"(all_regs.fregs.fs7));
    //    asm("mv %0, fs8" : "=r"(all_regs.fregs.fs8));
    //    asm("mv %0, fs9" : "=r"(all_regs.fregs.fs9));
    //    asm("mv %0, fs10" : "=r"(all_regs.fregs.fs10));
    //    asm("mv %0, fs11" : "=r"(all_regs.fregs.fs11));
    //    asm("mv %0, ft8" : "=r"(all_regs.fregs.ft8));
    //    asm("mv %0, ft9" : "=r"(all_regs.fregs.ft9));
    //    asm("mv %0, ft10" : "=r"(all_regs.fregs.ft10));
    //    asm("mv %0, ft11" : "=r"(all_regs.fregs.ft11));

    asm volatile("csrr %0, sepc" : "=r"(all_regs.sepc));
    asm volatile("csrr %0, stval" : "=r"(all_regs.stval));
    asm volatile("csrr %0, scause" : "=r"(all_regs.xcause));
    asm volatile("csrr %0, xstatus" : "=r"(all_regs.xstatus));
    asm volatile("csrr %0, sscratch" : "=r"(all_regs.sscratch));

    return;
  }

  /**
   * @brief 上下文，用于任务切换
   * @note caller_regs 由编译器保存/恢复
   */
  struct Context {
    /// 运行此任务的 core id
    uintptr_t coreid;
    uintptr_t ra;
    CalleeRegs callee_regs;
    Satp satp;
    uintptr_t sepc;
    Xstatus xstatus;
    Xie xie;
    Xip xip;
    uintptr_t sscratch;
    // friend std::ostream &operator<<(std::ostream &_os,
    //                                 const Context &_context) {
    //   printf("coreid: 0x%X, ", _context.coreid);
    //   printf("ra: 0x%p\n", _context.ra);
    //   std::cout << _context.callee_regs << std::endl;
    //   printf("satp: 0x%p, ", _context.satp.val);
    //   printf("sepc: 0x%p, ", _context.sepc);
    //   printf("xstatus: 0x%p, ", _context.xstatus_.val);
    //   printf("sie: 0x%p, ", _context.sie);
    //   printf("sip: 0x%p, ", _context.sip);
    //   printf("sscratch: 0x%p", _context.sscratch);
    //   return _os;
    // }
  };

  /**
   * @brief 读取 sstatus 寄存器
   * @return uint64_t         读取到的值
   */
  static inline Xstatus ReadSstatus(void) {
    Xstatus x;
    asm("csrr %0, sstatus" : "=r"(x));
    return x;
  }

  /**
   * @brief 写 sstatus 寄存器
   * @param  x                要写的值
   */
  static inline void WriteSstatus(Xstatus x) {
    asm("csrw sstatus, %0" : : "r"(x));
  }

  /**
   * @brief 读 sip
   * @return Xip 读取到的值
   * @note Supervisor Interrupt Pending
   */
  static inline Xip ReadSip(void) {
    Xip x;
    asm("csrr %0, sip" : "=r"(x));
    return x;
  }

  /**
   * @brief 写 sip
   * @param  x               要写的值
   */
  static inline void WriteSip(Xip x) { asm("csrw sip, %0" : : "r"(x)); }

  /**
   * @brief 读 sie
   * @return Xie         读到的值
   */
  static inline Xie ReadSie() {
    Xie x;
    asm("csrr %0, sie" : "=r"(x));
    return x;
  }

  /**
   * @brief 写 sie
   * @param  x                要写的值
   */
  static inline void WriteSie(Xie x) { asm("csrw sie, %0" : : "r"(x)); }

  /**
   * @brief 读 sepc
   * @return uint64_t         读到的值
   * @note machine exception program counter, holds the instruction address to
   * which a return from exception will go.
   */
  static inline uint64_t ReadSepc() {
    uint64_t x;
    asm("csrr %0, sepc" : "=r"(x));
    return x;
  }

  /**
   * @brief 写 sepc
   * @param  x               要写的值
   */
  static inline void WriteSepc(uint64_t x) { asm("csrw sepc, %0" : : "r"(x)); }

  /**
   * @brief 读 stvec
   * @return Xtvec 读到的值
   */
  static inline Xtvec ReadStvec() {
    Xtvec x;
    asm("csrr %0, stvec" : "=r"(x));
    return x;
  }

  /**
   * @brief 写 stvec
   * @param  x               要写的值
   */
  static inline void WriteStvec(Xtvec x) { asm("csrw stvec, %0" : : "r"(x)); }

  /**
   * @brief 读 sscratch 寄存器
   * @param  x                要写的值
   */
  static inline uint64_t ReadSscratch() {
    uint64_t x;
    __asm__ volatile("csrr %0, sscratch" : "=r"(x));
    return x;
  }

  /**
   * @brief 写 sscratch 寄存器
   * @param  x                要写的值
   */
  static inline void WriteSscratch(uint64_t x) {
    asm("csrw sscratch, %0" : : "r"(x));
  }

  /**
   * @brief 读 scause 寄存器 Supervisor Trap Cause
   * @return Xcause         读到的值
   */
  static inline Xcause ReadScause() {
    Xcause x;
    asm("csrr %0, scause" : "=r"(x));
    return x;
  }

  /**
   * @brief 读 stval 寄存器 Supervisor Trap Value
   * @return uint64_t         读到的值
   */
  static inline uint64_t ReadStval() {
    uint64_t x;
    asm("csrr %0, stval" : "=r"(x));
    return x;
  }

  /**
   * @brief 读 time 寄存器 supervisor-mode cycle counter
   * @return uint64_t         读到的值
   */
  static inline uint64_t ReadTime() {
    uint64_t x;
    // asm ("csrr %0, time" : "=r" (x) );
    // this instruction will trap in SBI
    asm("rdtime %0" : "=r"(x));
    return x;
  }

  /**
   * @brief 设置中断模式，直接
   */
  static inline void SetStvecDirect() {
    auto stvec = ReadStvec();
    stvec.xtvec_.mode = Xtvec::kDirect;
    WriteStvec(stvec);
  }

  /**
   * @brief 设置中断模式，直接
   * @param addr 处理函数的地址
   */
  static inline void SetStvecDirect(uint64_t addr) {
    auto stvec = ReadStvec();
    stvec.xtvec_.mode = Xtvec::kDirect;
    stvec.xtvec_.base = addr >> 2;
    WriteStvec(stvec);
  }

  /**
   * @brief 设置中断模式，向量
   */
  static inline void SetStvecVectored() {
    auto stvec = ReadStvec();
    stvec.xtvec_.mode = Xtvec::kVectored;
    WriteStvec(stvec);
  }

  /**
   * @brief 允许中断
   */
  static inline void EnableSupervisorIntr() {
    WriteSstatus(Xstatus(ReadSstatus().val_ | Xstatus::kXstatusSie));
  }

  /**
   * @brief 禁止中断
   */
  static inline void DisableIntr() {
    WriteSstatus(Xstatus(ReadSstatus().val_ & ~Xstatus::kXstatusSie));
  }

  /**
   * @brief 允许软件中断
   */
  static inline void EnableSupervisorSoftware() {
    WriteSie(Xie(ReadSie().val_ | Xie::kSsie));
  }

  /**
   * @brief 禁止软件中断
   */
  static inline void DisableSupervisorSoftware() {
    WriteSie(Xie(ReadSie().val_ & ~Xie::kSsie));
  }

  /**
   * @brief 允许定时器中断
   */
  static inline void EnableSupervisorTimer() {
    WriteSie(Xie(ReadSie().val_ | Xie::kStie));
  }

  /**
   * @brief 禁止定时器中断
   */
  static inline void DisableSupervisorTimer() {
    WriteSie(Xie(ReadSie().val_ & ~Xie::kStie));
  }

  /**
   * @brief 允许外部中断
   */
  static inline void EnableSupervisorExternal() {
    WriteSie(Xie(ReadSie().val_ | Xie::kSeie));
  }

  /**
   * @brief 禁止外部中断
   */
  static inline void DisableSupervisorExternal() {
    WriteSie(Xie(ReadSie().val_ & ~Xie::kSeie));
  }
};

#endif  // SIMPLEKERNEL_SRC_KERNEL_ARCH_RISCV64_CPU_HPP_
