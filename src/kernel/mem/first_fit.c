
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
//
// first_fit.c for MRNIU/SimpleKernel.

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"
#include "stdio.h"
#include "string.h"
#include "assert.h"
#include "mem/firstfit.h"

#define FF_USED         (0x00)
#define FF_UNUSED       (0x01)

static void init();
static ptr_t alloc(uint32_t bytes,char zone);
static void free(ptr_t addr_start, uint32_t bytes,char zone);
static uint32_t free_pages_count(char zone);

pmm_manage_t firstfit_manage = {
	"Fitst Fit",
	&init,
	&alloc,
	&free,
	&free_pages_count
};


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



/**********************************/
//分区管理，定义3个管理器，均使用first-fit算法
// First Fit 算法需要的信息
//static firstfit_manage_t ff_manage;
//static firstfit_manage_t ff_manage_dma;
//static firstfit_manage_t ff_manage_normal;
//static firstfit_manage_t ff_manage_highmem;

// 物理页信息保存地址
static list_entry_t * pmm_info = NULL;
/*****************************************/
//管理器信息也需要物理页进行存储，所以这些页面也需要被设置为已引用
/*****************************************/
void init() {
	// 位于内核结束后，大小为 (PMM_MAX_SIZE / PMM_PAGE_SIZE)*sizeof(list_entry_t)
	// 后面的操作是进行页对齐
	/****************************/
	//每个分区初始化一个管理器

/*
	normal_pmm_info=(list_entry_t *)((ptr_t)(0x1000000));
	uint32_t normal_pmm_info_size = mem_zone[NORMAL].free_pages * sizeof(list_entry_t);
	bzero(normal_pmm_info,normal_pmm_info_size);
	highmem_pmm_info=(list_entry_t *)((ptr_t)(0x6e00000));
	uint32_t highmem_pmm_info_size = mem_zone[HIGHMEM].free_pages * sizeof(list_entry_t);
	bzero(highmem_pmm_info,highmem_pmm_info_size);
*/	
	printk_info("%d\n",mem_zone[DMA].all_pages);
	//将DMA区域的空闲链表放在地址为0的位置,NORMAL区域的空闲链表放在16MB处，HIGHMEM放在110MB处
	list_entry_t *dma_pmm_info=(list_entry_t *)((ptr_t)(DMA_start_addr));
	list_entry_t *normal_pmm_info=(list_entry_t *)((ptr_t)(NORMAL_start_addr));
	list_entry_t *highmem_pmm_info=(list_entry_t *)((ptr_t)(HIGHMEM_start_addr));
	//最差情况，一块只有一个页，所以预先留好空间存储这些块信息
	uint32_t dma_pmm_info_size = mem_zone[DMA].all_pages * sizeof(list_entry_t);
	uint32_t normal_pmm_info_size = mem_zone[NORMAL].all_pages * sizeof(list_entry_t);
	uint32_t highmem_pmm_info_size = mem_zone[HIGHMEM].all_pages * sizeof(list_entry_t);
	printk_info("normal_need_page:%d\n",normal_pmm_info_size/PMM_PAGE_SIZE);
	printk_info("normal_need_page:%d\n",normal_pmm_info_size);
	//printk_info("addr:0x%08X\n",normal_pmm_info);
	//printk_info("dma_need_page:%d\n",dma_pmm_info_size/PMM_PAGE_SIZE);
	//printk_info("dma_need_page:%d\n",dma_pmm_info_size);
	//printk_info("addr:0x%08X\n",dma_pmm_info);
	//printk_info("successful1!\n");	
	//printk_info("successful2!\n");
	bzero(dma_pmm_info,dma_pmm_info_size);
	//printk_info("successful2!\n");
	bzero(normal_pmm_info,normal_pmm_info_size);
	//printk_info("successful3!\n");
	bzero(highmem_pmm_info,highmem_pmm_info_size);
	//printk_info("successful4!\n");
	//list_entry_t *dma_pmm_info_head;
	//mem_page数组的指示变量
	uint32_t k=0;
	//根据dma区域的总页数和mem_page数组映射关系确定 某个物理页是否空闲
	for(uint32_t z=0;z<zone_sum;z++)
	{
		//初始化DMA区域的空闲链表，当有连续的空闲页时，进行合并
		uint32_t i=0;
		//记录节点数
		uint32_t num=0;
		//count作为计数器，记录连续空闲页的个数
		int count=0;
		//flag1、flag2作为标志变量，
		int flag1=0,flag2=0;
		ptr_t addr=0;
		//first=true代表该节点是第一个节点，否则不是
		bool first=true;
		//中转节点
		list_entry_t *before;
		ptr_t info_addr;
		//头节点
		list_entry_t *pmm_info_head;
		//中间节点
		list_entry_t *pmm_info_node;
		if(z==DMA)
			info_addr=(ptr_t)DMA_start_addr;
		else if(z==NORMAL)
			info_addr=(ptr_t)NORMAL_start_addr;
		else if(z==HIGHMEM)
			info_addr=(ptr_t)HIGHMEM_start_addr;
		//printk_info("successful!\n");
		/*****************************/
		/*        
				需要解决两种情况      
				1、一个可用内存页接一个不可用内存页，此时需创建节点，通过标志变量flag2和计数器count解决
				2、可用内存页需进行合并，且地址为第一个可用内存页的地址，通过标志变量flag1解决
		*/
		/*****************************/
		while(i<mem_zone[z].all_pages)
		{
			//如果该页可用且空闲
			if(!mem_page[k].ref)
			{
				//记录该页前是否有空闲页，若有则地址为前面空闲页地址，然后计数加1
				if(!flag1)
					addr=mem_page[k].start;
				count++;
				flag1=1;
				flag2=1;
				//到达分区最后一页
				if(i==mem_zone[z].all_pages-1)
					flag2=0;
			}
			else
			{
				flag1=0;
				flag2=0;
			}
			i++;
			k++;	
			if(!flag2&&count)
			{
				if(first)
				{
					pmm_info_head=(list_entry_t *)(info_addr);
					//uint32_t new_dma_nfo_size =  sizeof(list_entry_t);
					bzero(pmm_info_head, sizeof(list_entry_t));
					pmm_info_head->chunk_info.addr=addr;
					pmm_info_head->chunk_info.npages=count;
					pmm_info_head->chunk_info.ref=0;
					pmm_info_head->chunk_info.flag=FF_UNUSED;
					info_addr+=sizeof(list_entry_t);
					first=false;
					list_init_head(pmm_info_head);
					before=pmm_info_head;
					num++;
				}
				else
				{
					pmm_info_node=(list_entry_t *)(info_addr);
					//uint32_t new_dma_info_size =  sizeof(list_entry_t);
					bzero(pmm_info_node, sizeof(list_entry_t));
					pmm_info_node->chunk_info.addr=addr;
					pmm_info_node->chunk_info.npages=count;
					pmm_info_node->chunk_info.ref=0;
					pmm_info_node->chunk_info.flag=FF_UNUSED;
					info_addr+=sizeof(list_entry_t);
					list_add_after(before,pmm_info_node);
					before=pmm_info_node;
					num++;
				}
				count=0;
			}
		}
		if(z==DMA)
		{
			ff_manage_dma.phy_page_count=mem_zone[z].all_pages;
			ff_manage_dma.phy_page_now_count=mem_zone[z].free_pages;
			ff_manage_dma.node_num=num;
			ff_manage_dma.free_list=pmm_info_head;//(list_entry_t *)(ptr_t)DMA_start_addr;
		}
		else if(z==NORMAL)
		{
			ff_manage_normal.phy_page_count=mem_zone[z].all_pages;
			ff_manage_normal.phy_page_now_count=mem_zone[z].free_pages;
			ff_manage_normal.node_num=num;
			ff_manage_normal.free_list=pmm_info_head;//(list_entry_t *)(ptr_t)NORMAL_start_addr;
		}
		else if(z==HIGHMEM)
		{
			ff_manage_highmem.phy_page_count=mem_zone[z].all_pages;
			ff_manage_highmem.phy_page_now_count=mem_zone[z].free_pages;
			ff_manage_highmem.node_num=num;
			ff_manage_highmem.free_list=pmm_info_head;//(list_entry_t *)(ptr_t)HIGHMEM_start_addr;		
		}

	}
	/*
	//打印DMA区域链表
	list_entry_t *head=ff_manage_dma.free_list;
	//打印头节点
	printk_test("DMA Physical Addr: 0x%08X\n",head->chunk_info.addr);
	printk_test("DMA Physical pages: %d\n",head->chunk_info.npages);
	printk_test("DMA Physical ref: %d\n",head->chunk_info.ref);
	printk_test("DMA Physical flag: %d\n",head->chunk_info.flag);
	list_entry_t *p=head->next;
	while(p!=head)
	{
		printk_test("DMA Physical Addr: 0x%08X\n",p->chunk_info.addr);
		printk_test("DMA Physical pages: %d\n",p->chunk_info.npages);
		printk_test("DMA Physical ref: %d\n",p->chunk_info.ref);
		printk_test("DMA Physical flag: %d\n",p->chunk_info.flag);
		p=p->next;
	}
	
	//打印NORMAL区域链表
	head=ff_manage_normal.free_list;
	//打印头节点
	printk_test("NORMAL Physical Addr: 0x%08X\n",head->chunk_info.addr);
	printk_test("NORMAL Physical pages: %d\n",head->chunk_info.npages);
	printk_test("NORMAL Physical ref: %d\n",head->chunk_info.ref);
	printk_test("NORMAL Physical flag: %d\n",head->chunk_info.flag);
	p=head->next;
	while(p!=head)
	{
		printk_test("NORMAL Physical Addr: 0x%08X\n",p->chunk_info.addr);
		printk_test("NORMAL Physical pages: %d\n",p->chunk_info.npages);
		printk_test("NORMAL Physical ref: %d\n",p->chunk_info.ref);
		printk_test("NORMAL Physical flag: %d\n",p->chunk_info.flag);
		p=p->next;
	}
	//打印HIGHMEM区域链表
	head=ff_manage_highmem.free_list;
	//打印头节点
	printk_test("HIGHMEM Physical Addr: 0x%08X\n",head->chunk_info.addr);
	printk_test("HIGHMEM Physical pages: %d\n",head->chunk_info.npages);
	printk_test("HIGHMEM Physical ref: %d\n",head->chunk_info.ref);
	printk_test("HIGHMEM Physical flag: %d\n",head->chunk_info.flag);
	p=head->next;
	while(p!=head)
	{
		printk_test("HIGHMEM Physical Addr: 0x%08X\n",p->chunk_info.addr);
		printk_test("HIGHMEM Physical pages: %d\n",p->chunk_info.npages);
		printk_test("HIGHMEM Physical ref: %d\n",p->chunk_info.ref);
		printk_test("HIGHMEM Physical flag: %d\n",p->chunk_info.flag);
		p=p->next;
	}
	*/
	printk_info("successful-final!\n");
	/****************************/

/*	pmm_info = (list_entry_t *)( ( (ptr_t)(&kernel_end + PMM_PAGE_SIZE) & PMM_PAGE_MASK) );
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
*/
	return;
}

