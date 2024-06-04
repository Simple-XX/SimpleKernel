/*
   Register access classes for RISC-V system registers.
   SPDX-License-Identifier: Unlicense

   https://five-embeddev.com/

*/

#ifndef RISCV_CSR_HPP
#define RISCV_CSR_HPP

#include <cstdint>

// ------------------------------------------------------------------------
// Base and common classes

namespace riscv {
    namespace csr {
#if __riscv_xlen==32 
        using uint_xlen_t  = std::uint32_t;
        using uint_csr32_t = std::uint32_t;
        using uint_csr64_t = std::uint32_t;
#elif __riscv_xlen==64
        using uint_xlen_t  = std::uint64_t;
        using uint_csr32_t = std::uint32_t;
        using uint_csr64_t = std::uint64_t;
#else
        using uint_xlen_t  = std::uint32_t;
        using uint_csr32_t = std::uint32_t;
        using uint_csr64_t = std::uint32_t;
#error "riscv::csr: unknown __riscv_xlen"
#endif
        /** Immediate instructions use a 5 bit immediate field */
        static constexpr uint_xlen_t CSR_IMM_OP_MASK = 0x01F; 
        
        /** CSR: Read only, and read-write base class */
        template<class C> class read_only_reg {
        public :
            using read_datatype_t = typename C::datatype;

            read_only_reg(void) {}
            read_only_reg(const read_only_reg&) = delete;
            read_only_reg& operator=(const read_only_reg&) = delete;
        
            /** Read the CSR value */
            static inline read_datatype_t read(void) {
                return C::read();
            }
            /** Operator alias to read the CSR value */
            inline read_datatype_t operator()(void) {
                return C::read();
            }
        };
        /** CSR: Write only, and read-write base class */
        template<class C> class write_only_reg {
        public :
            using write_datatype_t = typename C::datatype;

            write_only_reg(void) {}
            write_only_reg(const write_only_reg&) = delete;
            write_only_reg& operator=(const write_only_reg&) = delete;

            /** Write a constant to the CSR. */
            template<write_datatype_t VALUE> void write_const(void) {
                if constexpr ((VALUE & CSR_IMM_OP_MASK) == VALUE) {
                    C::write_imm(VALUE);
                } else {
                    C::write(VALUE);
                }
            }
            /** Write to the CSR. */
            inline void write(const write_datatype_t value) {
                C::write(value);
            }
            /** Set a constant mask of bits in the CSR. */
            template<write_datatype_t MASK> void set_const(void) {
                if constexpr ((MASK & CSR_IMM_OP_MASK) == MASK) {
                    C::set_bits_imm(MASK);
                } else {
                    C::set_bits(MASK);
                }
            }
            /** Set a mask of bits in the CSR. */
            inline void set(write_datatype_t mask) {
                C::set_bits(mask);
            }
            /** Clear a constant mask of bits in the CSR. */
            template<write_datatype_t MASK> void clr_const(void) {
                if constexpr ((MASK & CSR_IMM_OP_MASK) == MASK) {
                    C::clr_bits_imm(MASK);
                } else {
                    C::clr_bits(MASK);
                }
            }
            /** Clear a mask of bits in the CSR. */
            inline void clr(write_datatype_t mask) {
                C::clr_bits(mask);
            }
            /** Operator alias to set mask of bits in the CSR. */
            inline void operator|=(write_datatype_t mask) {
                C::set_bits(mask);
            }
        };
        /** CSR: Read-write base class */
        template<class C> class read_write_reg : public read_only_reg<C>,
                                                 public write_only_reg<C>  {
        public:
            using datatype_t = typename C::datatype;

            read_write_reg(void) 
                : read_only_reg<C>()
                , write_only_reg<C>()
                {}
            read_write_reg(const read_write_reg&)=delete;
            read_write_reg& operator=(const read_write_reg&)=delete;

            /** Read from, then write a constant value to the CSR. */
            template<datatype_t VALUE> datatype_t read_write_const(void) {
                if constexpr ((VALUE & CSR_IMM_OP_MASK) == VALUE) {
                    return C::read_write_imm(VALUE);
                } else {
                    return C::read_write(VALUE);
                }
            }
            /** Read from, then write to the CSR. */
            inline datatype_t read_write(const datatype_t value) {
                return C::read_write(value);
            }
            /** Read from, then set a constant bit mask to the CSR. */
            template<datatype_t MASK> datatype_t read_set_bits_const(void) {
                if constexpr ((MASK & CSR_IMM_OP_MASK) == MASK) {
                    return C::read_set_bits_imm(MASK);
                } else {
                    return C::read_set_bits(MASK);
                }
            }
            /** Read from, then set a bit mask to the CSR. */
            inline datatype_t read_set_bits(const datatype_t mask) {
                return C::read_set_bits(mask);
            }
            /** Read from, then clear a constant bit mask to the CSR. */
            template<datatype_t MASK> datatype_t read_clr_bits_const(void) {
                if constexpr ((MASK & CSR_IMM_OP_MASK) == MASK) {
                    return C::read_clr_bits_imm(MASK);
                } else {
                    return C::read_clr_bits(MASK);
                }
            }
            /** Read from, then clear a bit mask to the CSR. */
            inline datatype_t read_clr_bits(const datatype_t mask) {
                return C::read_clr_bits(mask);
            }
        };
        /** CSR Field: Read only, and read-write base class */
        template<class C, class F> class read_only_field {
        public:
            using read_datatype_t = typename F::datatype;

            read_only_field(void) {}
            read_only_field(const read_only_field&)=delete;
            read_only_field& operator=(const read_only_field&)=delete;

            /** Read a given field value from a CSR */
            read_datatype_t read(void) {
                return (read_datatype_t) ((C::read() & F::BIT_MASK) >> F::BIT_OFFSET);
            }
        };
        /** CSR Field: Write only, and read-write base class */
        template<class C, class F> class write_only_field {
        public:
            using write_datatype_t = typename F::datatype;
            using reg_write_datatype_t = typename C::datatype;

            write_only_field(void) {}
            write_only_field(const write_only_field&)=delete;
            write_only_field& operator=(const write_only_field&)=delete;

            inline void set(void) {
                if constexpr ((F::BIT_MASK & CSR_IMM_OP_MASK) == F::BIT_MASK) {
                    C::set_bits_imm(F::BIT_MASK);
                } else {
                    C::set_bits(F::BIT_MASK);
                }
            }
            inline void clr(void) {
                if constexpr ((F::BIT_MASK & CSR_IMM_OP_MASK) == F::BIT_MASK) {
                    C::clr_bits_imm(F::BIT_MASK);
                } else {
                    C::clr_bits(F::BIT_MASK);
                }
            }
        };
        /** CSR Field: Read-write base class */
        template<class C, class F> class read_write_field 
            : public read_only_field<C,F>
            , public write_only_field<C,F>  {
        public:
            using datatype_t = typename F::datatype;
            using reg_datatype_t = typename C::datatype;

            read_write_field(void) 
                : read_only_field<C,F>()
                , write_only_field<C,F>()
                {}
            read_write_field(const read_write_field&)=delete;
            read_write_field& operator=(const read_write_field&)=delete;

            /* Read-modify-write to write a field.
               NOTE - not atomic.
             */
            inline void write(const datatype_t value) {
                auto org_value = C::read();
                auto new_value = (org_value & ~F::BIT_MASK) | 
                    (((reg_datatype_t)value << F::BIT_OFFSET) & F::BIT_MASK);
                C::write(new_value);
            }
            /* Read-modify-write to set a field, and return original value.
               NOTE - not atomic.
             */
            inline datatype_t read_write(const datatype_t value) {
                auto org_value = C::read();
                auto new_value = (org_value & ~F::BIT_MASK) | 
                    (((reg_datatype_t)value << F::BIT_OFFSET) & F::BIT_MASK);
                C::write(new_value);
                return (datatype_t) ((org_value & F::BIT_MASK) >> F::BIT_OFFSET);
            }

        };

        /** CSR access context and read/write permission.
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

    } /* csr */
} /* riscv */

// ------------------------------------------------------------------------
// Assembler operations and bit field definitions

namespace riscv {
    namespace csr {
        // ----------------------------------------------------------------
        // sscratch - SRW - Supervisor Mode Scratch Register 
        //
        /** Supervisor Mode Scratch Register assembler operations */
        struct sscratch_ops  {
            using datatype = uint_xlen_t;
            static constexpr priv_t priv = SRW; 
            
            /** Read sscratch */
            static uint_xlen_t read(void) {
                uint_xlen_t value;        
                __asm__ volatile ("csrr    %0, sscratch" 
                                  : "=r" (value)  /* output : register */
                                  : /* input : none */
                                  : /* clobbers: none */);
                return value;
            }
            
            
            /** Write sscratch */
            static void write(uint_xlen_t value) {
                __asm__ volatile ("csrw    sscratch, %0" 
                                  : /* output: none */ 
                                  : "r" (value) /* input : from register */
                                  : /* clobbers: none */);
            }
            /** Write immediate value to sscratch */
            static void write_imm(uint_xlen_t value) {
                __asm__ volatile ("csrwi    sscratch, %0" 
                                  : /* output: none */ 
                                  : "i" (value) /* input : from immediate */
                                  : /* clobbers: none */);
            }
            /** Read and then write to sscratch */
            static uint_xlen_t read_write(uint_xlen_t new_value) {
                uint_xlen_t prev_value;
                __asm__ volatile ("csrrw    %0, sscratch, %1"  
                                  : "=r" (prev_value) /* output: register %0 */
                                  : "r" (new_value)  /* input : register */
                                  : /* clobbers: none */);
                return prev_value;
            }
            /** Read and then write immediate value to sscratch */
            static uint_xlen_t read_write_imm(const uint8_t new_value) {
                uint_xlen_t prev_value;
                __asm__ volatile ("csrrwi    %0, sscratch, %1"  
                                  : "=r" (prev_value) /* output: register %0 */
                                  : "i" (new_value)  /* input : register */
                                  : /* clobbers: none */);
                return prev_value;
            }
        
            // ------------------------------------------
            // Register CSR bit set and clear instructions

            /** Atomic modify and set bits for sscratch */
            static void set_bits(uint_xlen_t mask) {
                __asm__ volatile ("csrrs    zero, sscratch, %0"  
                                  : /* output: none */ 
                                  : "r" (mask)  /* input : register */
                                  : /* clobbers: none */);
            }
            /** Atomic read and then and set bits for sscratch */
            static uint32_t read_set_bits(uint_xlen_t mask) {
                uint_xlen_t value;
                __asm__ volatile ("csrrs    %0, sscratch, %1"  
                                  : "=r" (value) /* output: register %0 */
                                  : "r" (mask)  /* input : register */
                                  : /* clobbers: none */);
                return value;
            }
            /** Atomic modify and clear bits for sscratch */
            static void clr_bits(uint_xlen_t mask) {
                __asm__ volatile ("csrrc    zero, sscratch, %0"  
                                  : /* output: none */ 
                                  : "r" (mask)  /* input : register */
                                  : /* clobbers: none */);
            }
            /** Atomic read and then and clear bits for sscratch */
            static uint32_t read_clr_bits(uint_xlen_t mask) {
                uint_xlen_t value;
                __asm__ volatile ("csrrc    %0, sscratch, %1"  
                                  : "=r" (value) /* output: register %0 */
                                  : "r" (mask)  /* input : register */
                                  : /* clobbers: none */);
                return value;
            }
        
