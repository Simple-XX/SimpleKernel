
// This file is a part of SimpleXX/SimpleKernel
// (https://github.com/SimpleXX/SimpleKernel).
//
// slab.c for SimpleXX/SimpleKernel.

#ifdef __cplusplus
extern "C" {
#endif

#include "stdio.h"
#include "string.h"
#include "assert.h"
#include "mem/pmm.h"
#include "mem/vmm.h"
#include "mem/slab.h"

#define SLAB_USED (0x00)
#define SLAB_UNUSED (0x01)

// slab 管理的内存页数
#define SLAB_PAGE_COUNT (pmm_free_pages_count())
// 最小空间
#define SLAB_MIN (0xFF)

static void  init(ptr_t addr_start);
static ptr_t alloc(size_t byte);
static void  free(ptr_t addr);

heap_manage_t slab_manage = {"Slab", &init, &alloc, &free};

typedef struct slab_block {
    // 该内存块是否已经被申请
    size_t allocated;
    // 当前内存块的长度，不包括头长度
    size_t len;
} slab_block_t;

// 一个仅在这里使用的简单循环链表
typedef struct list_entry {
    slab_block_t       slab_block;
    struct list_entry *next;
    struct list_entry *prev;
} list_entry_t;

// 管理结构
typedef struct slab_manage {
    // 管理的内存起始地址，包括头的位置
    ptr_t addr_start;
    // 管理的内存结束地址
    ptr_t addr_end;
    // 物理内存的总大小，包括头的大小
    size_t mm_total;
    // 当前空闲内存大小
    size_t mm_free;
    // 当前存在的内存数量
    size_t block_count;
    // 内存头链表
    list_entry_t *slab_list;
} slab_manage_t;

// 管理信息
static slab_manage_t sb_manage;
static list_entry_t *sb_list = NULL;

// 初始化节点
static inline void list_init(list_entry_t *list);

// 在中间添加元素
static inline void list_add_middle(list_entry_t *prev, list_entry_t *next,
                                   list_entry_t *new);

// 在 prev 后添加项
static inline void list_add_after(list_entry_t *prev, list_entry_t *new);

// 在 next 前添加项
static inline void list_add_before(list_entry_t *next, list_entry_t *new);

// 删除元素
static inline void list_del(list_entry_t *list);

// 返回前面的元素
static inline list_entry_t *list_prev(list_entry_t *list);

// 返回后面的的元素
static inline list_entry_t *list_next(list_entry_t *list);

// 返回 chunk_info
static inline slab_block_t *list_slab_block(list_entry_t *list);

// 初始化
void list_init(list_entry_t *list) {
    list->next = list;
    list->prev = list;
    return;
}

// 在中间添加元素
void list_add_middle(list_entry_t *prev, list_entry_t *next,
                     list_entry_t *new) {
    next->prev = new;
    new->next  = next;
    new->prev  = prev;
    prev->next = new;
}

// 在 prev 后添加项
void list_add_after(list_entry_t *prev, list_entry_t *new) {
    list_add_middle(prev, prev->next, new);
    return;
}

// 在 next 前添加项
void list_add_before(list_entry_t *next, list_entry_t *new) {
    list_add_middle(next->prev, next, new);
    return;
}

// 删除元素
void list_del(list_entry_t *list) {
    list->next->prev = list->prev;
    list->prev->next = list->next;
    return;
}

// 返回前面的元素
list_entry_t *list_prev(list_entry_t *list) {
    return list->prev;
}

// 返回后面的的元素
list_entry_t *list_next(list_entry_t *list) {
    return list->next;
}

// 返回 chunk_info
slab_block_t *list_slab_block(list_entry_t *list) {
    return &(list->slab_block);
}

static inline void set_used(list_entry_t *entry);
static inline void set_unused(list_entry_t *entry);

// 将 entry 设置为已使用
static inline void set_used(list_entry_t *entry) {
    list_slab_block(entry)->allocated = SLAB_USED;
    sb_manage.mm_free -= sizeof(list_entry_t);
    sb_manage.mm_free -= list_slab_block(entry)->len;
    return;
}

// 将 entry 设置为未使用
static inline void set_unused(list_entry_t *entry) {
    list_slab_block(entry)->allocated = SLAB_UNUSED;
    sb_manage.mm_free -= sizeof(list_entry_t);
    sb_manage.mm_free += list_slab_block(entry)->len;
    return;
}

void init(ptr_t addr_start) {
    // 设置第一块内存的信息
    // 首先给链表中添加一个大小为 1 页的块
    ptr_t pa = pmm_alloc(VMM_PAGE_SIZE, NORMAL);
    ptr_t va = addr_start;
    // 映射内存
    map(pgd_kernel, va, pa, VMM_PAGE_PRESENT | VMM_PAGE_RW);
    bzero((void *)va, VMM_PAGE_SIZE);
    // 解释这段内存
    sb_list = (list_entry_t *)va;
    // 填充管理信息
    sb_manage.addr_start  = addr_start;
    sb_manage.addr_end    = addr_start + VMM_PAGE_SIZE;
    sb_manage.mm_total    = VMM_PAGE_SIZE;
    sb_manage.mm_free     = 0;
    sb_manage.block_count = 1;
    sb_manage.slab_list   = sb_list;
    list_init(sb_manage.slab_list);
    // 设置第一块内存的相关信息
    list_slab_block(sb_manage.slab_list)->allocated = SLAB_UNUSED;
    list_slab_block(sb_manage.slab_list)->len =
        VMM_PAGE_SIZE - sizeof(list_entry_t);
    return;
}

static inline list_entry_t *slab_split(list_entry_t *list, size_t byte);
static inline void          slab_merge(list_entry_t *list);

// 切分内存块，len 为调用者申请的大小，不包括头大小
// 返回分割出来的管理头地址
list_entry_t *slab_split(list_entry_t *entry, size_t len) {
    // 如果剩余内存大于内存头的长度+设定的最小长度
    if ((list_slab_block(entry)->len - len > sizeof(list_entry_t) + SLAB_MIN)) {
        // 添加新的链表项，位于旧表项开始地址+旧表项长度
        list_entry_t *new_entry =
            (list_entry_t *)((ptr_t)entry + sizeof(list_entry_t) + len);
        bzero((void *)new_entry, list_slab_block(entry)->len - len);
        list_init(new_entry);
        // 新表项的长度为：list->len（总大小）- 头大小 - 要求分割的大小
        list_slab_block(new_entry)->len =
            list_slab_block(entry)->len - len - sizeof(list_entry_t);
        set_unused(new_entry);
        sb_manage.block_count += 1;
        list_add_after(entry, new_entry);
        // 重新设置旧链表信息
        list_slab_block(entry)->len = len;
        return new_entry;
    }
    return (list_entry_t *)NULL;
}

// 合并内存块
void slab_merge(list_entry_t *list) {
    list_entry_t *entry = list;
    // 合并后面的
    if (entry->next != list &&
        list_slab_block(entry->next)->allocated == SLAB_UNUSED) {
        list_entry_t *next = entry->next;
        list_slab_block(entry)->len +=
            list_slab_block(entry->next)->len + sizeof(list_entry_t);
        list_del(next);
        sb_manage.block_count -= 1;
    }
    // 合并前面的
    if (entry->prev != list &&
        list_slab_block(entry->prev)->allocated == SLAB_UNUSED) {
        list_entry_t *prev = entry->prev;
        list_slab_block(prev)->len +=
            (list_slab_block(entry)->len + sizeof(list_entry_t));
        list_del(entry);
        sb_manage.block_count -= 1;
    }
    return;
}

// 寻找符合要求的内存块，未找到返回 NULL
static inline list_entry_t *find_entry(size_t len);
list_entry_t *              find_entry(size_t len) {
    list_entry_t *entry = sb_manage.slab_list;
    do {
        // 查找符合长度且未使用，符合对齐要求的内存
        if ((list_slab_block(entry)->len >= len) &&
            (list_slab_block(entry)->allocated == SLAB_UNUSED)) {
            // 进行分割，这个函数会同时设置 entry 的信息
            slab_split(entry, len);
            return entry;
        }
        // 没找到的话就查找下一个
        else {
            entry = list_next(entry);
        }
    } while (list_next(entry) != sb_manage.slab_list);
    return (list_entry_t *)NULL;
}

// 申请新的内存页
// 参数分别为：虚拟地址起点，要申请的页数
static inline ptr_t alloc_page(ptr_t va, size_t page);
ptr_t               alloc_page(ptr_t va, size_t page) {
    ptr_t pa = pmm_alloc(page * VMM_PAGE_SIZE, NORMAL);
    if (pa == (ptr_t)NULL) {
        printk_err(
            "Error at slab.c ptr_t alloc_page(): no enough physical memory\n");
        return (ptr_t)NULL;
    }
    for (ptr_t va_start = va, pa_start = pa;
         pa_start < pa + VMM_PAGE_SIZE * page;
         pa_start += VMM_PAGE_SIZE, va_start += VMM_PAGE_SIZE) {
        // 如果当前线性地址没有映射
        if (get_mapping(pgd_kernel, va_start, (ptr_t *)NULL) == 0) {
            map(pgd_kernel, va_start, pa_start, VMM_PAGE_PRESENT | VMM_PAGE_RW);
        }
        else {
            // 如果有部分映射了，则全部 unmap
            ptr_t addr = (ptr_t)NULL;
            for (addr = va; addr < va_start; addr += VMM_PAGE_SIZE) {
                unmap(pgd_kernel, addr);
            }
            // 并重新计算 va 的值
            // 运行到这里时 addr 尚未被检测，所以下面的代码从 addr 开始
            // 计算方法：addr~addr+page*VMM_PAGE_SIZE 的 get_mapping 结果全部为
            // 0
            ptr_t tmp = addr;
            while (addr < tmp + page * VMM_PAGE_SIZE) {
                // 如果遇到映射过的
                if (get_mapping(pgd_kernel, addr, (ptr_t *)NULL) != 0) {
                    // 更新 addr 地址
                    addr += VMM_PAGE_SIZE;
                    tmp = addr;
                }
                addr += VMM_PAGE_SIZE;
            }
            // 这时 addr 就是符合要求的地址
            // 全部映射即可
            map(pgd_kernel, addr, pa_start, VMM_PAGE_PRESENT | VMM_PAGE_RW);
            return addr;
        }
    }
    bzero((void *)va, VMM_PAGE_SIZE * page);
    return va;
}

ptr_t alloc(size_t byte) {
    // 所有申请的内存长度(限制最小大小)加上管理头的长度
    size_t        len   = (byte > SLAB_MIN) ? byte : SLAB_MIN;
    list_entry_t *entry = find_entry(len);
    if (entry != NULL) {
        set_used(entry);
        return (ptr_t)((ptr_t)entry + sizeof(list_entry_t));
    }
    entry = list_prev(sb_manage.slab_list);
    // 如果执行到这里，说明没有可用空间了，那么申请新的内存页
    list_entry_t *new_entry;
    len += sizeof(list_entry_t);
    size_t pages = (len % VMM_PAGE_SIZE == 0) ? (len / VMM_PAGE_SIZE)
                                              : ((len / VMM_PAGE_SIZE) + 1);
    ptr_t va = alloc_page((ptr_t)((ptr_t)entry + sizeof(list_entry_t) +
                                  list_slab_block(entry)->len),
                          pages);
    if (va == (ptr_t)NULL) {
        printk_err(
            "Error at slab.c ptr_t alloc_align(): no enough physical memory\n");
        return (ptr_t)NULL;
    }
    new_entry = (list_entry_t *)va;
    list_init(new_entry);
    // 新表项的可用长度为减去头的大小
    list_slab_block(new_entry)->len =
        (ptr_t)(pages * VMM_PAGE_SIZE) - sizeof(list_entry_t);
    list_add_after(entry, new_entry);
    // 进行分割
    slab_split(new_entry, len);
    set_used(new_entry);
    return (ptr_t)((ptr_t)new_entry + sizeof(list_entry_t));
}

void free(ptr_t addr) {
    // 获取实际开始地址
    list_entry_t *entry = (list_entry_t *)((ptr_t)addr - sizeof(list_entry_t));
    if (list_slab_block(entry)->allocated != SLAB_USED) {
        printk_err("Error at slab.c void free(ptr_t)\n");
        return;
    }
    list_slab_block(entry)->allocated = SLAB_UNUSED;
    slab_merge(entry);
    return;
}

#ifdef __cplusplus
}
#endif
