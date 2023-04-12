# SimpleKernel 虚拟内存管理

虚拟内存与架构密切相关，但仍然能够找到它们的共同之处。

在 `cpu.hpp` 中封装了它们的差异，使得可以获得一个近似一致的虚拟内存管理逻辑。


## IA32

TODO

## RISCV

页表结构可以抽象为以下数据

```c++
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

    static constexpr const uint64_t PPN_OFFSET = 12;

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
}; // namespace CPU
```

以下是在执行 mmap 映射内存时的逻辑。
```c++
// 在 _pgd 中查找 _va 对应的页表项
// 如果未找到，_alloc 为真时会进行分配
pte_t *VMM::find(const pt_t _pgd, uintptr_t _va, bool _alloc) {
    pt_t pgd = _pgd;
    // sv39 共有三级页表，一级一级查找
    // -1 是因为最后一级是具体的某一页，在函数最后直接返回
    for (size_t level = VMM_PT_LEVEL - 1; level > 0; level--) {
        // 每次循环会找到 _va 的第 level 级页表 pgd
        // 相当于 pgd_level[VPN_level]，这样相当于得到了第 level 级页表的地址
        pte_t *pte = (pte_t *)&pgd[PX(level, _va)];
        // 解引用 pte，如果有效，获取 level+1 级页表，
        if ((*pte & VMM_PAGE_VALID) == 1) {
            // pgd 指向下一级页表
            // *pte 保存的是页表项，需要转换为对应的物理地址
            pgd = (pt_t)PTE2PA(*pte);
        }
        // 如果无效
        else {
            // 判断是否需要分配
            // 如果需要
            if (_alloc == true) {
                // 申请新的物理页
                pgd = (pt_t)PMM::get_instance().alloc_page_kernel();
                bzero(pgd, COMMON::PAGE_SIZE);
                // 申请失败则返回
                if (pgd == nullptr) {
                    // 如果出现这种情况，说明物理内存不够，一般不会出现
                    assert(0);
                    return nullptr;
                }
                // 清零
                bzero(pgd, COMMON::PAGE_SIZE);
                // 填充页表项
                *pte = PA2PTE((uintptr_t)pgd) | VMM_PAGE_VALID;
            }
            // 不分配的话直接返回
            else {
                return nullptr;
            }
        }
    }
    // 0 最低级 pt
    return &pgd[PX(0, _va)];
}
```

更多细节请查看注释。

## 相关文档

TODO