            // ------------------------------------------
            // Immediate value CSR bit set and clear instructions (only up to 5 bits)
        
            /** Atomic modify and set bits from immediate for sscratch */
            static void set_bits_imm(const uint8_t mask) {
                __asm__ volatile ("csrrsi    zero, sscratch, %0"  
                                  : /* output: none */ 
                                  : "i" (mask)  /* input : register */
                                  : /* clobbers: none */);
            }
            /** Atomic read and then and set bits from immediate for sscratch */
            static uint_xlen_t read_set_bits_imm(const uint8_t mask) {
                uint_xlen_t value;
                __asm__ volatile ("csrrsi    %0, sscratch, %1"  
                                  : "=r" (value) /* output: register %0 */
                                  : "i" (mask)  /* input : register */
                                  : /* clobbers: none */);
                return value;
            }
            /** Atomic modify and clear bits from immediate for sscratch */
            static void clr_bits_imm(const uint8_t mask) {
                __asm__ volatile ("csrrci    zero, sscratch, %0"  
                                  : /* output: none */ 
                                  : "i" (mask)  /* input : register */
                                  : /* clobbers: none */);
            }
            /** Atomic read and then and clear bits from immediate for sscratch */
            static uint_xlen_t read_clr_bits_imm(const uint8_t mask) {
                uint_xlen_t value;
                __asm__ volatile ("csrrci    %0, sscratch, %1"  
                                  : "=r" (value) /* output: register %0 */
                                  : "i" (mask)  /* input : register */
                                  : /* clobbers: none */);
                return value;
            }
            
        }; /* sscratch_ops */

        // ----------------------------------------------------------------
        // sepc - SRW - Supervisor Exception Program Counter 
        //
        /** Supervisor Exception Program Counter assembler operations */
        struct sepc_ops  {
            using datatype = uint_xlen_t;
            static constexpr priv_t priv = SRW; 
            
            /** Read sepc */
            static uint_xlen_t read(void) {
                uint_xlen_t value;        
                __asm__ volatile ("csrr    %0, sepc" 
                                  : "=r" (value)  /* output : register */
                                  : /* input : none */
                                  : /* clobbers: none */);
                return value;
            }
            
            
            /** Write sepc */
            static void write(uint_xlen_t value) {
                __asm__ volatile ("csrw    sepc, %0" 
                                  : /* output: none */ 
                                  : "r" (value) /* input : from register */
                                  : /* clobbers: none */);
            }
            /** Write immediate value to sepc */
            static void write_imm(uint_xlen_t value) {
                __asm__ volatile ("csrwi    sepc, %0" 
                                  : /* output: none */ 
                                  : "i" (value) /* input : from immediate */
                                  : /* clobbers: none */);
            }
            /** Read and then write to sepc */
            static uint_xlen_t read_write(uint_xlen_t new_value) {
                uint_xlen_t prev_value;
                __asm__ volatile ("csrrw    %0, sepc, %1"  
                                  : "=r" (prev_value) /* output: register %0 */
                                  : "r" (new_value)  /* input : register */
                                  : /* clobbers: none */);
                return prev_value;
            }
            /** Read and then write immediate value to sepc */
            static uint_xlen_t read_write_imm(const uint8_t new_value) {
                uint_xlen_t prev_value;
                __asm__ volatile ("csrrwi    %0, sepc, %1"  
                                  : "=r" (prev_value) /* output: register %0 */
                                  : "i" (new_value)  /* input : register */
                                  : /* clobbers: none */);
                return prev_value;
            }
        
            // ------------------------------------------
            // Register CSR bit set and clear instructions

            /** Atomic modify and set bits for sepc */
            static void set_bits(uint_xlen_t mask) {
                __asm__ volatile ("csrrs    zero, sepc, %0"  
                                  : /* output: none */ 
                                  : "r" (mask)  /* input : register */
                                  : /* clobbers: none */);
            }
            /** Atomic read and then and set bits for sepc */
            static uint32_t read_set_bits(uint_xlen_t mask) {
                uint_xlen_t value;
                __asm__ volatile ("csrrs    %0, sepc, %1"  
                                  : "=r" (value) /* output: register %0 */
                                  : "r" (mask)  /* input : register */
                                  : /* clobbers: none */);
                return value;
            }
            /** Atomic modify and clear bits for sepc */
            static void clr_bits(uint_xlen_t mask) {
                __asm__ volatile ("csrrc    zero, sepc, %0"  
                                  : /* output: none */ 
                                  : "r" (mask)  /* input : register */
                                  : /* clobbers: none */);
            }
            /** Atomic read and then and clear bits for sepc */
            static uint32_t read_clr_bits(uint_xlen_t mask) {
                uint_xlen_t value;
                __asm__ volatile ("csrrc    %0, sepc, %1"  
                                  : "=r" (value) /* output: register %0 */
                                  : "r" (mask)  /* input : register */
                                  : /* clobbers: none */);
                return value;
            }
        
            // ------------------------------------------
            // Immediate value CSR bit set and clear instructions (only up to 5 bits)
        
            /** Atomic modify and set bits from immediate for sepc */
            static void set_bits_imm(const uint8_t mask) {
                __asm__ volatile ("csrrsi    zero, sepc, %0"  
                                  : /* output: none */ 
                                  : "i" (mask)  /* input : register */
                                  : /* clobbers: none */);
            }
            /** Atomic read and then and set bits from immediate for sepc */
            static uint_xlen_t read_set_bits_imm(const uint8_t mask) {
                uint_xlen_t value;
                __asm__ volatile ("csrrsi    %0, sepc, %1"  
                                  : "=r" (value) /* output: register %0 */
                                  : "i" (mask)  /* input : register */
                                  : /* clobbers: none */);
                return value;
            }
            /** Atomic modify and clear bits from immediate for sepc */
            static void clr_bits_imm(const uint8_t mask) {
                __asm__ volatile ("csrrci    zero, sepc, %0"  
                                  : /* output: none */ 
                                  : "i" (mask)  /* input : register */
                                  : /* clobbers: none */);
            }
            /** Atomic read and then and clear bits from immediate for sepc */
            static uint_xlen_t read_clr_bits_imm(const uint8_t mask) {
                uint_xlen_t value;
                __asm__ volatile ("csrrci    %0, sepc, %1"  
                                  : "=r" (value) /* output: register %0 */
                                  : "i" (mask)  /* input : register */
                                  : /* clobbers: none */);
                return value;
            }
            
        }; /* sepc_ops */

        // ----------------------------------------------------------------
        // scause - SRW - Supervisor Exception Cause 
        //
        /** Supervisor Exception Cause assembler operations */
        struct scause_ops  {
            using datatype = uint_xlen_t;
            static constexpr priv_t priv = SRW; 
            
            /** Read scause */
            static uint_xlen_t read(void) {
                uint_xlen_t value;        
                __asm__ volatile ("csrr    %0, scause" 
                                  : "=r" (value)  /* output : register */
                                  : /* input : none */
                                  : /* clobbers: none */);
                return value;
            }
            
            
            /** Write scause */
            static void write(uint_xlen_t value) {
                __asm__ volatile ("csrw    scause, %0" 
                                  : /* output: none */ 
                                  : "r" (value) /* input : from register */
                                  : /* clobbers: none */);
            }
            /** Write immediate value to scause */
            static void write_imm(uint_xlen_t value) {
                __asm__ volatile ("csrwi    scause, %0" 
                                  : /* output: none */ 
                                  : "i" (value) /* input : from immediate */
                                  : /* clobbers: none */);
            }
            /** Read and then write to scause */
            static uint_xlen_t read_write(uint_xlen_t new_value) {
                uint_xlen_t prev_value;
                __asm__ volatile ("csrrw    %0, scause, %1"  
                                  : "=r" (prev_value) /* output: register %0 */
                                  : "r" (new_value)  /* input : register */
                                  : /* clobbers: none */);
                return prev_value;
            }
            /** Read and then write immediate value to scause */
            static uint_xlen_t read_write_imm(const uint8_t new_value) {
                uint_xlen_t prev_value;
                __asm__ volatile ("csrrwi    %0, scause, %1"  
                                  : "=r" (prev_value) /* output: register %0 */
                                  : "i" (new_value)  /* input : register */
                                  : /* clobbers: none */);
                return prev_value;
            }
        
            // ------------------------------------------
            // Register CSR bit set and clear instructions

            /** Atomic modify and set bits for scause */
            static void set_bits(uint_xlen_t mask) {
                __asm__ volatile ("csrrs    zero, scause, %0"  
                                  : /* output: none */ 
                                  : "r" (mask)  /* input : register */
                                  : /* clobbers: none */);
            }
            /** Atomic read and then and set bits for scause */
            static uint32_t read_set_bits(uint_xlen_t mask) {
                uint_xlen_t value;
                __asm__ volatile ("csrrs    %0, scause, %1"  
                                  : "=r" (value) /* output: register %0 */
                                  : "r" (mask)  /* input : register */
                                  : /* clobbers: none */);
                return value;
            }
            /** Atomic modify and clear bits for scause */
            static void clr_bits(uint_xlen_t mask) {
                __asm__ volatile ("csrrc    zero, scause, %0"  
                                  : /* output: none */ 
                                  : "r" (mask)  /* input : register */
                                  : /* clobbers: none */);
            }
            /** Atomic read and then and clear bits for scause */
            static uint32_t read_clr_bits(uint_xlen_t mask) {
                uint_xlen_t value;
                __asm__ volatile ("csrrc    %0, scause, %1"  
                                  : "=r" (value) /* output: register %0 */
                                  : "r" (mask)  /* input : register */
                                  : /* clobbers: none */);
                return value;
            }
        
            // ------------------------------------------
            // Immediate value CSR bit set and clear instructions (only up to 5 bits)
        
