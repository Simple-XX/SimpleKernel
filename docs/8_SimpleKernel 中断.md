# SimpleKernel 中断

## IA32

TODO

## RISCV

寄存结构及中断上下文
```c++
namespace CPU {
/// 机器模式定义
enum {
    U_MODE = 0,
    S_MODE = 1,
    M_MODE = 3,
};

enum {
    INTR_SOFT = 0,
    /// U 态软中断
    INTR_SOFT_U = INTR_SOFT + U_MODE,
    /// S 态软中断
    INTR_SOFT_S = INTR_SOFT + S_MODE,
    /// M 态软中断
    INTR_SOFT_M = INTR_SOFT + M_MODE,
    INTR_TIMER  = 4,
    /// U 态时钟中断
    INTR_TIMER_U = INTR_TIMER + U_MODE,
    /// S 态时钟中断
    INTR_TIMER_S = INTR_TIMER + S_MODE,
    /// M 态时钟中断
    INTR_TIMER_M = INTR_TIMER + M_MODE,
    INTR_EXTERN  = 8,
    /// U 态外部中断
    INTR_EXTERN_U = INTR_EXTERN + U_MODE,
    /// S 态外部中断
    INTR_EXTERN_S = INTR_EXTERN + S_MODE,
    /// M 态外部中断
    INTR_EXTERN_M = INTR_EXTERN + M_MODE,
};

enum {
    EXCP_INSTRUCTION_ADDRESS_MISALIGNED = 0,
    EXCP_INSTRUCTION_ACCESS_FAULT       = 1,
    EXCP_ILLEGAL_INSTRUCTION            = 2,
    EXCP_BREAKPOINT                     = 3,
    EXCP_LOAD_ADDRESS_MISALIGNED        = 4,
    EXCP_LOAD_ACCESS_FAULT              = 5,
    EXCP_STORE_AMO_ADDRESS_MISALIGNED   = 6,
    EXCP_STORE_AMO_ACCESS_FAULT         = 7,
    EXCP_ECALL                          = 8,
    EXCP_ECALL_U                        = EXCP_ECALL + U_MODE,
    EXCP_ECALL_S                        = EXCP_ECALL + S_MODE,
    EXCP_ECALL_M                        = EXCP_ECALL + M_MODE,
    EXCP_INSTRUCTION_PAGE_FAULT         = 12,
    EXCP_LOAD_PAGE_FAULT                = 13,
    EXCP_STORE_AMO_PAGE_FAULT           = 15,
};

// Supervisor Status Register, sstatus
// User Interrupt Enable
static constexpr const uint64_t SSTATUS_UIE = 1 << 0;
// Supervisor Interrupt Enable
static constexpr const uint64_t SSTATUS_SIE = 1 << 1;
// User Previous Interrupt Enable
static constexpr const uint64_t SSTATUS_UPIE = 1 << 4;
// Supervisor Previous Interrupt Enable
static constexpr const uint64_t SSTATUS_SPIE = 1 << 5;
// Previous mode, 1=Supervisor, 0=User
static constexpr const uint64_t SSTATUS_SPP = 1 << 8;

/**
 * @brief sstatus 寄存器定义
 */
struct sstatus_t {
    union {
        struct {
            // Reserved Writes Preserve Values, Reads Ignore Values (WPRI)
            uint64_t wpri1 : 1;
            // interrupt enable
            uint64_t sie : 1;
            uint64_t wpri12 : 3;
            // previous interrupt enable
            uint64_t spie : 1;
            uint64_t ube : 1;
            uint64_t wpri3 : 1;
            // previous mode (supervisor)
            uint64_t spp : 1;
            uint64_t wpri4 : 4;
            // FPU status
            uint64_t fs : 2;
            // extensions status
            uint64_t xs : 2;
            uint64_t wpri5 : 1;
            // permit supervisor user memory access
            uint64_t sum : 1;
            // make executable readable
            uint64_t mxr : 1;
            uint64_t wpri6 : 12;
            // U-mode XLEN
            uint64_t uxl : 2;
            uint64_t wpri7 : 29;
            // status dirty
            uint64_t sd : 1;
        };
        uint64_t val;
    };

    sstatus_t(void) {
        val = 0;
        return;
    }
    sstatus_t(uint64_t _val) : val(_val) {
        return;
    }
    friend std::ostream &operator<<(std::ostream    &_os,
                                    const sstatus_t &_sstatus) {
        printf("val: 0x%p, sie: %s, spie: %s, spp: %s", _sstatus.val,
               (_sstatus.sie == true ? "enable" : "disable"),
               (_sstatus.spie == true ? "enable" : "disable"),
               (_sstatus.spp == true ? "S mode" : "U mode"));
        return _os;
    }
};

// Supervisor Interrupt Enable
// software
static constexpr const uint64_t SIE_SSIE = 1 << 1;
// timer
static constexpr const uint64_t SIE_STIE = 1 << 5;
// external
static constexpr const uint64_t SIE_SEIE = 1 << 9;

/// 中断模式 直接
static constexpr const uint64_t TVEC_DIRECT = 0xFFFFFFFFFFFFFFFC;
/// 中断模式 向量
static constexpr const uint64_t TVEC_VECTORED = 0xFFFFFFFFFFFFFFFD;

/**
 * @brief 允许中断
 */
static inline void ENABLE_INTR(void) {
    WRITE_SSTATUS(READ_SSTATUS().val | SSTATUS_SIE);
    return;
}

/**
 * @brief 允许中断
 * @param  _sstatus         要设置的 sstatus
 */
static inline void ENABLE_INTR(sstatus_t &_sstatus) {
    _sstatus.sie = true;
    return;
}

/**
 * @brief 禁止中断
 */
static inline void DISABLE_INTR(void) {
    WRITE_SSTATUS(READ_SSTATUS().val & ~SSTATUS_SIE);
    return;
}

/**
 * @brief 禁止中断
 * @param  _sstatus         要设置的原 sstatus 值
 */
static inline void DISABLE_INTR(sstatus_t &_sstatus) {
    _sstatus.sie = false;
    return;
}

/**
 * @brief 读取中断状态
 * @return true             允许
 * @return false            禁止
 */
static inline bool STATUS_INTR(void) {
    sstatus_t x = READ_SSTATUS();
    return x.sie;
}

/**
 * @brief 通用寄存器
 */
struct xregs_t {
    uintptr_t            zero;
    uintptr_t            ra;
    uintptr_t            sp;
    uintptr_t            gp;
    uintptr_t            tp;
    uintptr_t            t0;
    uintptr_t            t1;
    uintptr_t            t2;
    uintptr_t            s0;
    uintptr_t            s1;
    uintptr_t            a0;
    uintptr_t            a1;
    uintptr_t            a2;
    uintptr_t            a3;
    uintptr_t            a4;
    uintptr_t            a5;
    uintptr_t            a6;
    uintptr_t            a7;
    uintptr_t            s2;
    uintptr_t            s3;
    uintptr_t            s4;
    uintptr_t            s5;
    uintptr_t            s6;
    uintptr_t            s7;
    uintptr_t            s8;
    uintptr_t            s9;
    uintptr_t            s10;
    uintptr_t            s11;
    uintptr_t            t3;
    uintptr_t            t4;
    uintptr_t            t5;
    uintptr_t            t6;
};

/**
 * @brief 浮点寄存器
 */
struct fregs_t {
    uintptr_t            ft0;
    uintptr_t            ft1;
    uintptr_t            ft2;
    uintptr_t            ft3;
    uintptr_t            ft4;
    uintptr_t            ft5;
    uintptr_t            ft6;
    uintptr_t            ft7;
    uintptr_t            fs0;
    uintptr_t            fs1;
    uintptr_t            fa0;
    uintptr_t            fa1;
    uintptr_t            fa2;
    uintptr_t            fa3;
    uintptr_t            fa4;
    uintptr_t            fa5;
    uintptr_t            fa6;
    uintptr_t            fa7;
    uintptr_t            fs2;
    uintptr_t            fs3;
    uintptr_t            fs4;
    uintptr_t            fs5;
    uintptr_t            fs6;
    uintptr_t            fs7;
    uintptr_t            fs8;
    uintptr_t            fs9;
    uintptr_t            fs10;
    uintptr_t            fs11;
    uintptr_t            ft8;
    uintptr_t            ft9;
    uintptr_t            ft10;
    uintptr_t            ft11;
};

/**
 * @brief 所有寄存器，在中断时使用，共 32+32+7=71 个
 */
struct all_regs_t {
    xregs_t              xregs;
    fregs_t              fregs;
    uintptr_t            sepc;
    uintptr_t            stval;
    uintptr_t            scause;
    uintptr_t            sie;
    sstatus_t            sstatus;
    satp_t               satp;
    uintptr_t            sscratch;
};

}; // namespace CPU
```

