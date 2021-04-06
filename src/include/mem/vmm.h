
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// vmm.h for Simple-XX/SimpleKernel.

#ifndef _VMM_H_
#define _VMM_H_

#include "limits.h"
#include "common.h"
#include "pmm.h"
#include "io.h"

// Format of a 32-Bit Page-Table Entry that Maps a 4-KByte Page
class page_table32_entry_t {
private:
    IO io;

public:
    // Present; must be 1 to map a 4-KByte page
    uint32_t p : 1;
    // Read/write; if 0, writes may not be allowed to the 4-KByte page
    // referenced by this entry
    uint32_t rw : 1;
    // User/supervisor; if 0, user-mode accesses are not allowed to the 4-KByte
    // page referenced by this entry
    uint32_t us : 1;
    // Page-level write-through; indirectly determines the memory type used to
    // access the 4-KByte page referenced by this entry
    uint32_t pwt : 1;
    // Page-level cache disable; indirectly determines the memory type used to
    // access the 4-KByte page referenced by this entry
    uint32_t pcd : 1;
    // Accessed; indicates whether software has accessed the 4-KByte page
    // referenced by this entry
    uint32_t a : 1;
    // Dirty; indicates whether software has written to the 4-KByte page
    // referenced by this entry
    uint32_t d : 1;
    // If the PAT is supported, indirectly determines the memory type used to
    // access the 4-KByte page referenced by this entry;
    // otherwise, reserved (must be 0)
    uint32_t pat : 1;
    // Global; if CR4.PGE = 1, determines whether the translation is global (see
    // Section 4.10); ignored otherwise
    uint32_t g : 1;
    // Ignored
    uint32_t ignore : 3;
    // Physical address of the 4-KByte page referenced by this entry
    uint32_t addr : 20;
    void     printf(void) {
        io.printf("%05X %d %d%d%d%d%d%d %d%d%d", addr, ignore, g, pat, d, a,
                  pcd, pwt, us, rw, p);
    }
};

// Format of a 32-Bit Page-Directory Entry that References a Page Table
class page_dir32_entry_t {
private:
    IO io;

public:
    // Present; must be 1 to reference a page table
    uint32_t p : 1;
    // Read/write; if 0, writes may not be allowed to the 4-MByte region
    // controlled by this entry
    uint32_t rw : 1;
    // User/supervisor; if 0, user-mode accesses are not allowed to the 4-MByte
    // region controlled by this entry
    uint32_t us : 1;
    // Page-level write-through; indirectly determines the memory type used to
    // access the page table referenced by this entry
    uint32_t pwt : 1;
    // Page-level cache disable; indirectly determines the memory type used to
    // access the page table referenced by this entry
    uint32_t pcd : 1;
    // Accessed; indicates whether this entry has been used for linear-address
    // translation
    uint32_t a : 1;
    //  Ignored
    uint32_t ignore1 : 1;
    // If CR4.PSE = 1, must be 0 (otherwise, this entry maps a 4-MByte page)
    // otherwise, ignored
    uint32_t ps : 1;
    // Ignored
    uint32_t ignore2 : 4;
    // Physical address of 4-KByte aligned page table referenced by this entry
    uint32_t addr : 20;
    void     printf(void) {
        io.printf("%05X %d %d %d %d%d%d %d%d%d", addr, ignore2, ps, ignore1, a,
                  pcd, pwt, us, rw, p);
    }
};

// Use of CR3 with 32-Bit Paging
class cr3_t {
public:
    // Ignored
    uint32_t ignore1 : 3;
    // Page-level write-through; indirectly determines the memory type used to
    // access the page directory during linear-address translation
    uint32_t pwt : 1;
    // Page-level cache disable; indirectly determines the memory type used to
    // access the page directory during linear-address translation
    uint32_t pcd : 1;
    // Ignored
    uint32_t ignore2 : 7;
    // Physical address of the 4-KByte aligned page directory used for
    // linear-address translation
    uint32_t addr : 20;
};

// 页表项 sizeof(pte_t) 大小 = 4B 2^3
static constexpr const uint32_t VMM_PTE_SIZE = sizeof(page_table32_entry_t);

// 每页能映射多少页表项 = 页大小/页表项大小 2^9
static constexpr const uint32_t VMM_PTE_PRE_PAGE =
    COMMON::PAGE_SIZE / VMM_PTE_SIZE;

// 页目录项 sizeof(pgd_t) 大小 = 4B 2^3
static constexpr const uint32_t VMM_PGDE_SIZE = sizeof(page_dir32_entry_t);

// 每页能映射多少页目录项 = 页大小/页目录项大小 2^9
static constexpr const uint32_t VMM_PGDE_PRE_PAGE =
    COMMON::PAGE_SIZE / VMM_PGDE_SIZE;

// 每个页表能映射多少页 = 页大小/页表项大小: 2^9
static constexpr const uint32_t VMM_PAGES_PRE_PAGE_TABLE =
    COMMON::PAGE_SIZE / VMM_PTE_SIZE;

// 每个页目录能映射多少页表 = 页大小/页目录项大小 2^9
static constexpr const uint32_t VMM_PAGE_TABLES_PRE_PAGE_DIRECTORY =
    COMMON::PAGE_SIZE / VMM_PGDE_SIZE;