            /** Atomic modify and set bits from immediate for scause */
            static void set_bits_imm(const uint8_t mask) {
                __asm__ volatile ("csrrsi    zero, scause, %0"  
                                  : /* output: none */ 
                                  : "i" (mask)  /* input : register */
                                  : /* clobbers: none */);
            }
            /** Atomic read and then and set bits from immediate for scause */
            static uint_xlen_t read_set_bits_imm(const uint8_t mask) {
                uint_xlen_t value;
                __asm__ volatile ("csrrsi    %0, scause, %1"  
                                  : "=r" (value) /* output: register %0 */
                                  : "i" (mask)  /* input : register */
                                  : /* clobbers: none */);
                return value;
            }
            /** Atomic modify and clear bits from immediate for scause */
            static void clr_bits_imm(const uint8_t mask) {
                __asm__ volatile ("csrrci    zero, scause, %0"  
                                  : /* output: none */ 
                                  : "i" (mask)  /* input : register */
                                  : /* clobbers: none */);
            }
            /** Atomic read and then and clear bits from immediate for scause */
            static uint_xlen_t read_clr_bits_imm(const uint8_t mask) {
                uint_xlen_t value;
                __asm__ volatile ("csrrci    %0, scause, %1"  
                                  : "=r" (value) /* output: register %0 */
                                  : "i" (mask)  /* input : register */
                                  : /* clobbers: none */);
                return value;
            }
            
        }; /* scause_ops */
        /** Parameter data for fields in scause */
        namespace scause_data {
            /** Parameter data for interrupt */
            struct interrupt {
                using datatype = uint_xlen_t;
                static constexpr uint_xlen_t BIT_OFFSET = (__riscv_xlen-1);
                static constexpr uint_xlen_t BIT_WIDTH  = 1;
                static constexpr uint_xlen_t BIT_MASK   = (0x1UL << ((__riscv_xlen-1)));
                static constexpr uint_xlen_t ALL_SET_MASK = 0x1;
            };
            /** Parameter data for exception_code */
            struct exception_code {
                using datatype = uint_xlen_t;
                static constexpr uint_xlen_t BIT_OFFSET = 0;
                static constexpr uint_xlen_t BIT_WIDTH  = ((__riscv_xlen-2)-(0) + 1);
                static constexpr uint_xlen_t BIT_MASK   = ((1UL<<(((__riscv_xlen-2)-(0) + 1)-1)) << (0));
                static constexpr uint_xlen_t ALL_SET_MASK = ((1UL<<(((__riscv_xlen-2)-(0) + 1)-1)) << (0));
            };
        } /* scause_data */

        // ----------------------------------------------------------------
        // sstatus - SRW - Supervisor Status 
        //
        /** Supervisor Status assembler operations */
        struct sstatus_ops  {
            using datatype = uint_xlen_t;
            static constexpr priv_t priv = SRW; 
            
            /** Read sstatus */
            static uint_xlen_t read(void) {
                uint_xlen_t value;        
                __asm__ volatile ("csrr    %0, sstatus" 
                                  : "=r" (value)  /* output : register */
                                  : /* input : none */
                                  : /* clobbers: none */);
                return value;
            }
            
            
            /** Write sstatus */
            static void write(uint_xlen_t value) {
                __asm__ volatile ("csrw    sstatus, %0" 
                                  : /* output: none */ 
                                  : "r" (value) /* input : from register */
                                  : /* clobbers: none */);
            }
            /** Write immediate value to sstatus */
            static void write_imm(uint_xlen_t value) {
                __asm__ volatile ("csrwi    sstatus, %0" 
                                  : /* output: none */ 
                                  : "i" (value) /* input : from immediate */
                                  : /* clobbers: none */);
            }
            /** Read and then write to sstatus */
            static uint_xlen_t read_write(uint_xlen_t new_value) {
                uint_xlen_t prev_value;
                __asm__ volatile ("csrrw    %0, sstatus, %1"  
                                  : "=r" (prev_value) /* output: register %0 */
                                  : "r" (new_value)  /* input : register */
                                  : /* clobbers: none */);
                return prev_value;
            }
            /** Read and then write immediate value to sstatus */
            static uint_xlen_t read_write_imm(const uint8_t new_value) {
                uint_xlen_t prev_value;
                __asm__ volatile ("csrrwi    %0, sstatus, %1"  
                                  : "=r" (prev_value) /* output: register %0 */
                                  : "i" (new_value)  /* input : register */
                                  : /* clobbers: none */);
                return prev_value;
            }
        
            // ------------------------------------------
            // Register CSR bit set and clear instructions

            /** Atomic modify and set bits for sstatus */
            static void set_bits(uint_xlen_t mask) {
                __asm__ volatile ("csrrs    zero, sstatus, %0"  
                                  : /* output: none */ 
                                  : "r" (mask)  /* input : register */
                                  : /* clobbers: none */);
            }
            /** Atomic read and then and set bits for sstatus */
            static uint32_t read_set_bits(uint_xlen_t mask) {
                uint_xlen_t value;
                __asm__ volatile ("csrrs    %0, sstatus, %1"  
                                  : "=r" (value) /* output: register %0 */
                                  : "r" (mask)  /* input : register */
                                  : /* clobbers: none */);
                return value;
            }
            /** Atomic modify and clear bits for sstatus */
            static void clr_bits(uint_xlen_t mask) {
                __asm__ volatile ("csrrc    zero, sstatus, %0"  
                                  : /* output: none */ 
                                  : "r" (mask)  /* input : register */
                                  : /* clobbers: none */);
            }
            /** Atomic read and then and clear bits for sstatus */
            static uint32_t read_clr_bits(uint_xlen_t mask) {
                uint_xlen_t value;
                __asm__ volatile ("csrrc    %0, sstatus, %1"  
                                  : "=r" (value) /* output: register %0 */
                                  : "r" (mask)  /* input : register */
                                  : /* clobbers: none */);
                return value;
            }
        
            // ------------------------------------------
            // Immediate value CSR bit set and clear instructions (only up to 5 bits)
        
            /** Atomic modify and set bits from immediate for sstatus */
            static void set_bits_imm(const uint8_t mask) {
                __asm__ volatile ("csrrsi    zero, sstatus, %0"  
                                  : /* output: none */ 
                                  : "i" (mask)  /* input : register */
                                  : /* clobbers: none */);
            }
            /** Atomic read and then and set bits from immediate for sstatus */
            static uint_xlen_t read_set_bits_imm(const uint8_t mask) {
                uint_xlen_t value;
                __asm__ volatile ("csrrsi    %0, sstatus, %1"  
                                  : "=r" (value) /* output: register %0 */
                                  : "i" (mask)  /* input : register */
                                  : /* clobbers: none */);
                return value;
            }
            /** Atomic modify and clear bits from immediate for sstatus */
            static void clr_bits_imm(const uint8_t mask) {
                __asm__ volatile ("csrrci    zero, sstatus, %0"  
                                  : /* output: none */ 
                                  : "i" (mask)  /* input : register */
                                  : /* clobbers: none */);
            }
            /** Atomic read and then and clear bits from immediate for sstatus */
            static uint_xlen_t read_clr_bits_imm(const uint8_t mask) {
                uint_xlen_t value;
                __asm__ volatile ("csrrci    %0, sstatus, %1"  
                                  : "=r" (value) /* output: register %0 */
                                  : "i" (mask)  /* input : register */
                                  : /* clobbers: none */);
                return value;
            }
            
        }; /* sstatus_ops */
        /** Parameter data for fields in sstatus */
        namespace sstatus_data {
            /** Parameter data for sie */
            struct sie {
                using datatype = uint_xlen_t;
                static constexpr uint_xlen_t BIT_OFFSET = 2;
                static constexpr uint_xlen_t BIT_WIDTH  = 1;
                static constexpr uint_xlen_t BIT_MASK   = 0x4;
                static constexpr uint_xlen_t ALL_SET_MASK = 0x1;
            };
            /** Parameter data for spie */
            struct spie {
                using datatype = uint_xlen_t;
                static constexpr uint_xlen_t BIT_OFFSET = 5;
                static constexpr uint_xlen_t BIT_WIDTH  = 1;
                static constexpr uint_xlen_t BIT_MASK   = 0x20;
                static constexpr uint_xlen_t ALL_SET_MASK = 0x1;
            };
            /** Parameter data for spp */
            struct spp {
                using datatype = uint_xlen_t;
                static constexpr uint_xlen_t BIT_OFFSET = 8;
                static constexpr uint_xlen_t BIT_WIDTH  = 1;
                static constexpr uint_xlen_t BIT_MASK   = 0x100;
                static constexpr uint_xlen_t ALL_SET_MASK = 0x1;
            };
        } /* sstatus_data */

        // ----------------------------------------------------------------
        // stvec - SRW - Supervisor Trap Vector Base Address 
        //
        /** Supervisor Trap Vector Base Address assembler operations */
        struct stvec_ops  {
            using datatype = uint_xlen_t;
            static constexpr priv_t priv = SRW; 
            
            /** Read stvec */
            static uint_xlen_t read(void) {
                uint_xlen_t value;        
                __asm__ volatile ("csrr    %0, stvec" 
                                  : "=r" (value)  /* output : register */
                                  : /* input : none */
                                  : /* clobbers: none */);
                return value;
            }
            
            
            /** Write stvec */
            static void write(uint_xlen_t value) {
                __asm__ volatile ("csrw    stvec, %0" 
                                  : /* output: none */ 
                                  : "r" (value) /* input : from register */
                                  : /* clobbers: none */);
            }
            /** Write immediate value to stvec */
            static void write_imm(uint_xlen_t value) {
                __asm__ volatile ("csrwi    stvec, %0" 
                                  : /* output: none */ 
                                  : "i" (value) /* input : from immediate */
                                  : /* clobbers: none */);
            }
            /** Read and then write to stvec */
            static uint_xlen_t read_write(uint_xlen_t new_value) {
                uint_xlen_t prev_value;
                __asm__ volatile ("csrrw    %0, stvec, %1"  
                                  : "=r" (prev_value) /* output: register %0 */
                                  : "r" (new_value)  /* input : register */
                                  : /* clobbers: none */);
                return prev_value;
            }
            /** Read and then write immediate value to stvec */
            static uint_xlen_t read_write_imm(const uint8_t new_value) {
                uint_xlen_t prev_value;
                __asm__ volatile ("csrrwi    %0, stvec, %1"  
                                  : "=r" (prev_value) /* output: register %0 */
                                  : "i" (new_value)  /* input : register */
                                  : /* clobbers: none */);
                return prev_value;
            }
        
            // ------------------------------------------
            // Register CSR bit set and clear instructions

            /** Atomic modify and set bits for stvec */
            static void set_bits(uint_xlen_t mask) {
                __asm__ volatile ("csrrs    zero, stvec, %0"  
                                  : /* output: none */ 
                                  : "r" (mask)  /* input : register */
                                  : /* clobbers: none */);
            }
            /** Atomic read and then and set bits for stvec */
            static uint32_t read_set_bits(uint_xlen_t mask) {
                uint_xlen_t value;
                __asm__ volatile ("csrrs    %0, stvec, %1"  
                                  : "=r" (value) /* output: register %0 */
                                  : "r" (mask)  /* input : register */
                                  : /* clobbers: none */);
                return value;
            }
            /** Atomic modify and clear bits for stvec */
            static void clr_bits(uint_xlen_t mask) {
                __asm__ volatile ("csrrc    zero, stvec, %0"  
                                  : /* output: none */ 
                                  : "r" (mask)  /* input : register */
                                  : /* clobbers: none */);
            }
            /** Atomic read and then and clear bits for stvec */
            static uint32_t read_clr_bits(uint_xlen_t mask) {
                uint_xlen_t value;
                __asm__ volatile ("csrrc    %0, stvec, %1"  
                                  : "=r" (value) /* output: register %0 */
                                  : "r" (mask)  /* input : register */
                                  : /* clobbers: none */);
                return value;
            }
        
            // ------------------------------------------
            // Immediate value CSR bit set and clear instructions (only up to 5 bits)
        
