
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
//
// firstfit.c for MRNIU/SimpleKernel.

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"
#include "stdio.h"
#include "assert.h"
#include "mem/firstfit.h"

#define FF_USED 0x00
#define FF_UNUSED 0x01

// 块
typedef
    struct chunk_info {
	// 当前页的地址
	ptr_t		addr;
	// 拥有多少个连续的页
	uint32_t	npages;
	// 物理页被引用的次数
	int32_t		ref;
	// 当前页状态
	uint32_t	flag;
} chunk_info_t;

// 一个仅在这里使用的简单循环链表
typedef
    struct list_entry {
	chunk_info_t			chunk_info;
	struct list_entry *		next;
	struct list_entry *		prev;
} list_entry_t;

static inline void list_init_head(list_entry_t * list);

// 在中间添加元素
static inline void list_add_middle(list_entry_t * prev,  list_entry_t * next, list_entry_t * new);

// 在 prev 后添加项
static inline void list_add_after(list_entry_t * prev, list_entry_t * new);

// 在 next 前添加项
static inline void list_add_before(list_entry_t * next, list_entry_t * new);

// 删除元素
static inline void list_del(list_entry_t * list);

// 返回前面的元素
static inline list_entry_t * list_prev(list_entry_t * list);

// 返回后面的的元素
static inline list_entry_t * list_next(list_entry_t * list);

// 返回 chunk_info
static inline chunk_info_t * list_chunk_info(list_entry_t * list);

// 初始化
void list_init_head(list_entry_t * list) {
	list->next = list;
	list->prev = list;
	return;
}

// 在中间添加元素
void list_add_middle(list_entry_t * prev,  list_entry_t * next, list_entry_t * new) {
	// printk_debug("------list_add_middle------\n");
	next->prev = new;
	new->next = next;
	new->prev = prev;
	prev->next = new;
}

// 在 prev 后添加项
void list_add_after(list_entry_t * prev, list_entry_t * new) {
	list_add_middle(prev, prev->next, new);
	return;
}

// 在 next 前添加项
void list_add_before(list_entry_t * next, list_entry_t * new) {
	// printk_debug("------list_add_before------\n");
	list_add_middle(next->prev, next, new);
	return;
}

// 删除元素
void list_del(list_entry_t * list) {
	list->next->prev = list->prev;
	list->prev->next = list->next;
	return;
}

// 返回前面的元素
list_entry_t * list_prev(list_entry_t * list) {
	return list->prev;
}

// 返回后面的的元素
list_entry_t * list_next(list_entry_t * list) {
	return list->next;
}

// 返回 chunk_info
chunk_info_t * list_chunk_info(list_entry_t * list) {
	return &(list->chunk_info);
}

typedef
    struct firstfit_manage {
	// 物理内存起始地址
	ptr_t			pmm_addr_start;
	// 物理内存结束地址
	ptr_t			pmm_addr_end;
	// 物理内存页的总数量
	uint32_t		phy_page_count;
	// 物理内存页的当前数量
	uint32_t		phy_page_now_count;
	// 空闲链表
	list_entry_t *		free_list;
} firstfit_manage_t;

// First Fit 算法需要的信息
static firstfit_manage_t ff_manage;

// 物理页信息保存地址
static list_entry_t * pmm_info = NULL;

static void init(ptr_t page_start, uint32_t page_count) {
	// 位于内核结束后，大小为 (PMM_MAX_SIZE / PMM_PAGE_SIZE)*sizeof(list_entry_t)
	// 后面的操作是进行页对齐
	pmm_info = (list_entry_t *)( ( (ptr_t)(&kernel_end + PMM_PAGE_SIZE) & PMM_PAGE_MASK) );
	uint32_t pmm_info_size = (PMM_MAX_SIZE / PMM_PAGE_SIZE) * sizeof(list_entry_t);
	bzero(pmm_info, pmm_info_size);

	// 越过内核使用的内存
	pmm_info->chunk_info.addr = page_start + KERNEL_SIZE;
	pmm_info->chunk_info.npages = page_count - PMM_PAGES_KERNEL;
	pmm_info->chunk_info.ref = 0;
	pmm_info->chunk_info.flag = FF_UNUSED;

	ff_manage.phy_page_count = page_count;
	ff_manage.phy_page_now_count = page_count - PMM_PAGES_KERNEL;
	// 0x100000
	ff_manage.pmm_addr_start = page_start + KERNEL_SIZE;
	// 0x1FFF0000
	ff_manage.pmm_addr_end = page_start + page_count * PMM_PAGE_SIZE;
	ff_manage.free_list = pmm_info;
	list_init_head(ff_manage.free_list);
	return;
}

static ptr_t alloc(uint32_t bytes) {
	uint32_t pages = bytes / PMM_PAGE_SIZE;
	if(bytes % PMM_PAGE_SIZE != 0) {
		pages++;
	}
	list_entry_t * entry = ff_manage.free_list;
	while(entry != NULL) {
		// 查找符合长度且未使用的内存
		if( (list_chunk_info(entry)->npages >= pages) && (list_chunk_info(entry)->flag == FF_UNUSED) ) {
			// 如果剩余大小足够
			if(list_chunk_info(entry)->npages - pages > 1) {
				printk_debug("------if start------\n");
				printk_debug("addr: 0x%08X\n", (list_chunk_info(entry)->addr) );
				printk_debug("addr+pages * PMM_PAGE_SIZE: 0x%08X\n", (list_chunk_info(entry)->addr + pages * PMM_PAGE_SIZE) );
				// 添加新的链表项
				list_entry_t * tmp = (list_entry_t *)(entry + sizeof(list_entry_t) );
				list_chunk_info(tmp)->addr = entry->chunk_info.addr + pages * PMM_PAGE_SIZE;
				list_chunk_info(tmp)->npages =  entry->chunk_info.npages - pages;
				list_chunk_info(tmp)->ref = 0;
				list_chunk_info(tmp)->flag = FF_UNUSED;
				list_add_before(entry, tmp);
				printk_debug("------if end------\n");
			}
			// 不够的话直接分配
			list_chunk_info(entry)->npages = pages;
			list_chunk_info(entry)->ref = 1;
			list_chunk_info(entry)->flag = FF_USED;
			return list_chunk_info(entry)->addr;
		}
		// 没找到的话就查找下一个
		else if(list_next(entry) != ff_manage.free_list) {
			entry = list_next(entry);
		}
		else {
			printk_err("Error at firstfit.c: ptr_t alloc(uint32_t)\n");
			break;
		}
		return (ptr_t)NULL;
	}
}

static void free(ptr_t addr_start UNUSED, uint32_t bytes UNUSED) {
	return;
}

static uint32_t free_pages_count(void) {
	return 0;
}

pmm_manage_t firstfit_manage = {
	"Fitst Fit",
	&init,
	&alloc,
	&free,
	&free_pages_count
};

#ifdef __cplusplus
}
#endif
