
/**
 * @file cpu.hpp
 * @brief cpu 相关定义
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2021-09-18
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2021-09-18<td>digmouse233<td>迁移到 doxygen
 * </table>
 */

#ifndef SIMPLEKERNEL_CPU_HPP
#define SIMPLEKERNEL_CPU_HPP

#include "cstdbool"
#include "cstdint"
#include "iostream"

/**
 * @brief cpu 相关
 * @todo
 */
namespace CPU {
/**
 * @brief pte 结构
 * @todo 使用 pte 结构重写 vmm
 */
struct pte_t {
    enum {
        VALID_OFFSET    = 0,
        READ_OFFSET     = 1,
        WRITE_OFFSET    = 2,
        EXEC_OFFSET     = 3,
        USER_OFFSET     = 4,
        GLOBAL_OFFSET   = 5,
        ACCESSED_OFFSET = 6,
        DIRTY_OFFSET    = 7,
        VALID           = 1 << VALID_OFFSET,
        READ            = 1 << READ_OFFSET,
        WRITE           = 1 << WRITE_OFFSET,
        EXEC            = 1 << EXEC_OFFSET,
        USER            = 1 << USER_OFFSET,
        GLOBAL          = 1 << GLOBAL_OFFSET,
        ACCESSED        = 1 << ACCESSED_OFFSET,
        DIRTY           = 1 << DIRTY_OFFSET,
    };

    union {
        struct {
            uint64_t flags    : 8;
            uint64_t rsw      : 2;
            uint64_t ppn      : 44;
            uint64_t reserved : 10;
        };

        uint64_t val;
    };

    pte_t(void) {
        val = 0;
        return;
    }

    pte_t(uint64_t _val) : val(_val) {
        return;
    }

    friend std::ostream& operator<<(std::ostream& _os, const pte_t& _pte) {
        printf("val: 0x%p, valid: %s, read: %s, write: %s, exec: %s, user: %s, "
               "global: %s, accessed: %s, dirty: %s, rsw: 0x%p, ppn: 0x%p",
               _pte.val, (_pte.flags & VALID) == VALID ? "true" : "false",
               (_pte.flags & READ) == READ ? "true" : "false",
               (_pte.flags & WRITE) == WRITE ? "true" : "false",
               (_pte.flags & EXEC) == EXEC ? "true" : "false",
               (_pte.flags & USER) == USER ? "true" : "false",
               (_pte.flags & GLOBAL) == GLOBAL ? "true" : "false",
               (_pte.flags & ACCESSED) == ACCESSED ? "true" : "false",
               (_pte.flags & DIRTY) == DIRTY ? "true" : "false", _pte.rsw,
               _pte.ppn);
        return _os;
    }
};

/**
 * @brief satp 结构
 */
struct satp_t {
    enum {
        NONE = 0,
        SV39 = 8,
        SV48 = 9,
        SV57 = 10,
        SV64 = 11,
    };

    static constexpr const char* MODE_NAME[] = {
        [NONE] = "NONE", "UNKNOWN",       "UNKNOWN",       "UNKNOWN",
        "UNKNOWN",       "UNKNOWN",       "UNKNOWN",       "UNKNOWN",
        [SV39] = "SV39", [SV48] = "SV48", [SV57] = "SV57", [SV64] = "SV64",
    };

    union {
        struct {
            uint64_t ppn  : 44;
            uint64_t asid : 16;
            uint64_t mode : 4;
        };

        uint64_t val;
    };

    static constexpr const uint64_t PPN_OFFSET = 12;

    satp_t(void) {
        val = 0;
        return;
    }

    satp_t(uint64_t _val) : val(_val) {
        return;
    }

    friend std::ostream& operator<<(std::ostream& _os, const satp_t& _satp) {
        printf("val: 0x%p, ppn: 0x%p, asid: 0x%p, mode: %s", _satp.val,
               _satp.ppn, _satp.asid, MODE_NAME[_satp.mode]);
        return _os;
    }
};

/**
 * @brief 设置页目录，仅更改 ppn
 * @param  _x               要设置的页目录
 * @note supervisor address translation and protection; holds the address of
 * the page table.
 * @todo 需要判断 _x 是否已经处理过
 */
inline static void SET_PGD(uintptr_t _x) {
    satp_t satp;
    // 读取现在的 pgd
    asm("csrr %0, satp" : "=r"(satp));
    // 更改 ppn
    // satp.ppn = _x & satp_t::PPN_MASK;
    satp.ppn = _x >> satp_t::PPN_OFFSET;
    // 写回
    asm("csrw satp, %0" : : "r"(satp));
    return;
}

/**
 * @brief 获取页目录，仅获取 ppn
 * @return uintptr_t        页目录
 */
inline static uintptr_t GET_PGD(void) {
    satp_t satp;
    asm("csrr %0, satp" : "=r"(satp));
    return satp.ppn << satp_t::PPN_OFFSET;
}

/**
 * @brief 开启分页
 * @return true             成功
 * @return false            失败
 */
inline static bool ENABLE_PG(void) {
    uintptr_t x = GET_PGD();
    satp_t    satp;
    satp.ppn  = x >> satp_t::PPN_OFFSET;
    satp.asid = 0;
    satp.mode = satp_t::SV39;
    asm("csrw satp, %0" : : "r"(satp));
    info("paging enabled.\n");
    return true;
}

/**
 * @brief 刷新 tlb
 */
inline static void VMM_FLUSH(uintptr_t) {
    // the zero, zero means flush all TLB entries.
    asm("sfence.vma zero, zero");
    return;
}

};     // namespace CPU

#endif /* SIMPLEKERNEL_CPU_HPP */