            /** Atomic modify and set bits from immediate for stvec */
            static void set_bits_imm(const uint8_t mask) {
                __asm__ volatile ("csrrsi    zero, stvec, %0"  
                                  : /* output: none */ 
                                  : "i" (mask)  /* input : register */
                                  : /* clobbers: none */);
            }
            /** Atomic read and then and set bits from immediate for stvec */
            static uint_xlen_t read_set_bits_imm(const uint8_t mask) {
                uint_xlen_t value;
                __asm__ volatile ("csrrsi    %0, stvec, %1"  
                                  : "=r" (value) /* output: register %0 */
                                  : "i" (mask)  /* input : register */
                                  : /* clobbers: none */);
                return value;
            }
            /** Atomic modify and clear bits from immediate for stvec */
            static void clr_bits_imm(const uint8_t mask) {
                __asm__ volatile ("csrrci    zero, stvec, %0"  
                                  : /* output: none */ 
                                  : "i" (mask)  /* input : register */
                                  : /* clobbers: none */);
            }
            /** Atomic read and then and clear bits from immediate for stvec */
            static uint_xlen_t read_clr_bits_imm(const uint8_t mask) {
                uint_xlen_t value;
                __asm__ volatile ("csrrci    %0, stvec, %1"  
                                  : "=r" (value) /* output: register %0 */
                                  : "i" (mask)  /* input : register */
                                  : /* clobbers: none */);
                return value;
            }
            
        }; /* stvec_ops */
        /** Parameter data for fields in stvec */
        namespace stvec_data {
            /** Parameter data for base */
            struct base {
                using datatype = uint_xlen_t;
                static constexpr uint_xlen_t BIT_OFFSET = 2;
                static constexpr uint_xlen_t BIT_WIDTH  = ((__riscv_xlen-1)-(2) + 1);
                static constexpr uint_xlen_t BIT_MASK   = ((1UL<<(((__riscv_xlen-1)-(2) + 1)-1)) << (2));
                static constexpr uint_xlen_t ALL_SET_MASK = ((1UL<<(((__riscv_xlen-1)-(2) + 1)-1)) << (0));
            };
            /** Parameter data for mode */
            struct mode {
                using datatype = uint_xlen_t;
                static constexpr uint_xlen_t BIT_OFFSET = 0;
                static constexpr uint_xlen_t BIT_WIDTH  = 2;
                static constexpr uint_xlen_t BIT_MASK   = 0x3;
                static constexpr uint_xlen_t ALL_SET_MASK = 0x3;
            };
        } /* stvec_data */

        // ----------------------------------------------------------------
        // sideleg - SRW - Supervisor Interrupt Delegation 
        //
        /** Supervisor Interrupt Delegation assembler operations */
        struct sideleg_ops  {
            using datatype = uint_xlen_t;
            static constexpr priv_t priv = SRW; 
            
            /** Read sideleg */
            static uint_xlen_t read(void) {
                uint_xlen_t value;        
                __asm__ volatile ("csrr    %0, sideleg" 
                                  : "=r" (value)  /* output : register */
                                  : /* input : none */
                                  : /* clobbers: none */);
                return value;
            }
            
            
            /** Write sideleg */
            static void write(uint_xlen_t value) {
                __asm__ volatile ("csrw    sideleg, %0" 
                                  : /* output: none */ 
                                  : "r" (value) /* input : from register */
                                  : /* clobbers: none */);
            }
            /** Write immediate value to sideleg */
            static void write_imm(uint_xlen_t value) {
                __asm__ volatile ("csrwi    sideleg, %0" 
                                  : /* output: none */ 
                                  : "i" (value) /* input : from immediate */
                                  : /* clobbers: none */);
            }
            /** Read and then write to sideleg */
            static uint_xlen_t read_write(uint_xlen_t new_value) {
                uint_xlen_t prev_value;
                __asm__ volatile ("csrrw    %0, sideleg, %1"  
                                  : "=r" (prev_value) /* output: register %0 */
                                  : "r" (new_value)  /* input : register */
                                  : /* clobbers: none */);
                return prev_value;
            }
            /** Read and then write immediate value to sideleg */
            static uint_xlen_t read_write_imm(const uint8_t new_value) {
                uint_xlen_t prev_value;
                __asm__ volatile ("csrrwi    %0, sideleg, %1"  
                                  : "=r" (prev_value) /* output: register %0 */
                                  : "i" (new_value)  /* input : register */
                                  : /* clobbers: none */);
                return prev_value;
            }
        
            // ------------------------------------------
            // Register CSR bit set and clear instructions

            /** Atomic modify and set bits for sideleg */
            static void set_bits(uint_xlen_t mask) {
                __asm__ volatile ("csrrs    zero, sideleg, %0"  
                                  : /* output: none */ 
                                  : "r" (mask)  /* input : register */
                                  : /* clobbers: none */);
            }
            /** Atomic read and then and set bits for sideleg */
            static uint32_t read_set_bits(uint_xlen_t mask) {
                uint_xlen_t value;
                __asm__ volatile ("csrrs    %0, sideleg, %1"  
                                  : "=r" (value) /* output: register %0 */
                                  : "r" (mask)  /* input : register */
                                  : /* clobbers: none */);
                return value;
            }
            /** Atomic modify and clear bits for sideleg */
            static void clr_bits(uint_xlen_t mask) {
                __asm__ volatile ("csrrc    zero, sideleg, %0"  
                                  : /* output: none */ 
                                  : "r" (mask)  /* input : register */
                                  : /* clobbers: none */);
            }
            /** Atomic read and then and clear bits for sideleg */
            static uint32_t read_clr_bits(uint_xlen_t mask) {
                uint_xlen_t value;
                __asm__ volatile ("csrrc    %0, sideleg, %1"  
                                  : "=r" (value) /* output: register %0 */
                                  : "r" (mask)  /* input : register */
                                  : /* clobbers: none */);
                return value;
            }
        
            // ------------------------------------------
            // Immediate value CSR bit set and clear instructions (only up to 5 bits)
        
            /** Atomic modify and set bits from immediate for sideleg */
            static void set_bits_imm(const uint8_t mask) {
                __asm__ volatile ("csrrsi    zero, sideleg, %0"  
                                  : /* output: none */ 
                                  : "i" (mask)  /* input : register */
                                  : /* clobbers: none */);
            }
            /** Atomic read and then and set bits from immediate for sideleg */
            static uint_xlen_t read_set_bits_imm(const uint8_t mask) {
                uint_xlen_t value;
                __asm__ volatile ("csrrsi    %0, sideleg, %1"  
                                  : "=r" (value) /* output: register %0 */
                                  : "i" (mask)  /* input : register */
                                  : /* clobbers: none */);
                return value;
            }
            /** Atomic modify and clear bits from immediate for sideleg */
            static void clr_bits_imm(const uint8_t mask) {
                __asm__ volatile ("csrrci    zero, sideleg, %0"  
                                  : /* output: none */ 
                                  : "i" (mask)  /* input : register */
                                  : /* clobbers: none */);
            }
            /** Atomic read and then and clear bits from immediate for sideleg */
            static uint_xlen_t read_clr_bits_imm(const uint8_t mask) {
                uint_xlen_t value;
                __asm__ volatile ("csrrci    %0, sideleg, %1"  
                                  : "=r" (value) /* output: register %0 */
                                  : "i" (mask)  /* input : register */
                                  : /* clobbers: none */);
                return value;
            }
            
        }; /* sideleg_ops */

        // ----------------------------------------------------------------
        // sedeleg - SRW - Supervisor Exception Delegation 
        //
        /** Supervisor Exception Delegation assembler operations */
        struct sedeleg_ops  {
            using datatype = uint_xlen_t;
            static constexpr priv_t priv = SRW; 
            
            /** Read sedeleg */
            static uint_xlen_t read(void) {
                uint_xlen_t value;        
                __asm__ volatile ("csrr    %0, sedeleg" 
                                  : "=r" (value)  /* output : register */
                                  : /* input : none */
                                  : /* clobbers: none */);
                return value;
            }
            
            
            /** Write sedeleg */
            static void write(uint_xlen_t value) {
                __asm__ volatile ("csrw    sedeleg, %0" 
                                  : /* output: none */ 
                                  : "r" (value) /* input : from register */
                                  : /* clobbers: none */);
            }
            /** Write immediate value to sedeleg */
            static void write_imm(uint_xlen_t value) {
                __asm__ volatile ("csrwi    sedeleg, %0" 
                                  : /* output: none */ 
                                  : "i" (value) /* input : from immediate */
                                  : /* clobbers: none */);
            }
            /** Read and then write to sedeleg */
            static uint_xlen_t read_write(uint_xlen_t new_value) {
                uint_xlen_t prev_value;
                __asm__ volatile ("csrrw    %0, sedeleg, %1"  
                                  : "=r" (prev_value) /* output: register %0 */
                                  : "r" (new_value)  /* input : register */
                                  : /* clobbers: none */);
                return prev_value;
            }
            /** Read and then write immediate value to sedeleg */
            static uint_xlen_t read_write_imm(const uint8_t new_value) {
                uint_xlen_t prev_value;
                __asm__ volatile ("csrrwi    %0, sedeleg, %1"  
                                  : "=r" (prev_value) /* output: register %0 */
                                  : "i" (new_value)  /* input : register */
                                  : /* clobbers: none */);
                return prev_value;
            }
        
            // ------------------------------------------
            // Register CSR bit set and clear instructions

            /** Atomic modify and set bits for sedeleg */
            static void set_bits(uint_xlen_t mask) {
                __asm__ volatile ("csrrs    zero, sedeleg, %0"  
                                  : /* output: none */ 
                                  : "r" (mask)  /* input : register */
                                  : /* clobbers: none */);
            }
            /** Atomic read and then and set bits for sedeleg */
            static uint32_t read_set_bits(uint_xlen_t mask) {
                uint_xlen_t value;
                __asm__ volatile ("csrrs    %0, sedeleg, %1"  
                                  : "=r" (value) /* output: register %0 */
                                  : "r" (mask)  /* input : register */
                                  : /* clobbers: none */);
                return value;
            }
            /** Atomic modify and clear bits for sedeleg */
            static void clr_bits(uint_xlen_t mask) {
                __asm__ volatile ("csrrc    zero, sedeleg, %0"  
                                  : /* output: none */ 
                                  : "r" (mask)  /* input : register */
                                  : /* clobbers: none */);
            }
            /** Atomic read and then and clear bits for sedeleg */
            static uint32_t read_clr_bits(uint_xlen_t mask) {
                uint_xlen_t value;
                __asm__ volatile ("csrrc    %0, sedeleg, %1"  
                                  : "=r" (value) /* output: register %0 */
                                  : "r" (mask)  /* input : register */
                                  : /* clobbers: none */);
                return value;
            }
        
            // ------------------------------------------
            // Immediate value CSR bit set and clear instructions (only up to 5 bits)
        
