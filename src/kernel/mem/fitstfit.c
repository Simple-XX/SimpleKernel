
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

// 物理页类型
typedef
    struct page_info {
	// 当前页的地址
	ptr_t		addr;
	// 物理页被引用的次数
	int32_t		ref;
	// 当前页状态
	uint32_t	flag;
} page_info_t;

// 一个仅在这里使用的简单循环链表
typedef
    struct list_entry {
	page_info_t        page_info;
	struct list_entry * next;
	struct list_entry * prev;
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

// 初始化
void list_init_head(list_entry_t * list) {
	list->next = list;
	list->prev = list;
	return;
}

// 在中间添加元素
void list_add_middle(list_entry_t * prev,  list_entry_t * next, list_entry_t * new) {
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
	list_add_middle(next->prev, next, new);
	return;
}

// 删除元素
void list_del(list_entry_t * list) {
	list->next->prev = list->prev;
	list->prev->next = list->next;
	return;
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
	list_entry_t		free_list;
} firstfit_manage_t;

static firstfit_manage_t ff_manage;

static void init(ptr_t page_start, uint32_t page_count) {
	ff_manage.phy_page_count = page_count;
	ff_manage.phy_page_now_count = 0;

	// 0x100000
	ff_manage.pmm_addr_start = page_start;
	// 0x1FFF0000
	ff_manage.pmm_addr_end = page_start + page_count * PMM_PAGE_SIZE;

	list_init_head(&ff_manage.free_list);
	ff_manage.free_list.page_info.addr = page_start;
	ff_manage.free_list.page_info.ref = 0;
	ff_manage.free_list.page_info.flag = 0;
	// printk_debug("page_start: 0x%08X\n", page_start);
	// printk_debug("page_end: 0x%08X\n", page_start + (page_count * PMM_PAGE_SIZE) );
	// 将每个页地址加入空闲链表中
	// for(ptr_t addr = page_start + PMM_PAGE_SIZE ; addr < (page_start + (page_count * PMM_PAGE_SIZE) ) ;
	for(ptr_t addr = page_start + PMM_PAGE_SIZE ; addr < (page_start + (11 * PMM_PAGE_SIZE) ) ;
	    addr += PMM_PAGE_SIZE) {
		// printk_debug("sdsds\n");
		list_entry_t entry;
		entry.page_info.addr = addr;
		// printk_test("addr!!: 0X%08X\n", addr);
		entry.page_info.ref = 0;
		entry.page_info.flag = 0;
		list_add_before(&ff_manage.free_list, &entry);
		ff_manage.phy_page_now_count++;
	}

	list_entry_t * ttt = &ff_manage.free_list;
	for(int i = 0 ; i < 10 ; i++, ttt = ttt->next) {
		printk_test("addr: 0X%08X\n", ttt->page_info.addr);
	}

	return;
}

static ptr_t alloc(uint32_t bytes UNUSED) {
	return 0;
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