现场保存与恢复

```assembly

// 寄存器长度，8 字节
.equ REG_BYTES, 8
// 所有寄存器数量
.equ ALL_REGS,  71
// 保存所有寄存器需要的大小
.equ ALL_SIZE,  (ALL_REGS * REG_BYTES)

// 将寄存器 a 保存在 c 偏移 b 的位置
.macro sd_base a, b, c
sd \a, ((\b) * REG_BYTES)(\c)
.endm

// 从 c 的偏移 b 处获取数据并赋值给寄存器 a
.macro ld_base a, b, c
ld \a, ((\b) * REG_BYTES)(\c)
.endm

// 将 float 寄存器 a 保存在 c 偏移 b 的位置
.macro fsd_base a, b, c
fsd \a, ((\b) * REG_BYTES)(\c)
.endm

// 从 c 的偏移 b 处获取数据并赋值给 float 寄存器 a
.macro fld_base a, b, c
fld \a, ((\b) * REG_BYTES)(\c)
.endm

/**
 * @brief 保存所有寄存器
 * @param  _base           要保存到的基地址
 */
.macro all_regs_save _base
    sd_base  zero, 0,  \_base
    sd_base  ra,   1,  \_base
    sd_base  sp,   2,  \_base
    sd_base  gp,   3,  \_base
    sd_base  tp,   4,  \_base
    sd_base  t0,   5,  \_base
    sd_base  t1,   6,  \_base
    sd_base  t2,   7,  \_base
    sd_base  s0,   8,  \_base
    sd_base  s1,   9,  \_base
    sd_base  a0,   10, \_base
    sd_base  a1,   11, \_base
    sd_base  a2,   12, \_base
    sd_base  a3,   13, \_base
    sd_base  a4,   14, \_base
    sd_base  a5,   15, \_base
    sd_base  a6,   16, \_base
    sd_base  a7,   17, \_base
    sd_base  s2,   18, \_base
    sd_base  s3,   19, \_base
    sd_base  s4,   20, \_base
    sd_base  s5,   21, \_base
    sd_base  s6,   22, \_base
    sd_base  s7,   23, \_base
    sd_base  s8,   24, \_base
    sd_base  s9,   25, \_base
    sd_base  s10,  26, \_base
    sd_base  s11,  27, \_base
    sd_base  t3,   28, \_base
    sd_base  t4,   29, \_base
    sd_base  t5,   30, \_base
    sd_base  t6,   31, \_base

    fsd_base ft0,  32, \_base
    fsd_base ft1,  33, \_base
    fsd_base ft2,  34, \_base
    fsd_base ft3,  35, \_base
    fsd_base ft4,  36, \_base
    fsd_base ft5,  37, \_base
    fsd_base ft6,  38, \_base
    fsd_base ft7,  39, \_base
    fsd_base fs0,  40, \_base
    fsd_base fs1,  41, \_base
    fsd_base fa0,  42, \_base
    fsd_base fa1,  43, \_base
    fsd_base fa2,  44, \_base
    fsd_base fa3,  45, \_base
    fsd_base fa4,  46, \_base
    fsd_base fa5,  47, \_base
    fsd_base fa6,  48, \_base
    fsd_base fa7,  49, \_base
    fsd_base fs2,  50, \_base
    fsd_base fs3,  51, \_base
    fsd_base fs4,  52, \_base
    fsd_base fs5,  53, \_base
    fsd_base fs6,  54, \_base
    fsd_base fs7,  55, \_base
    fsd_base fs8,  56, \_base
    fsd_base fs9,  57, \_base
    fsd_base fs10, 58, \_base
    fsd_base fs11, 59, \_base
    fsd_base ft8,  60, \_base
    fsd_base ft9,  61, \_base
    fsd_base ft10, 62, \_base
    fsd_base ft11, 63, \_base

    csrr     t0,   sepc
    sd_base  t0,   64, \_base
    csrr     t0,   stval
    sd_base  t0,   65, \_base
    csrr     t0,   scause
    sd_base  t0,   66, \_base
    csrr     t0,   sie
    sd_base  t0,   67, \_base
    csrr     t0,   sstatus
    sd_base  t0,   68, \_base
    csrr     t0,   satp
    sd_base  t0,   69, \_base
    csrr     t0,   sscratch
    sd_base  t0,   70, \_base
.endm

/**
 * @brief 恢复所有寄存器
 * @param  _base           要恢复数据的基地址
 */
.macro all_regs_load _base
    ld_base  t0,   64, \_base
    csrw     sepc,     t0
    ld_base  t0,   65, \_base
    csrw     stval,    t0
    ld_base  t0,   66, \_base
    csrw     scause,   t0
    ld_base  t0,   67, \_base
    csrw     sie,  t0
    ld_base  t0,   68, \_base
    csrw     sstatus,  t0
    ld_base  t0,   69, \_base
    csrw     satp,     t0
    ld_base  t0,   70, \_base
    csrw     sscratch, t0

    ld_base  zero, 0,  \_base
    ld_base  ra,   1,  \_base
    ld_base  sp,   2,  \_base
    ld_base  gp,   3,  \_base
    ld_base  tp,   4,  \_base
    ld_base  t0,   5,  \_base
    ld_base  t1,   6,  \_base
    ld_base  t2,   7,  \_base
    ld_base  s0,   8,  \_base
    ld_base  s1,   9,  \_base
    ld_base  a0,   10, \_base
    ld_base  a1,   11, \_base
    ld_base  a2,   12, \_base
    ld_base  a3,   13, \_base
    ld_base  a4,   14, \_base
    ld_base  a5,   15, \_base
    ld_base  a6,   16, \_base
    ld_base  a7,   17, \_base
    ld_base  s2,   18, \_base
    ld_base  s3,   19, \_base
    ld_base  s4,   20, \_base
    ld_base  s5,   21, \_base
    ld_base  s6,   22, \_base
    ld_base  s7,   23, \_base
    ld_base  s8,   24, \_base
    ld_base  s9,   25, \_base
    ld_base  s10,  26, \_base
    ld_base  s11,  27, \_base
    ld_base  t3,   28, \_base
    ld_base  t4,   29, \_base
    ld_base  t5,   30, \_base
    ld_base  t6,   31, \_base

    fld_base ft0,  32, \_base
    fld_base ft1,  33, \_base
    fld_base ft2,  34, \_base
    fld_base ft3,  35, \_base
    fld_base ft4,  36, \_base
    fld_base ft5,  37, \_base
    fld_base ft6,  38, \_base
    fld_base ft7,  39, \_base
    fld_base fs0,  40, \_base
    fld_base fs1,  41, \_base
    fld_base fa0,  42, \_base
    fld_base fa1,  43, \_base
    fld_base fa2,  44, \_base
    fld_base fa3,  45, \_base
    fld_base fa4,  46, \_base
    fld_base fa5,  47, \_base
    fld_base fa6,  48, \_base
    fld_base fa7,  49, \_base
    fld_base fs2,  50, \_base
    fld_base fs3,  51, \_base
    fld_base fs4,  52, \_base
    fld_base fs5,  53, \_base
    fld_base fs6,  54, \_base
    fld_base fs7,  55, \_base
    fld_base fs8,  56, \_base
    fld_base fs9,  57, \_base
    fld_base fs10, 58, \_base
    fld_base fs11, 59, \_base
    fld_base ft8,  60, \_base
    fld_base ft9,  61, \_base
    fld_base ft10, 62, \_base
    fld_base ft11, 63, \_base
.endm

.section .text
// 保存所有寄存器
.globl trap_entry
.extern trap_handler
.align 4
trap_entry:
    // 将所有寄存器保存到栈上
    // 在栈上留出保存寄存器的空间
    addi sp, sp, -ALL_SIZE
    all_regs_save sp

    // 调用 intr.cpp: trap_handler
    // 传递参数
    csrr a0, sepc
    csrr a1, stval
    csrr a2, scause
    mv   a3, sp
    csrr a4, sie
    csrr a5, sstatus
    csrr a6, satp
    csrr a7, sscratch
    jal trap_handler

    // 从栈上恢复所有寄存器
    all_regs_load sp
    // 释放栈上用于保存寄存器的空间
    addi sp, sp, ALL_SIZE

    // 跳转到 sepc 处执行
    sret
```