            /** Atomic modify and set bits from immediate for sedeleg */
            static void set_bits_imm(const uint8_t mask) {
                __asm__ volatile ("csrrsi    zero, sedeleg, %0"  
                                  : /* output: none */ 
                                  : "i" (mask)  /* input : register */
                                  : /* clobbers: none */);
            }
            /** Atomic read and then and set bits from immediate for sedeleg */
            static uint_xlen_t read_set_bits_imm(const uint8_t mask) {
                uint_xlen_t value;
                __asm__ volatile ("csrrsi    %0, sedeleg, %1"  
                                  : "=r" (value) /* output: register %0 */
                                  : "i" (mask)  /* input : register */
                                  : /* clobbers: none */);
                return value;
            }
            /** Atomic modify and clear bits from immediate for sedeleg */
            static void clr_bits_imm(const uint8_t mask) {
                __asm__ volatile ("csrrci    zero, sedeleg, %0"  
                                  : /* output: none */ 
                                  : "i" (mask)  /* input : register */
                                  : /* clobbers: none */);
            }
            /** Atomic read and then and clear bits from immediate for sedeleg */
            static uint_xlen_t read_clr_bits_imm(const uint8_t mask) {
                uint_xlen_t value;
                __asm__ volatile ("csrrci    %0, sedeleg, %1"  
                                  : "=r" (value) /* output: register %0 */
                                  : "i" (mask)  /* input : register */
                                  : /* clobbers: none */);
                return value;
            }
            
        }; /* sedeleg_ops */

        // ----------------------------------------------------------------
        // sip - SRW - Supervisor Interrupt Pending 
        //
        /** Supervisor Interrupt Pending assembler operations */
        struct sip_ops  {
            using datatype = uint_xlen_t;
            static constexpr priv_t priv = SRW; 
            
            /** Read sip */
            static uint_xlen_t read(void) {
                uint_xlen_t value;        
                __asm__ volatile ("csrr    %0, sip" 
                                  : "=r" (value)  /* output : register */
                                  : /* input : none */
                                  : /* clobbers: none */);
                return value;
            }
            
            
            /** Write sip */
            static void write(uint_xlen_t value) {
                __asm__ volatile ("csrw    sip, %0" 
                                  : /* output: none */ 
                                  : "r" (value) /* input : from register */
                                  : /* clobbers: none */);
            }
            /** Write immediate value to sip */
            static void write_imm(uint_xlen_t value) {
                __asm__ volatile ("csrwi    sip, %0" 
                                  : /* output: none */ 
                                  : "i" (value) /* input : from immediate */
                                  : /* clobbers: none */);
            }
            /** Read and then write to sip */
            static uint_xlen_t read_write(uint_xlen_t new_value) {
                uint_xlen_t prev_value;
                __asm__ volatile ("csrrw    %0, sip, %1"  
                                  : "=r" (prev_value) /* output: register %0 */
                                  : "r" (new_value)  /* input : register */
                                  : /* clobbers: none */);
                return prev_value;
            }
            /** Read and then write immediate value to sip */
            static uint_xlen_t read_write_imm(const uint8_t new_value) {
                uint_xlen_t prev_value;
                __asm__ volatile ("csrrwi    %0, sip, %1"  
                                  : "=r" (prev_value) /* output: register %0 */
                                  : "i" (new_value)  /* input : register */
                                  : /* clobbers: none */);
                return prev_value;
            }
        
            // ------------------------------------------
            // Register CSR bit set and clear instructions

            /** Atomic modify and set bits for sip */
            static void set_bits(uint_xlen_t mask) {
                __asm__ volatile ("csrrs    zero, sip, %0"  
                                  : /* output: none */ 
                                  : "r" (mask)  /* input : register */
                                  : /* clobbers: none */);
            }
            /** Atomic read and then and set bits for sip */
            static uint32_t read_set_bits(uint_xlen_t mask) {
                uint_xlen_t value;
                __asm__ volatile ("csrrs    %0, sip, %1"  
                                  : "=r" (value) /* output: register %0 */
                                  : "r" (mask)  /* input : register */
                                  : /* clobbers: none */);
                return value;
            }
            /** Atomic modify and clear bits for sip */
            static void clr_bits(uint_xlen_t mask) {
                __asm__ volatile ("csrrc    zero, sip, %0"  
                                  : /* output: none */ 
                                  : "r" (mask)  /* input : register */
                                  : /* clobbers: none */);
            }
            /** Atomic read and then and clear bits for sip */
            static uint32_t read_clr_bits(uint_xlen_t mask) {
                uint_xlen_t value;
                __asm__ volatile ("csrrc    %0, sip, %1"  
                                  : "=r" (value) /* output: register %0 */
                                  : "r" (mask)  /* input : register */
                                  : /* clobbers: none */);
                return value;
            }
        
            // ------------------------------------------
            // Immediate value CSR bit set and clear instructions (only up to 5 bits)
        
            /** Atomic modify and set bits from immediate for sip */
            static void set_bits_imm(const uint8_t mask) {
                __asm__ volatile ("csrrsi    zero, sip, %0"  
                                  : /* output: none */ 
                                  : "i" (mask)  /* input : register */
                                  : /* clobbers: none */);
            }
            /** Atomic read and then and set bits from immediate for sip */
            static uint_xlen_t read_set_bits_imm(const uint8_t mask) {
                uint_xlen_t value;
                __asm__ volatile ("csrrsi    %0, sip, %1"  
                                  : "=r" (value) /* output: register %0 */
                                  : "i" (mask)  /* input : register */
                                  : /* clobbers: none */);
                return value;
            }
            /** Atomic modify and clear bits from immediate for sip */
            static void clr_bits_imm(const uint8_t mask) {
                __asm__ volatile ("csrrci    zero, sip, %0"  
                                  : /* output: none */ 
                                  : "i" (mask)  /* input : register */
                                  : /* clobbers: none */);
            }
            /** Atomic read and then and clear bits from immediate for sip */
            static uint_xlen_t read_clr_bits_imm(const uint8_t mask) {
                uint_xlen_t value;
                __asm__ volatile ("csrrci    %0, sip, %1"  
                                  : "=r" (value) /* output: register %0 */
                                  : "i" (mask)  /* input : register */
                                  : /* clobbers: none */);
                return value;
            }
            
        }; /* sip_ops */
        /** Parameter data for fields in sip */
        namespace sip_data {
            /** Parameter data for ssi */
            struct ssi {
                using datatype = uint_xlen_t;
                static constexpr uint_xlen_t BIT_OFFSET = 1;
                static constexpr uint_xlen_t BIT_WIDTH  = 1;
                static constexpr uint_xlen_t BIT_MASK   = 0x2;
                static constexpr uint_xlen_t ALL_SET_MASK = 0x1;
            };
            /** Parameter data for sti */
            struct sti {
                using datatype = uint_xlen_t;
                static constexpr uint_xlen_t BIT_OFFSET = 5;
                static constexpr uint_xlen_t BIT_WIDTH  = 1;
                static constexpr uint_xlen_t BIT_MASK   = 0x20;
                static constexpr uint_xlen_t ALL_SET_MASK = 0x1;
            };
            /** Parameter data for sei */
            struct sei {
                using datatype = uint_xlen_t;
                static constexpr uint_xlen_t BIT_OFFSET = 9;
                static constexpr uint_xlen_t BIT_WIDTH  = 1;
                static constexpr uint_xlen_t BIT_MASK   = 0x200;
                static constexpr uint_xlen_t ALL_SET_MASK = 0x1;
            };
            /** Parameter data for usi */
            struct usi {
                using datatype = uint_xlen_t;
                static constexpr uint_xlen_t BIT_OFFSET = 0;
                static constexpr uint_xlen_t BIT_WIDTH  = 1;
                static constexpr uint_xlen_t BIT_MASK   = 0x1;
                static constexpr uint_xlen_t ALL_SET_MASK = 0x1;
            };
            /** Parameter data for uti */
            struct uti {
                using datatype = uint_xlen_t;
                static constexpr uint_xlen_t BIT_OFFSET = 4;
                static constexpr uint_xlen_t BIT_WIDTH  = 1;
                static constexpr uint_xlen_t BIT_MASK   = 0x10;
                static constexpr uint_xlen_t ALL_SET_MASK = 0x1;
            };
            /** Parameter data for uei */
            struct uei {
                using datatype = uint_xlen_t;
                static constexpr uint_xlen_t BIT_OFFSET = 8;
                static constexpr uint_xlen_t BIT_WIDTH  = 1;
                static constexpr uint_xlen_t BIT_MASK   = 0x100;
                static constexpr uint_xlen_t ALL_SET_MASK = 0x1;
            };
        } /* sip_data */

        // ----------------------------------------------------------------
        // sie - SRW - Supervisor Interrupt Enable 
        //
        /** Supervisor Interrupt Enable assembler operations */
        struct sie_ops  {
            using datatype = uint_xlen_t;
            static constexpr priv_t priv = SRW; 
            
            /** Read sie */
            static uint_xlen_t read(void) {
                uint_xlen_t value;        
                __asm__ volatile ("csrr    %0, sie" 
                                  : "=r" (value)  /* output : register */
                                  : /* input : none */
                                  : /* clobbers: none */);
                return value;
            }
            
            
            /** Write sie */
            static void write(uint_xlen_t value) {
                __asm__ volatile ("csrw    sie, %0" 
                                  : /* output: none */ 
                                  : "r" (value) /* input : from register */
                                  : /* clobbers: none */);
            }
            /** Write immediate value to sie */
            static void write_imm(uint_xlen_t value) {
                __asm__ volatile ("csrwi    sie, %0" 
                                  : /* output: none */ 
                                  : "i" (value) /* input : from immediate */
                                  : /* clobbers: none */);
            }
            /** Read and then write to sie */
            static uint_xlen_t read_write(uint_xlen_t new_value) {
                uint_xlen_t prev_value;
                __asm__ volatile ("csrrw    %0, sie, %1"  
                                  : "=r" (prev_value) /* output: register %0 */
                                  : "r" (new_value)  /* input : register */
                                  : /* clobbers: none */);
                return prev_value;
            }
            /** Read and then write immediate value to sie */
            static uint_xlen_t read_write_imm(const uint8_t new_value) {
                uint_xlen_t prev_value;
                __asm__ volatile ("csrrwi    %0, sie, %1"  
                                  : "=r" (prev_value) /* output: register %0 */
                                  : "i" (new_value)  /* input : register */
                                  : /* clobbers: none */);
                return prev_value;
            }
        
            // ------------------------------------------
            // Register CSR bit set and clear instructions

            /** Atomic modify and set bits for sie */
            static void set_bits(uint_xlen_t mask) {
                __asm__ volatile ("csrrs    zero, sie, %0"  
                                  : /* output: none */ 
                                  : "r" (mask)  /* input : register */
                                  : /* clobbers: none */);
            }
            /** Atomic read and then and set bits for sie */
            static uint32_t read_set_bits(uint_xlen_t mask) {
                uint_xlen_t value;
                __asm__ volatile ("csrrs    %0, sie, %1"  
                                  : "=r" (value) /* output: register %0 */
                                  : "r" (mask)  /* input : register */
                                  : /* clobbers: none */);
                return value;
            }
            /** Atomic modify and clear bits for sie */
            static void clr_bits(uint_xlen_t mask) {
                __asm__ volatile ("csrrc    zero, sie, %0"  
                                  : /* output: none */ 
                                  : "r" (mask)  /* input : register */
                                  : /* clobbers: none */);
            }
            /** Atomic read and then and clear bits for sie */
            static uint32_t read_clr_bits(uint_xlen_t mask) {
                uint_xlen_t value;
                __asm__ volatile ("csrrc    %0, sie, %1"  
                                  : "=r" (value) /* output: register %0 */
                                  : "r" (mask)  /* input : register */
                                  : /* clobbers: none */);
                return value;
            }
        