//根据线性地址判断属于那个管理区，然后使用对应的物理分区管理器进行分配。
ptr_t alloc(uint32_t bytes,char zone) {
	// 计算需要的页数
	size_t pages = bytes / PMM_PAGE_SIZE;
	// 不足一页的+1
	if(bytes % PMM_PAGE_SIZE != 0) {
		pages++;
	}

	// 首先根据分区找到对应的管理器，然后找到地址对应的管理节点
	firstfit_manage_t ff_manage;
	if(zone==DMA)
		ff_manage = ff_manage_dma;
	else if(zone==NORMAL)
		ff_manage = ff_manage_normal;
	else if(zone==HIGHMEM)
		ff_manage = ff_manage_highmem;
	list_entry_t *entry=ff_manage.free_list;
	//printk_info("successful-1!\n");
	while(entry >=0) {
		//printk_info("successful-2!\n");
		// 查找符合长度且未使用的内存
		if( (list_chunk_info(entry)->npages >= pages) && (list_chunk_info(entry)->flag == FF_UNUSED) ) {
			//printk_info("successful-2!\n");
			// 如果剩余大小足够
			if(list_chunk_info(entry)->npages - pages > 1) {
				// 添加新的链表项
				list_entry_t * tmp = (list_entry_t *)(entry + ff_manage.node_num*sizeof(list_entry_t) );
				list_chunk_info(tmp)->addr = entry->chunk_info.addr + pages * PMM_PAGE_SIZE;
				list_chunk_info(tmp)->npages =  entry->chunk_info.npages - pages;
				list_chunk_info(tmp)->ref = 0;
				list_chunk_info(tmp)->flag = FF_UNUSED;
				list_add_after(entry, tmp);
			}
			//printk_info("successful-3!\n");
			// 不够的话直接分配
			list_chunk_info(entry)->npages = pages;
			list_chunk_info(entry)->ref = 1;
			list_chunk_info(entry)->flag = FF_USED;
			ff_manage.phy_page_now_count -= pages;
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
	}
	printk_err("Error at firstfit.c: ptr_t alloc(uint32_t)\n");
	return (ptr_t)NULL;
}

void free(ptr_t addr_start, uint32_t bytes,char zone) {
	// 计算需要的页数
	size_t pages = bytes / PMM_PAGE_SIZE;
	// 不足一页的+1
	if(bytes % PMM_PAGE_SIZE != 0) {
		pages++;
	}
	// 首先根据分区找到对应的管理器，然后找到地址对应的管理节点
	firstfit_manage_t *ff_manage;
	if(zone==DMA)
		ff_manage = &ff_manage_dma;
	else if(zone==NORMAL)
		ff_manage = &ff_manage_normal;
	else if(zone==HIGHMEM)
		ff_manage = &ff_manage_highmem;
	list_entry_t * entry = ff_manage->free_list;
	while( ( (entry = list_next(entry) ) != ff_manage->free_list) && (list_chunk_info(entry)->addr != addr_start) );

	// 释放所有页
	list_chunk_info(entry)->ref = 0;
	list_chunk_info(entry)->flag = FF_UNUSED;

	// 如果于相邻链表有空闲的则合并
	// 后面
	if(entry->next != entry && list_chunk_info(entry->next)->flag == FF_UNUSED) {
		list_entry_t * next = entry->next;
		list_chunk_info(entry)->npages += list_chunk_info(next)->npages;
		list_chunk_info(next)->npages = 0;
		list_del(next);
	}
	// 前面
	if(entry->prev != entry && list_chunk_info(entry->prev)->flag == FF_UNUSED) {
		list_entry_t * prev = entry->prev;
		list_chunk_info(prev)->npages += list_chunk_info(entry)->npages;
		list_chunk_info(entry)->npages = 0;
		list_del(entry);
	}
	ff_manage->phy_page_now_count += pages;
	return;
}

uint32_t free_pages_count(char zone) {
	if(zone==DMA)
		return ff_manage_dma.phy_page_now_count;
	else if(zone==NORMAL)
		return ff_manage_normal.phy_page_now_count;
	else if(zone==HIGHMEM)
		return ff_manage_highmem.phy_page_now_count;
	
}

#ifdef __cplusplus
}
#endif