// 每个页目录能映射多少页 = 页表数量*每个页表能映射多少页 2^18
static constexpr const uint32_t VMM_PAGES_PRE_PAGE_DIRECTORY =
    VMM_PAGE_TABLES_PRE_PAGE_DIRECTORY * VMM_PAGES_PRE_PAGE_TABLE;

// 页表大小，一页表能映射多少 Byte 内存 = 页表项数量*页表项映射大小 2^21
static constexpr const uint64_t VMM_PAGE_TABLE_SIZE =
    VMM_PAGES_PRE_PAGE_TABLE * COMMON::PAGE_SIZE;

// 页目录大小，一页目录能映射多少 Byte 内存 = 页表数量*页表映射大小 2^32
static constexpr const uint64_t VMM_PAGE_DIRECTORY_SIZE =
    VMM_PAGE_TABLES_PRE_PAGE_DIRECTORY * VMM_PAGE_TABLE_SIZE;

// 虚拟内存大小 4GB
static constexpr const uint64_t VMM_VMEM_SIZE = UINT_MAX + (uint64_t)1;

// 映射全部虚拟内存需要的页数 = 虚拟内存大小/页大小 2^20
static constexpr const uint64_t VMM_PAGES_TOTAL =
    VMM_VMEM_SIZE / COMMON::PAGE_SIZE;

// 映射全部虚拟内存需要的页表数 = 虚拟内存大小/页表大小 2^12
static constexpr const uint64_t VMM_PAGE_TABLES_TOTAL =
    VMM_VMEM_SIZE / VMM_PAGE_TABLE_SIZE;

// 映射全部虚拟内存需要的页目录数 = 虚拟内存大小/页目录大小 2^0
static constexpr const uint64_t VMM_PAGE_DIRECTORIES_TOTAL =
    (VMM_VMEM_SIZE / VMM_PAGE_DIRECTORY_SIZE) +
    ((VMM_VMEM_SIZE % VMM_PAGE_DIRECTORY_SIZE) == 0 ? 0 : 1);

// 映射内核需要的页表数
static const uint64_t VMM_PAGE_TABLES_KERNEL =
    (COMMON::KERNEL_SIZE / VMM_PAGE_TABLE_SIZE) +
    (COMMON::KERNEL_SIZE % VMM_PAGE_TABLE_SIZE == 0 ? 0 : 1);

// 内核映射的大小 32MB
static constexpr const uint64_t VMM_KERNEL_SIZE = 0x2000000;

// 内核映射的页数
static constexpr const uint64_t VMM_KERNEL_PAGES =
    VMM_KERNEL_SIZE / COMMON::PAGE_SIZE;

// 内核映射的页表数
static constexpr const uint64_t VMM_KERNEL_PAGE_TABLES =
    VMM_KERNEL_SIZE / VMM_PAGE_TABLE_SIZE;

// 内核映射的页目录数
static constexpr const uint64_t VMM_KERNEL_PAGE_DIRECTORIES =
    (VMM_KERNEL_SIZE / VMM_PAGE_DIRECTORY_SIZE) +
    ((VMM_KERNEL_SIZE % VMM_PAGE_DIRECTORY_SIZE) == 0 ? 0 : 1);

// 如果为 0  那么页面只能被运行在超级用户特权级 (0,1 或 2)  的程序访问。
static constexpr const uint32_t VMM_PAGE_KERNEL = 0x00000000;
// P = 1 表示有效； P = 0 表示无效。
static constexpr const uint32_t VMM_PAGE_PRESENT = 0x00000001;
// 如果为 0  表示页面只读或可执行。
static constexpr const uint32_t VMM_PAGE_RW = 0x00000002;
// U/S-- 位 2 是用户 / 超级用户 (User/Supervisor) 标志。
// 如果为 1  那么运行在任何特权级上的程序都可以访问该页面。
static constexpr const uint32_t VMM_PAGE_USER = 0x00000004;

// 获取一个地址的页目录，高 10 位
#define VMM_PGD_INDEX(x) (((x) >> 22) & 0x03FF)

// 获取一个地址的页表，中间 10 位
#define VMM_PTE_INDEX(x) (((x) >> 12) & 0x03FF)

// 获取一个地址的页內偏移，低 12 位
#define VMM_OFFSET_INDEX(x) ((x)&0x0FFF)

// 逻辑地址到物理地址转换
#define VMM_LA_PA(la) (la - COMMON::KERNEL_BASE)

// 物理地址到逻辑地址转换
#define VMM_PA_LA(pa) (pa + COMMON::KERNEL_BASE)

class VMM {
private:
    static IO                   io;
    static PMM                  pmm;
    static page_table32_entry_t pte_kernel[VMM_KERNEL_PAGES]
        __attribute__((aligned(0x1000)));
    static page_dir32_entry_t pgde_kernel[VMM_KERNEL_PAGE_TABLES]
        __attribute__((aligned(0x1000)));
    static cr3_t cr3_kernel;

protected:
public:
    VMM(void);
    ~VMM(void);
    // 初始化
    void init(void);
    // 设置当前页目录
    // void set_pgd(page_dir_t pgd);
    // 映射物理地址到虚拟地址
    void mmap(void *va, void *pa, uint32_t flag);
    // 取消映射
    void unmmap(void *va);
    // 获取映射的物理地址
    // 已映射返回 1，未映射返回 0
    uint32_t get_mmap(void *va, void *pa);
};

#endif /* INCLUDE_VMM_H */