            // ------------------------------------------
            // Immediate value CSR bit set and clear instructions (only up to 5 bits)
        
            /** Atomic modify and set bits from immediate for sie */
            static void set_bits_imm(const uint8_t mask) {
                __asm__ volatile ("csrrsi    zero, sie, %0"  
                                  : /* output: none */ 
                                  : "i" (mask)  /* input : register */
                                  : /* clobbers: none */);
            }
            /** Atomic read and then and set bits from immediate for sie */
            static uint_xlen_t read_set_bits_imm(const uint8_t mask) {
                uint_xlen_t value;
                __asm__ volatile ("csrrsi    %0, sie, %1"  
                                  : "=r" (value) /* output: register %0 */
                                  : "i" (mask)  /* input : register */
                                  : /* clobbers: none */);
                return value;
            }
            /** Atomic modify and clear bits from immediate for sie */
            static void clr_bits_imm(const uint8_t mask) {
                __asm__ volatile ("csrrci    zero, sie, %0"  
                                  : /* output: none */ 
                                  : "i" (mask)  /* input : register */
                                  : /* clobbers: none */);
            }
            /** Atomic read and then and clear bits from immediate for sie */
            static uint_xlen_t read_clr_bits_imm(const uint8_t mask) {
                uint_xlen_t value;
                __asm__ volatile ("csrrci    %0, sie, %1"  
                                  : "=r" (value) /* output: register %0 */
                                  : "i" (mask)  /* input : register */
                                  : /* clobbers: none */);
                return value;
            }
            
        }; /* sie_ops */
        /** Parameter data for fields in sie */
        namespace sie_data {
            /** Parameter data for ssi */
            struct ssi {
                using datatype = uint_xlen_t;
                static constexpr uint_xlen_t BIT_OFFSET = 1;
                static constexpr uint_xlen_t BIT_WIDTH  = 1;
                static constexpr uint_xlen_t BIT_MASK   = 0x2;
                static constexpr uint_xlen_t ALL_SET_MASK = 0x1;
            };
            /** Parameter data for sti */
            struct sti {
                using datatype = uint_xlen_t;
                static constexpr uint_xlen_t BIT_OFFSET = 5;
                static constexpr uint_xlen_t BIT_WIDTH  = 1;
                static constexpr uint_xlen_t BIT_MASK   = 0x20;
                static constexpr uint_xlen_t ALL_SET_MASK = 0x1;
            };
            /** Parameter data for sei */
            struct sei {
                using datatype = uint_xlen_t;
                static constexpr uint_xlen_t BIT_OFFSET = 9;
                static constexpr uint_xlen_t BIT_WIDTH  = 1;
                static constexpr uint_xlen_t BIT_MASK   = 0x200;
                static constexpr uint_xlen_t ALL_SET_MASK = 0x1;
            };
            /** Parameter data for usi */
            struct usi {
                using datatype = uint_xlen_t;
                static constexpr uint_xlen_t BIT_OFFSET = 0;
                static constexpr uint_xlen_t BIT_WIDTH  = 1;
                static constexpr uint_xlen_t BIT_MASK   = 0x1;
                static constexpr uint_xlen_t ALL_SET_MASK = 0x1;
            };
            /** Parameter data for uti */
            struct uti {
                using datatype = uint_xlen_t;
                static constexpr uint_xlen_t BIT_OFFSET = 4;
                static constexpr uint_xlen_t BIT_WIDTH  = 1;
                static constexpr uint_xlen_t BIT_MASK   = 0x10;
                static constexpr uint_xlen_t ALL_SET_MASK = 0x1;
            };
            /** Parameter data for uei */
            struct uei {
                using datatype = uint_xlen_t;
                static constexpr uint_xlen_t BIT_OFFSET = 8;
                static constexpr uint_xlen_t BIT_WIDTH  = 1;
                static constexpr uint_xlen_t BIT_MASK   = 0x100;
                static constexpr uint_xlen_t ALL_SET_MASK = 0x1;
            };
        } /* sie_data */

        // ----------------------------------------------------------------
        // fflags - URW - Floating-Point Accrued Exceptions. 
        //
        /** Floating-Point Accrued Exceptions. assembler operations */
        struct fflags_ops  {
            using datatype = uint_xlen_t;
            static constexpr priv_t priv = URW; 
            
            /** Read fflags */
            static uint_xlen_t read(void) {
                uint_xlen_t value;        
                __asm__ volatile ("csrr    %0, fflags" 
                                  : "=r" (value)  /* output : register */
                                  : /* input : none */
                                  : /* clobbers: none */);
                return value;
            }
            
            
            /** Write fflags */
            static void write(uint_xlen_t value) {
                __asm__ volatile ("csrw    fflags, %0" 
                                  : /* output: none */ 
                                  : "r" (value) /* input : from register */
                                  : /* clobbers: none */);
            }
            /** Write immediate value to fflags */
            static void write_imm(uint_xlen_t value) {
                __asm__ volatile ("csrwi    fflags, %0" 
                                  : /* output: none */ 
                                  : "i" (value) /* input : from immediate */
                                  : /* clobbers: none */);
            }
            /** Read and then write to fflags */
            static uint_xlen_t read_write(uint_xlen_t new_value) {
                uint_xlen_t prev_value;
                __asm__ volatile ("csrrw    %0, fflags, %1"  
                                  : "=r" (prev_value) /* output: register %0 */
                                  : "r" (new_value)  /* input : register */
                                  : /* clobbers: none */);
                return prev_value;
            }
            /** Read and then write immediate value to fflags */
            static uint_xlen_t read_write_imm(const uint8_t new_value) {
                uint_xlen_t prev_value;
                __asm__ volatile ("csrrwi    %0, fflags, %1"  
                                  : "=r" (prev_value) /* output: register %0 */
                                  : "i" (new_value)  /* input : register */
                                  : /* clobbers: none */);
                return prev_value;
            }
        
            // ------------------------------------------
            // Register CSR bit set and clear instructions

            /** Atomic modify and set bits for fflags */
            static void set_bits(uint_xlen_t mask) {
                __asm__ volatile ("csrrs    zero, fflags, %0"  
                                  : /* output: none */ 
                                  : "r" (mask)  /* input : register */
                                  : /* clobbers: none */);
            }
            /** Atomic read and then and set bits for fflags */
            static uint32_t read_set_bits(uint_xlen_t mask) {
                uint_xlen_t value;
                __asm__ volatile ("csrrs    %0, fflags, %1"  
                                  : "=r" (value) /* output: register %0 */
                                  : "r" (mask)  /* input : register */
                                  : /* clobbers: none */);
                return value;
            }
            /** Atomic modify and clear bits for fflags */
            static void clr_bits(uint_xlen_t mask) {
                __asm__ volatile ("csrrc    zero, fflags, %0"  
                                  : /* output: none */ 
                                  : "r" (mask)  /* input : register */
                                  : /* clobbers: none */);
            }
            /** Atomic read and then and clear bits for fflags */
            static uint32_t read_clr_bits(uint_xlen_t mask) {
                uint_xlen_t value;
                __asm__ volatile ("csrrc    %0, fflags, %1"  
                                  : "=r" (value) /* output: register %0 */
                                  : "r" (mask)  /* input : register */
                                  : /* clobbers: none */);
                return value;
            }
        
            // ------------------------------------------
            // Immediate value CSR bit set and clear instructions (only up to 5 bits)
        
            /** Atomic modify and set bits from immediate for fflags */
            static void set_bits_imm(const uint8_t mask) {
                __asm__ volatile ("csrrsi    zero, fflags, %0"  
                                  : /* output: none */ 
                                  : "i" (mask)  /* input : register */
                                  : /* clobbers: none */);
            }
            /** Atomic read and then and set bits from immediate for fflags */
            static uint_xlen_t read_set_bits_imm(const uint8_t mask) {
                uint_xlen_t value;
                __asm__ volatile ("csrrsi    %0, fflags, %1"  
                                  : "=r" (value) /* output: register %0 */
                                  : "i" (mask)  /* input : register */
                                  : /* clobbers: none */);
                return value;
            }
            /** Atomic modify and clear bits from immediate for fflags */
            static void clr_bits_imm(const uint8_t mask) {
                __asm__ volatile ("csrrci    zero, fflags, %0"  
                                  : /* output: none */ 
                                  : "i" (mask)  /* input : register */
                                  : /* clobbers: none */);
            }
            /** Atomic read and then and clear bits from immediate for fflags */
            static uint_xlen_t read_clr_bits_imm(const uint8_t mask) {
                uint_xlen_t value;
                __asm__ volatile ("csrrci    %0, fflags, %1"  
                                  : "=r" (value) /* output: register %0 */
                                  : "i" (mask)  /* input : register */
                                  : /* clobbers: none */);
                return value;
            }
            
        }; /* fflags_ops */

        // ----------------------------------------------------------------
        // frm - URW - Floating-Point Dynamic Rounding Mode. 
        //
        /** Floating-Point Dynamic Rounding Mode. assembler operations */
        struct frm_ops  {
            using datatype = uint_xlen_t;
            static constexpr priv_t priv = URW; 
            
            /** Read frm */
            static uint_xlen_t read(void) {
                uint_xlen_t value;        
                __asm__ volatile ("csrr    %0, frm" 
                                  : "=r" (value)  /* output : register */
                                  : /* input : none */
                                  : /* clobbers: none */);
                return value;
            }
            
            
            /** Write frm */
            static void write(uint_xlen_t value) {
                __asm__ volatile ("csrw    frm, %0" 
                                  : /* output: none */ 
                                  : "r" (value) /* input : from register */
                                  : /* clobbers: none */);
            }
            /** Write immediate value to frm */
            static void write_imm(uint_xlen_t value) {
                __asm__ volatile ("csrwi    frm, %0" 
                                  : /* output: none */ 
                                  : "i" (value) /* input : from immediate */
                                  : /* clobbers: none */);
            }
            /** Read and then write to frm */
            static uint_xlen_t read_write(uint_xlen_t new_value) {
                uint_xlen_t prev_value;
                __asm__ volatile ("csrrw    %0, frm, %1"  
                                  : "=r" (prev_value) /* output: register %0 */
                                  : "r" (new_value)  /* input : register */
                                  : /* clobbers: none */);
                return prev_value;
            }
            /** Read and then write immediate value to frm */
            static uint_xlen_t read_write_imm(const uint8_t new_value) {
                uint_xlen_t prev_value;
                __asm__ volatile ("csrrwi    %0, frm, %1"  
                                  : "=r" (prev_value) /* output: register %0 */
                                  : "i" (new_value)  /* input : register */
                                  : /* clobbers: none */);
                return prev_value;
            }
        
            // ------------------------------------------
            // Register CSR bit set and clear instructions

