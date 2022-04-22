
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

#ifndef _CPU_HPP_
#define _CPU_HPP_

#include "stdint.h"
#include "stdbool.h"
#include "iostream"

/**
 * @brief cpu 相关
 * @todo
 */
namespace CPU {
/**
 * @brief pte 结构
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
            uint64_t flags : 8;
            uint64_t rsw : 2;
            uint64_t ppn : 44;
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
    friend std::ostream &operator<<(std::ostream &_os, const pte_t &_pte) {
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
    static constexpr const char *MODE_NAME[] = {
        [NONE] = "NONE", "UNKNOWN",       "UNKNOWN",       "UNKNOWN",
        "UNKNOWN",       "UNKNOWN",       "UNKNOWN",       "UNKNOWN",
        [SV39] = "SV39", [SV48] = "SV48", [SV57] = "SV57", [SV64] = "SV64",
    };

    union {
        struct {
            uint64_t ppn : 44;
            uint64_t asid : 16;
            uint64_t mode : 4;
        };
        uint64_t val;
    };

    satp_t(void) {
        val = 0;
        return;
    }
    satp_t(uint64_t _val) : val(_val) {
        return;
    }
    friend std::ostream &operator<<(std::ostream &_os, const satp_t &_satp) {
        printf("val: 0x%p, ppn: 0x%p, asid: 0x%p, mode: %s", _satp.val,
               _satp.ppn, _satp.asid, MODE_NAME[_satp.mode]);
        return _os;
    }
};

/**
 * @brief 设置 sv39 虚拟内存模式
 * @param  _pgd             要设置的页目录
 * @return constexpr uintptr_t 设置好的页目录
 */
static uintptr_t SET_SV39(uintptr_t _pgd) {
    satp_t satp;
    satp.val  = _pgd >> 12;
    satp.asid = 0;
    satp.mode = satp_t::SV39;
    return satp.val;
}

/**
 * @brief 设置页目录
 * @param  _x               要设置的页目录
 * @note supervisor address translation and protection; holds the address of
 * the page table.
 * @todo 需要判断 _x 是否已经处理过
 */
static inline void SET_PGD(uintptr_t _x) {
    satp_t satp_old;
    satp_t satp_new;
    satp_new.val  = _x;
    satp_new.asid = 0;
    // 读取现在的 pgd
    asm("csrr %0, satp" : "=r"(satp_old));
    // 如果开启了 sv39
    if (satp_old.mode == satp_t::SV39) {
        // 将新的页目录也设为开启
        satp_new.mode = satp_t::SV39;
    }
    asm("csrw satp, %0" : : "r"(satp_new));
    return;
}

/**
 * @brief 获取页目录
 * @return uintptr_t        页目录
 */
static inline uintptr_t GET_PGD(void) {
    satp_t satp;
    asm("csrr %0, satp" : "=r"(satp));
    // 如果开启了虚拟内存，恢复为原始格式
    if (satp.mode == satp_t::SV39) {
        return satp.ppn << 12;
    }
    return satp.val;
}

/**
 * @brief 开启分页
 * @return true             成功
 * @return false            失败
 */
static inline bool ENABLE_PG(void) {
    uintptr_t x = GET_PGD();
    SET_PGD(SET_SV39(x));
    info("paging enabled.\n");
    return true;
}

/**
 * @brief 刷新 tlb
 */
static inline void VMM_FLUSH(uintptr_t) {
    // the zero, zero means flush all TLB entries.
    asm("sfence.vma zero, zero");
    return;
}

}; // namespace CPU

#endif /* _CPU_HPP_ */
