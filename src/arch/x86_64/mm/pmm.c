
// This file is a part of SimpleXX/SimpleKernel
// (https://github.com/SimpleXX/SimpleKernel).
//
// pmm.c for SimpleXX/SimpleKernel.

#ifdef __cplusplus
extern "C" {
#endif

#include "stdio.h"

#include "string.h"
#include "assert.h"
#include "debug.h"
#include "cpu.hpp"
#include "sync.hpp"
#include "mem/pmm.h"
#include "mem/firstfit.h"

// 物理页帧数组长度,可用内存总页数
static uint32_t phy_pages_count = 0;

static const pmm_manage_t *pmm_manager = &firstfit_manage;

memory_zone   mem_zone[zone_sum];
physical_page mem_page[PMM_PAGE_MAX_SIZE];

// 从 GRUB 读取物理内存信息
static void pmm_get_ram_info(e820map_t *e820map);
void        pmm_get_ram_info(e820map_t *e820map) {
    for (; (uint8_t *)mmap_entries < (uint8_t *)mmap_tag + mmap_tag->size;
         mmap_entries =
             (multiboot_memory_map_entry_t
                  *)((uint32_t)mmap_entries +
                     ((struct multiboot_tag_mmap *)mmap_tag)->entry_size)) {
        // 如果是可用内存
        // printk("addr:%x%x,len:%x%x,type:%x:\n",mmap_entries->addr,mmap_entries->len,mmap_entries->type);
        if ((unsigned)mmap_entries->type ==
            MULTIBOOT_MEMORY_AVAILABLE) //&& (unsigned)(mmap_entries->addr &
                                        // 0xFFFFFFFF) == 0x100000
        {
            e820map->map[e820map->nr_map].addr   = mmap_entries->addr;
            e820map->map[e820map->nr_map].length = mmap_entries->len;
            e820map->map[e820map->nr_map].type   = mmap_entries->type;
            e820map->nr_map++;
        }
    }
    return;
}

void pmm_phy_init(e820map_t *e820map) {
    //分区页面总数
    uint32_t count_dma = 0, count_normal = 0, count_highmem = 0;
    //分区空闲页面总数
    uint32_t free_dma = 0, free_normal = 0, free_highmem = 0;
    /****************************/
    //初始化mem_map数组
    for (uint32_t i = 0; i < PMM_PAGE_MAX_SIZE; i++) {
        ptr_t address     = i * PMM_PAGE_SIZE;
        mem_page[i].start = address;
        mem_page[i].ref   = -1;
        if (address < (ptr_t)NORMAL_start_addr) //小于16MB
            count_dma++;
        else if (address < (ptr_t)HIGHMEM_start_addr) //大于16MB小于110MB
            count_normal++;
        else
            count_highmem++;
    }
    /****************************/
    // 计算可用内存段的物理页总数
    for (uint32_t i = 0; i < e820map->nr_map; i++) {
        // printk_info("addr:0x%X\n",&kernel_init_start);
        // printk_info("addr:0x%X\n",(ptr_t)&kernel_data_end);
        for (ptr_t addr = e820map->map[i].addr;
             addr < e820map->map[i].addr + e820map->map[i].length;
             addr += PMM_PAGE_SIZE) {
            /*******************************************************/
            //初始化可用内存段的物理页数组
            // ptr_t address=(ptr_t)addr;
            //地址对应的物理页数组下标
            uint32_t j = (addr & PMM_PAGE_MASK) / PMM_PAGE_SIZE;
            // mem_page[j].start=address;
            if (addr >= (ptr_t)&kernel_start &&
                addr <= ((ptr_t)&kernel_end - (ptr_t)0xc0000000))
                //内核已占用
                mem_page[j].ref = 1;
            else if (addr < (ptr_t)NORMAL_start_addr) //小于16MB
            {
                mem_page[j].zone = 0;
                mem_page[j].ref  = 0;
                free_dma++;
            }
            else if (addr < (ptr_t)HIGHMEM_start_addr) //大于16MB小于110MB
            {
                mem_page[j].zone = 1;
                mem_page[j].ref  = 0;
                free_normal++;
            }
            else {
                mem_page[j].zone = 2;
                mem_page[j].ref  = 0;
                free_highmem++;
            }

            phy_pages_count++;
        }
    }
    //设置分区的总页面和空闲页面信息
    mem_zone[DMA].all_pages = count_dma;
    printk_info("%d\n", mem_zone[DMA].all_pages);
    mem_zone[DMA].free_pages     = free_dma;
    mem_zone[NORMAL].all_pages   = count_normal;
    mem_zone[NORMAL].free_pages  = free_normal;
    mem_zone[HIGHMEM].all_pages  = count_highmem;
    mem_zone[HIGHMEM].free_pages = free_highmem;
    //分别设置分区的极值点和平衡条件
    for (int i = 0; i < zone_sum; i++) {
        mem_zone[i].pages_min    = mem_zone[i].all_pages / 3;
        mem_zone[i].pages_low    = mem_zone[i].all_pages / 2;
        mem_zone[i].pages_high   = mem_zone[i].all_pages * 2 / 3;
        mem_zone[i].need_balance = false;
    }
    /*******************************************************/
    return;
}

void pmm_mamage_init() {
    // 因为只有一个可用内存区域，所以直接传递
    pmm_manager->pmm_manage_init();
    return;
}

void pmm_init() {
    cpu_cli();
    // uint32_t cr0;
    // cr0 |= (0u << 31);
    //__asm__ volatile ("mov %0, %%cr0" : : "r" (cr0) );
    e820map_t e820map;
    bzero(&e820map, sizeof(e820map_t));
    pmm_get_ram_info(&e820map);
    pmm_phy_init(&e820map);
    pmm_mamage_init();
    // printk_info("pmm_init\n");
    // printk_info("phy_pages_count: %d\n", phy_pages_count);
    // printk_info("phy_pages_allow_count: %d\n", pmm_free_pages_count(DMA) );
    /********************************/
    printk_info("mem_DMA free_pages:%d\n", mem_zone[DMA].free_pages);
    printk_info("mem_DMA pages_min:%d\n", mem_zone[DMA].pages_min);
    printk_info("mem_DMA pages_low:%d\n", mem_zone[DMA].pages_low);
    printk_info("mem_DMA pages_high:%d\n", mem_zone[DMA].pages_high);
    printk_info("mem_DMA need_balance:%d\n", mem_zone[DMA].need_balance);
    printk_info("mem_DMA all_pages:%d\n", mem_zone[DMA].all_pages);
    printk_info("mem_NORMAL free_pages:%d\n", mem_zone[NORMAL].free_pages);
    printk_info("mem_NORMAL pages_min:%d\n", mem_zone[NORMAL].pages_min);
    printk_info("mem_NORMAL pages_low:%d\n", mem_zone[NORMAL].pages_low);
    printk_info("mem_NORMAL pages_high:%d\n", mem_zone[NORMAL].pages_high);
    printk_info("mem_NORMAL need_balance:%d\n", mem_zone[NORMAL].need_balance);
    printk_info("mem_NORMAL all_pages:%d\n", mem_zone[NORMAL].all_pages);
    printk_info("mem_HIGHMEM free_pages:%d\n", mem_zone[HIGHMEM].free_pages);
    printk_info("mem_HIGHMEM pages_min:%d\n", mem_zone[HIGHMEM].pages_min);
    printk_info("mem_HIGHMEM pages_low:%d\n", mem_zone[HIGHMEM].pages_low);
    printk_info("mem_HIGHMEM pages_high:%d\n", mem_zone[HIGHMEM].pages_high);
    printk_info("mem_HIGHMEM need_balance:%d\n",
                mem_zone[HIGHMEM].need_balance);
    printk_info("mem_HIGHMEM all_pages:%d\n", mem_zone[HIGHMEM].all_pages);
    /********************************/
    /*list_entry_t *head=ff_manage_dma.free_list;
    list_entry_t *p=head;
    while(p->next!=head)
    {
            printk_test("DMA Physical Addr: 0x%08X\n",p->chunk_info.addr);
            printk_test("DMA Physical pages: %d\n",p->chunk_info.npages);
            printk_test("DMA Physical ref: %d\n",p->chunk_info.ref);
            printk_test("DMA Physical flag: %d\n",p->chunk_info.flag);
    }*/
    cpu_sti();
    return;
}

ptr_t pmm_alloc(uint32_t byte, char zone) {
    ptr_t page;
    page = pmm_manager->pmm_manage_alloc(byte, zone);
    return page;
}

void pmm_free_page(ptr_t addr, uint32_t byte, char zone) {
    pmm_manager->pmm_manage_free(addr, byte, zone);
    return;
}

uint32_t pmm_free_pages_count(char zone) {
    return pmm_manager->pmm_manage_free_pages_count(zone);
}

#ifdef __cplusplus
}
#endif