            /** Atomic modify and set bits for frm */
            static void set_bits(uint_xlen_t mask) {
                __asm__ volatile ("csrrs    zero, frm, %0"  
                                  : /* output: none */ 
                                  : "r" (mask)  /* input : register */
                                  : /* clobbers: none */);
            }
            /** Atomic read and then and set bits for frm */
            static uint32_t read_set_bits(uint_xlen_t mask) {
                uint_xlen_t value;
                __asm__ volatile ("csrrs    %0, frm, %1"  
                                  : "=r" (value) /* output: register %0 */
                                  : "r" (mask)  /* input : register */
                                  : /* clobbers: none */);
                return value;
            }
            /** Atomic modify and clear bits for frm */
            static void clr_bits(uint_xlen_t mask) {
                __asm__ volatile ("csrrc    zero, frm, %0"  
                                  : /* output: none */ 
                                  : "r" (mask)  /* input : register */
                                  : /* clobbers: none */);
            }
            /** Atomic read and then and clear bits for frm */
            static uint32_t read_clr_bits(uint_xlen_t mask) {
                uint_xlen_t value;
                __asm__ volatile ("csrrc    %0, frm, %1"  
                                  : "=r" (value) /* output: register %0 */
                                  : "r" (mask)  /* input : register */
                                  : /* clobbers: none */);
                return value;
            }
        
            // ------------------------------------------
            // Immediate value CSR bit set and clear instructions (only up to 5 bits)
        
            /** Atomic modify and set bits from immediate for frm */
            static void set_bits_imm(const uint8_t mask) {
                __asm__ volatile ("csrrsi    zero, frm, %0"  
                                  : /* output: none */ 
                                  : "i" (mask)  /* input : register */
                                  : /* clobbers: none */);
            }
            /** Atomic read and then and set bits from immediate for frm */
            static uint_xlen_t read_set_bits_imm(const uint8_t mask) {
                uint_xlen_t value;
                __asm__ volatile ("csrrsi    %0, frm, %1"  
                                  : "=r" (value) /* output: register %0 */
                                  : "i" (mask)  /* input : register */
                                  : /* clobbers: none */);
                return value;
            }
            /** Atomic modify and clear bits from immediate for frm */
            static void clr_bits_imm(const uint8_t mask) {
                __asm__ volatile ("csrrci    zero, frm, %0"  
                                  : /* output: none */ 
                                  : "i" (mask)  /* input : register */
                                  : /* clobbers: none */);
            }
            /** Atomic read and then and clear bits from immediate for frm */
            static uint_xlen_t read_clr_bits_imm(const uint8_t mask) {
                uint_xlen_t value;
                __asm__ volatile ("csrrci    %0, frm, %1"  
                                  : "=r" (value) /* output: register %0 */
                                  : "i" (mask)  /* input : register */
                                  : /* clobbers: none */);
                return value;
            }
            
        }; /* frm_ops */

        // ----------------------------------------------------------------
        // fcsr - URW - Floating-Point Control and Status 
        //
        /** Floating-Point Control and Status assembler operations */
        struct fcsr_ops  {
            using datatype = uint_xlen_t;
            static constexpr priv_t priv = URW; 
            
            /** Read fcsr */
            static uint_xlen_t read(void) {
                uint_xlen_t value;        
                __asm__ volatile ("csrr    %0, fcsr" 
                                  : "=r" (value)  /* output : register */
                                  : /* input : none */
                                  : /* clobbers: none */);
                return value;
            }
            
            
            /** Write fcsr */
            static void write(uint_xlen_t value) {
                __asm__ volatile ("csrw    fcsr, %0" 
                                  : /* output: none */ 
                                  : "r" (value) /* input : from register */
                                  : /* clobbers: none */);
            }
            /** Write immediate value to fcsr */
            static void write_imm(uint_xlen_t value) {
                __asm__ volatile ("csrwi    fcsr, %0" 
                                  : /* output: none */ 
                                  : "i" (value) /* input : from immediate */
                                  : /* clobbers: none */);
            }
            /** Read and then write to fcsr */
            static uint_xlen_t read_write(uint_xlen_t new_value) {
                uint_xlen_t prev_value;
                __asm__ volatile ("csrrw    %0, fcsr, %1"  
                                  : "=r" (prev_value) /* output: register %0 */
                                  : "r" (new_value)  /* input : register */
                                  : /* clobbers: none */);
                return prev_value;
            }
            /** Read and then write immediate value to fcsr */
            static uint_xlen_t read_write_imm(const uint8_t new_value) {
                uint_xlen_t prev_value;
                __asm__ volatile ("csrrwi    %0, fcsr, %1"  
                                  : "=r" (prev_value) /* output: register %0 */
                                  : "i" (new_value)  /* input : register */
                                  : /* clobbers: none */);
                return prev_value;
            }
        
            // ------------------------------------------
            // Register CSR bit set and clear instructions

            /** Atomic modify and set bits for fcsr */
            static void set_bits(uint_xlen_t mask) {
                __asm__ volatile ("csrrs    zero, fcsr, %0"  
                                  : /* output: none */ 
                                  : "r" (mask)  /* input : register */
                                  : /* clobbers: none */);
            }
            /** Atomic read and then and set bits for fcsr */
            static uint32_t read_set_bits(uint_xlen_t mask) {
                uint_xlen_t value;
                __asm__ volatile ("csrrs    %0, fcsr, %1"  
                                  : "=r" (value) /* output: register %0 */
                                  : "r" (mask)  /* input : register */
                                  : /* clobbers: none */);
                return value;
            }
            /** Atomic modify and clear bits for fcsr */
            static void clr_bits(uint_xlen_t mask) {
                __asm__ volatile ("csrrc    zero, fcsr, %0"  
                                  : /* output: none */ 
                                  : "r" (mask)  /* input : register */
                                  : /* clobbers: none */);
            }
            /** Atomic read and then and clear bits for fcsr */
            static uint32_t read_clr_bits(uint_xlen_t mask) {
                uint_xlen_t value;
                __asm__ volatile ("csrrc    %0, fcsr, %1"  
                                  : "=r" (value) /* output: register %0 */
                                  : "r" (mask)  /* input : register */
                                  : /* clobbers: none */);
                return value;
            }
        
            // ------------------------------------------
            // Immediate value CSR bit set and clear instructions (only up to 5 bits)
        
            /** Atomic modify and set bits from immediate for fcsr */
            static void set_bits_imm(const uint8_t mask) {
                __asm__ volatile ("csrrsi    zero, fcsr, %0"  
                                  : /* output: none */ 
                                  : "i" (mask)  /* input : register */
                                  : /* clobbers: none */);
            }
            /** Atomic read and then and set bits from immediate for fcsr */
            static uint_xlen_t read_set_bits_imm(const uint8_t mask) {
                uint_xlen_t value;
                __asm__ volatile ("csrrsi    %0, fcsr, %1"  
                                  : "=r" (value) /* output: register %0 */
                                  : "i" (mask)  /* input : register */
                                  : /* clobbers: none */);
                return value;
            }
            /** Atomic modify and clear bits from immediate for fcsr */
            static void clr_bits_imm(const uint8_t mask) {
                __asm__ volatile ("csrrci    zero, fcsr, %0"  
                                  : /* output: none */ 
                                  : "i" (mask)  /* input : register */
                                  : /* clobbers: none */);
            }
            /** Atomic read and then and clear bits from immediate for fcsr */
            static uint_xlen_t read_clr_bits_imm(const uint8_t mask) {
                uint_xlen_t value;
                __asm__ volatile ("csrrci    %0, fcsr, %1"  
                                  : "=r" (value) /* output: register %0 */
                                  : "i" (mask)  /* input : register */
                                  : /* clobbers: none */);
                return value;
            }
            
        }; /* fcsr_ops */

        // ----------------------------------------------------------------
        // cycle - URO - Cycle counter for RDCYCLE instruction. 
        //
        /** Cycle counter for RDCYCLE instruction. assembler operations */
        struct cycle_ops  {
            using datatype = uint_xlen_t;
            static constexpr priv_t priv = URO; 
            
            /** Read cycle */
            static uint_xlen_t read(void) {
                uint_xlen_t value;        
                __asm__ volatile ("csrr    %0, cycle" 
                                  : "=r" (value)  /* output : register */
                                  : /* input : none */
                                  : /* clobbers: none */);
                return value;
            }
            
            
        }; /* cycle_ops */

        // ----------------------------------------------------------------
        // time - URO - Timer for RDTIME instruction. 
        //
        /** Timer for RDTIME instruction. assembler operations */
        struct time_ops  {
            using datatype = uint_xlen_t;
            static constexpr priv_t priv = URO; 
            
            /** Read time */
            static uint_xlen_t read(void) {
                uint_xlen_t value;        
                __asm__ volatile ("csrr    %0, time" 
                                  : "=r" (value)  /* output : register */
                                  : /* input : none */
                                  : /* clobbers: none */);
                return value;
            }
            
            
        }; /* time_ops */

        // ----------------------------------------------------------------
        // instret - URO - Instructions-retired counter for RDINSTRET instruction. 
        //
        /** Instructions-retired counter for RDINSTRET instruction. assembler operations */
        struct instret_ops  {
            using datatype = uint_xlen_t;
            static constexpr priv_t priv = URO; 
            
            /** Read instret */
            static uint_xlen_t read(void) {
                uint_xlen_t value;        
                __asm__ volatile ("csrr    %0, instret" 
                                  : "=r" (value)  /* output : register */
                                  : /* input : none */
                                  : /* clobbers: none */);
                return value;
            }
            
            
        }; /* instret_ops */

        // ----------------------------------------------------------------
        // stval - SRW - Supervisor bad address or instruction. 
        //
        /** Supervisor bad address or instruction. assembler operations */
        struct stval_ops  {
            using datatype = uint_xlen_t;
            static constexpr priv_t priv = SRW; 
            
            /** Read stval */
            static uint_xlen_t read(void) {
                uint_xlen_t value;        
                __asm__ volatile ("csrr    %0, stval" 
                                  : "=r" (value)  /* output : register */
                                  : /* input : none */
                                  : /* clobbers: none */);
                return value;
            }
            
            
            /** Write stval */
            static void write(uint_xlen_t value) {
                __asm__ volatile ("csrw    stval, %0" 
                                  : /* output: none */ 
                                  : "r" (value) /* input : from register */
                                  : /* clobbers: none */);
            }
            /** Write immediate value to stval */
            static void write_imm(uint_xlen_t value) {
                __asm__ volatile ("csrwi    stval, %0" 
                                  : /* output: none */ 
                                  : "i" (value) /* input : from immediate */
                                  : /* clobbers: none */);
            }
            /** Read and then write to stval */
            static uint_xlen_t read_write(uint_xlen_t new_value) {
                uint_xlen_t prev_value;
                __asm__ volatile ("csrrw    %0, stval, %1"  
                                  : "=r" (prev_value) /* output: register %0 */
                                  : "r" (new_value)  /* input : register */
                                  : /* clobbers: none */);
                return prev_value;
            }
            /** Read and then write immediate value to stval */
            static uint_xlen_t read_write_imm(const uint8_t new_value) {
                uint_xlen_t prev_value;
                __asm__ volatile ("csrrwi    %0, stval, %1"  
                                  : "=r" (prev_value) /* output: register %0 */
                                  : "i" (new_value)  /* input : register */
                                  : /* clobbers: none */);
                return prev_value;
            }
        
            // ------------------------------------------
            // Register CSR bit set and clear instructions