中断初始化
```c++
int32_t INTR::init(void) {
    // 设置 trap vector
    CPU::WRITE_STVEC((uintptr_t)trap_entry);
    // 直接跳转到处理函数
    CPU::STVEC_DIRECT();
    // 设置处理函数
    for (auto &i : interrupt_handlers) {
        i = handler_default;
    }
    for (auto &i : excp_handlers) {
        i = handler_default;
    }
    // 内部中断初始化
    CLINT::get_instance().init();
    // 外部中断初始化
    PLIC::get_instance().init();
    // 注册缺页中断
    register_excp_handler(CPU::EXCP_LOAD_PAGE_FAULT, pg_load_excp);
    // 注册缺页中断
    register_excp_handler(CPU::EXCP_STORE_AMO_PAGE_FAULT, pg_store_excp);
    info("intr init.\n");
    return 0;
}
```

中断处理
```c++
/**
 * @brief 中断处理函数
 * @param  _scause         原因
 * @param  _sepc           值
 * @param  _stval          值
 * @param  _scause         值
 * @param  _all_regs       保存在栈上的所有寄存器，实际上是 sp
 * @param  _sie            值
 * @param  _sstatus        值
 * @param  _sscratch       值
 */
extern "C" void trap_handler(uintptr_t _sepc, uintptr_t _stval,
                             uintptr_t _scause, CPU::all_regs_t *_all_regs,
                             uintptr_t _sie, CPU::sstatus_t _sstatus,
                             CPU::satp_t _satp, uintptr_t _sscratch) {
    if (_scause & CPU::CAUSE_INTR_MASK) {
// 中断
        // 跳转到对应的处理函数
        INTR::get_instance().do_interrupt(_scause & CPU::CAUSE_CODE_MASK, 0,
                                          nullptr);
    }
    else {
// 异常
// 跳转到对应的处理函数
        INTR::get_instance().do_excp(_scause & CPU::CAUSE_CODE_MASK, 0,
                                     nullptr);
    }
    return;
}
```

更多细节请查看注释。

- 相关代码

    ./src/arch/riscv64/context.S

    ./src/arch/riscv64/intr/intr_s.S

    ./src/arch/riscv64/intr/include/intr.h

    ./src/arch/riscv64/intr/intr.cpp
    
    ./src/arch/riscv64/intr/clint.cpp
        
    ./src/arch/riscv64/intr/plic.cpp
    
    ./src/arch/riscv64/intr/timer.cpp

## 相关文档

TODO