            /** Atomic modify and set bits for stval */
            static void set_bits(uint_xlen_t mask) {
                __asm__ volatile ("csrrs    zero, stval, %0"  
                                  : /* output: none */ 
                                  : "r" (mask)  /* input : register */
                                  : /* clobbers: none */);
            }
            /** Atomic read and then and set bits for stval */
            static uint32_t read_set_bits(uint_xlen_t mask) {
                uint_xlen_t value;
                __asm__ volatile ("csrrs    %0, stval, %1"  
                                  : "=r" (value) /* output: register %0 */
                                  : "r" (mask)  /* input : register */
                                  : /* clobbers: none */);
                return value;
            }
            /** Atomic modify and clear bits for stval */
            static void clr_bits(uint_xlen_t mask) {
                __asm__ volatile ("csrrc    zero, stval, %0"  
                                  : /* output: none */ 
                                  : "r" (mask)  /* input : register */
                                  : /* clobbers: none */);
            }
            /** Atomic read and then and clear bits for stval */
            static uint32_t read_clr_bits(uint_xlen_t mask) {
                uint_xlen_t value;
                __asm__ volatile ("csrrc    %0, stval, %1"  
                                  : "=r" (value) /* output: register %0 */
                                  : "r" (mask)  /* input : register */
                                  : /* clobbers: none */);
                return value;
            }
        
            // ------------------------------------------
            // Immediate value CSR bit set and clear instructions (only up to 5 bits)
        
            /** Atomic modify and set bits from immediate for stval */
            static void set_bits_imm(const uint8_t mask) {
                __asm__ volatile ("csrrsi    zero, stval, %0"  
                                  : /* output: none */ 
                                  : "i" (mask)  /* input : register */
                                  : /* clobbers: none */);
            }
            /** Atomic read and then and set bits from immediate for stval */
            static uint_xlen_t read_set_bits_imm(const uint8_t mask) {
                uint_xlen_t value;
                __asm__ volatile ("csrrsi    %0, stval, %1"  
                                  : "=r" (value) /* output: register %0 */
                                  : "i" (mask)  /* input : register */
                                  : /* clobbers: none */);
                return value;
            }
            /** Atomic modify and clear bits from immediate for stval */
            static void clr_bits_imm(const uint8_t mask) {
                __asm__ volatile ("csrrci    zero, stval, %0"  
                                  : /* output: none */ 
                                  : "i" (mask)  /* input : register */
                                  : /* clobbers: none */);
            }
            /** Atomic read and then and clear bits from immediate for stval */
            static uint_xlen_t read_clr_bits_imm(const uint8_t mask) {
                uint_xlen_t value;
                __asm__ volatile ("csrrci    %0, stval, %1"  
                                  : "=r" (value) /* output: register %0 */
                                  : "i" (mask)  /* input : register */
                                  : /* clobbers: none */);
                return value;
            }
            
        }; /* stval_ops */

        // ----------------------------------------------------------------
        // satp - SRW - Supervisor address translation and protection. 
        //
        /** Supervisor address translation and protection. assembler operations */
        struct satp_ops  {
            using datatype = uint_xlen_t;
            static constexpr priv_t priv = SRW; 
            
            /** Read satp */
            static uint_xlen_t read(void) {
                uint_xlen_t value;        
                __asm__ volatile ("csrr    %0, satp" 
                                  : "=r" (value)  /* output : register */
                                  : /* input : none */
                                  : /* clobbers: none */);
                return value;
            }
            
            
            /** Write satp */
            static void write(uint_xlen_t value) {
                __asm__ volatile ("csrw    satp, %0" 
                                  : /* output: none */ 
                                  : "r" (value) /* input : from register */
                                  : /* clobbers: none */);
            }
            /** Write immediate value to satp */
            static void write_imm(uint_xlen_t value) {
                __asm__ volatile ("csrwi    satp, %0" 
                                  : /* output: none */ 
                                  : "i" (value) /* input : from immediate */
                                  : /* clobbers: none */);
            }
            /** Read and then write to satp */
            static uint_xlen_t read_write(uint_xlen_t new_value) {
                uint_xlen_t prev_value;
                __asm__ volatile ("csrrw    %0, satp, %1"  
                                  : "=r" (prev_value) /* output: register %0 */
                                  : "r" (new_value)  /* input : register */
                                  : /* clobbers: none */);
                return prev_value;
            }
            /** Read and then write immediate value to satp */
            static uint_xlen_t read_write_imm(const uint8_t new_value) {
                uint_xlen_t prev_value;
                __asm__ volatile ("csrrwi    %0, satp, %1"  
                                  : "=r" (prev_value) /* output: register %0 */
                                  : "i" (new_value)  /* input : register */
                                  : /* clobbers: none */);
                return prev_value;
            }
        
            // ------------------------------------------
            // Register CSR bit set and clear instructions

            /** Atomic modify and set bits for satp */
            static void set_bits(uint_xlen_t mask) {
                __asm__ volatile ("csrrs    zero, satp, %0"  
                                  : /* output: none */ 
                                  : "r" (mask)  /* input : register */
                                  : /* clobbers: none */);
            }
            /** Atomic read and then and set bits for satp */
            static uint32_t read_set_bits(uint_xlen_t mask) {
                uint_xlen_t value;
                __asm__ volatile ("csrrs    %0, satp, %1"  
                                  : "=r" (value) /* output: register %0 */
                                  : "r" (mask)  /* input : register */
                                  : /* clobbers: none */);
                return value;
            }
            /** Atomic modify and clear bits for satp */
            static void clr_bits(uint_xlen_t mask) {
                __asm__ volatile ("csrrc    zero, satp, %0"  
                                  : /* output: none */ 
                                  : "r" (mask)  /* input : register */
                                  : /* clobbers: none */);
            }
            /** Atomic read and then and clear bits for satp */
            static uint32_t read_clr_bits(uint_xlen_t mask) {
                uint_xlen_t value;
                __asm__ volatile ("csrrc    %0, satp, %1"  
                                  : "=r" (value) /* output: register %0 */
                                  : "r" (mask)  /* input : register */
                                  : /* clobbers: none */);
                return value;
            }
        
            // ------------------------------------------
            // Immediate value CSR bit set and clear instructions (only up to 5 bits)
        
            /** Atomic modify and set bits from immediate for satp */
            static void set_bits_imm(const uint8_t mask) {
                __asm__ volatile ("csrrsi    zero, satp, %0"  
                                  : /* output: none */ 
                                  : "i" (mask)  /* input : register */
                                  : /* clobbers: none */);
            }
            /** Atomic read and then and set bits from immediate for satp */
            static uint_xlen_t read_set_bits_imm(const uint8_t mask) {
                uint_xlen_t value;
                __asm__ volatile ("csrrsi    %0, satp, %1"  
                                  : "=r" (value) /* output: register %0 */
                                  : "i" (mask)  /* input : register */
                                  : /* clobbers: none */);
                return value;
            }
            /** Atomic modify and clear bits from immediate for satp */
            static void clr_bits_imm(const uint8_t mask) {
                __asm__ volatile ("csrrci    zero, satp, %0"  
                                  : /* output: none */ 
                                  : "i" (mask)  /* input : register */
                                  : /* clobbers: none */);
            }
            /** Atomic read and then and clear bits from immediate for satp */
            static uint_xlen_t read_clr_bits_imm(const uint8_t mask) {
                uint_xlen_t value;
                __asm__ volatile ("csrrci    %0, satp, %1"  
                                  : "=r" (value) /* output: register %0 */
                                  : "i" (mask)  /* input : register */
                                  : /* clobbers: none */);
                return value;
            }
            
        }; /* satp_ops */
    } /* csr */
} /* riscv */


// ------------------------------------------------------------------------
// Register and field interface classes.
namespace riscv {
    namespace csr {
        /* Supervisor Mode Scratch Register */
        template<class OPS> class sscratch_reg : public read_write_reg<OPS>
        {
        };
        using sscratch = sscratch_reg<riscv::csr::sscratch_ops>;
        /* Supervisor Exception Program Counter */
        template<class OPS> class sepc_reg : public read_write_reg<OPS>
        {
        };
        using sepc = sepc_reg<riscv::csr::sepc_ops>;
        /* Supervisor Exception Cause */
        template<class OPS> class scause_reg : public read_write_reg<OPS>
        {
            public:
                read_write_field<OPS, riscv::csr::scause_data::interrupt> interrupt;
                read_write_field<OPS, riscv::csr::scause_data::exception_code> exception_code;
        };
        using scause = scause_reg<riscv::csr::scause_ops>;
        /* Supervisor Status */
        template<class OPS> class sstatus_reg : public read_write_reg<OPS>
        {
            public:
                read_write_field<OPS, riscv::csr::sstatus_data::sie> sie;
                read_write_field<OPS, riscv::csr::sstatus_data::spie> spie;
                read_write_field<OPS, riscv::csr::sstatus_data::spp> spp;
        };
        using sstatus = sstatus_reg<riscv::csr::sstatus_ops>;
        /* Supervisor Trap Vector Base Address */
        template<class OPS> class stvec_reg : public read_write_reg<OPS>
        {
            public:
                read_write_field<OPS, riscv::csr::stvec_data::base> base;
                read_write_field<OPS, riscv::csr::stvec_data::mode> mode;
        };
        using stvec = stvec_reg<riscv::csr::stvec_ops>;
        /* Supervisor Interrupt Delegation */
        template<class OPS> class sideleg_reg : public read_write_reg<OPS>
        {
        };
        using sideleg = sideleg_reg<riscv::csr::sideleg_ops>;
        /* Supervisor Exception Delegation */
        template<class OPS> class sedeleg_reg : public read_write_reg<OPS>
        {
        };
        using sedeleg = sedeleg_reg<riscv::csr::sedeleg_ops>;
        /* Supervisor Interrupt Pending */
        template<class OPS> class sip_reg : public read_write_reg<OPS>
        {
            public:
                read_write_field<OPS, riscv::csr::sip_data::ssi> ssi;
                read_write_field<OPS, riscv::csr::sip_data::sti> sti;
                read_write_field<OPS, riscv::csr::sip_data::sei> sei;
                read_write_field<OPS, riscv::csr::sip_data::usi> usi;
                read_write_field<OPS, riscv::csr::sip_data::uti> uti;
                read_write_field<OPS, riscv::csr::sip_data::uei> uei;
        };
        using sip = sip_reg<riscv::csr::sip_ops>;
        /* Supervisor Interrupt Enable */
        template<class OPS> class sie_reg : public read_write_reg<OPS>
        {
            public:
                read_write_field<OPS, riscv::csr::sie_data::ssi> ssi;
                read_write_field<OPS, riscv::csr::sie_data::sti> sti;
                read_write_field<OPS, riscv::csr::sie_data::sei> sei;
                read_write_field<OPS, riscv::csr::sie_data::usi> usi;
                read_write_field<OPS, riscv::csr::sie_data::uti> uti;
                read_write_field<OPS, riscv::csr::sie_data::uei> uei;
        };
        using sie = sie_reg<riscv::csr::sie_ops>;

                /* Supervisor bad address or instruction. */
        template<class OPS> class stval_reg : public read_write_reg<OPS>
        {
        };
        using stval = stval_reg<riscv::csr::stval_ops>;
        /* Supervisor address translation and protection. */
        template<class OPS> class satp_reg : public read_write_reg<OPS>
        {
        };
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

    } /* csr */

    static csr::all csrs;

} /* riscv */

#endif // #define RISCV_CSR_HPP
