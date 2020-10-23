
// This file is a part of SimpleXX/SimpleKernel
// (https://github.com/SimpleXX/SimpleKernel).
//
// pmm.c for SimpleXX/SimpleKernel.

#ifdef __cplusplus
extern "C" {
#endif

#include "stdio.h"
#include "string.h"
#include "pmm.h"
#include "firstfit.h"

static const pmm_manage_t *pmm_manager = &firstfit_manage;

memory_zone_mamage_t mem_zone[ZONE_SUM];
physical_page_t      mem_page[PMM_PAGE_MAX_SIZE];

// TODO: 太难看了也
void pmm_get_ram_info(e820map_t *e820map) {
    for (; (uint8_t *)mmap_entries < (uint8_t *)mmap_tag + mmap_tag->size;
         mmap_entries =
             (multiboot_memory_map_entry_t
                  *)((uint32_t)mmap_entries +
                     ((struct multiboot_tag_mmap *)mmap_tag)->entry_size)) {
        // 如果是可用内存
        // printk("addr:%x%x,len:%x%x,type:%x:\n",mmap_entries->addr,mmap_entries->len,mmap_entries->type);
        if ((unsigned)mmap_entries->type == MULTIBOOT_MEMORY_AVAILABLE) {
            e820map->map[e820map->nr_map].addr   = mmap_entries->addr;
            e820map->map[e820map->nr_map].length = mmap_entries->len;
            e820map->map[e820map->nr_map].type   = mmap_entries->type;
            e820map->nr_map++;
        }
    }
    return;
}

void pmm_zone_init(e820map_t *e820map) {
    // 分区页面总数
    uint32_t count_dma     = 0;
    uint32_t count_normal  = 0;
    uint32_t count_highmem = 0;
    // 分区空闲页面总数
    uint32_t free_dma     = 0;
    uint32_t free_normal  = 0;
    uint32_t free_highmem = 0;
    // 初始化 mem_map 数组
    // 统计所有内存
    for (uint32_t i = 0; i < PMM_PAGE_MAX_SIZE; i++) {
        ptr_t address     = i * PMM_PAGE_SIZE;
        mem_page[i].start = address;
        mem_page[i].ref   = -1;
        // 小于 16MB
        if (address < (ptr_t)NORMAL_START_ADDR) {
            count_dma++;
        }
        // 大于 16MB 小于 110MB
        else if (address < (ptr_t)HIGHMEM_START_ADDR) {
            count_normal++;
        }
        // 110MB 至结束
        else {
            count_highmem++;
        }
    }
    // 计算可用的内存
    for (size_t i = 0; i < e820map->nr_map; i++) {
        // printk_info("addr:0x%X\n",&kernel_init_start);
        // printk_info("addr:0x%X\n",(ptr_t)&kernel_data_end);
        for (ptr_t addr = e820map->map[i].addr;
             addr < e820map->map[i].addr + e820map->map[i].length;
             addr += PMM_PAGE_SIZE) {
            // 初始化可用内存段的物理页数组
            // 地址对应的物理页数组下标
            size_t j = (addr & PMM_PAGE_MASK) / PMM_PAGE_SIZE;
            // 内核已占用部分
            if (addr >= (ptr_t)&kernel_start && addr <= ((ptr_t)&kernel_end)) {
                mem_page[j].ref = 1;
            }
            // 小于 16MB
            else if (addr < (ptr_t)NORMAL_START_ADDR) {
                mem_page[j].zone = DMA;
                mem_page[j].ref  = 0;
                free_dma++;
            }
            // 大于 16MB 小于 110MB
            else if (addr < (ptr_t)HIGHMEM_START_ADDR) {
                mem_page[j].zone = NORMAL;
                mem_page[j].ref  = 0;
                free_normal++;
            }
            // 110MB 至结束
            else {
                mem_page[j].zone = HIGHMEM;
                mem_page[j].ref  = 0;
                free_highmem++;
            }
        }
    }
    // 设置分区的总页面和空闲页面信息
    mem_zone[DMA].all_pages = count_dma;
    printk_info("%d\n", mem_zone[DMA].all_pages);
    mem_zone[DMA].free_pages     = free_dma;
    mem_zone[NORMAL].all_pages   = count_normal;
    mem_zone[NORMAL].free_pages  = free_normal;
    mem_zone[HIGHMEM].all_pages  = count_highmem;
    mem_zone[HIGHMEM].free_pages = free_highmem;
    // 分别设置分区的极值点和平衡条件
    for (int i = 0; i < ZONE_SUM; i++) {
        mem_zone[i].pages_min    = mem_zone[i].all_pages / 3;
        mem_zone[i].pages_low    = mem_zone[i].all_pages / 2;
        mem_zone[i].pages_high   = mem_zone[i].all_pages * 2 / 3;
        mem_zone[i].need_balance = false;
    }
    return;
}

void pmm_mamage_init() {
    // 因为只有一个可用内存区域，所以直接传递
    pmm_manager->pmm_manage_init();
    return;
}

void pmm_init() {
    e820map_t e820map;
    bzero(&e820map, sizeof(e820map_t));
    pmm_get_ram_info(&e820map);
    pmm_zone_init(&e820map);
    pmm_mamage_init();
    printk_info("mem_DMA free_pages: 0x%X\n", mem_zone[DMA].free_pages);
    printk_info("mem_DMA pages_min: 0x%X\n", mem_zone[DMA].pages_min);
    printk_info("mem_DMA pages_low: 0x%X\n", mem_zone[DMA].pages_low);
    printk_info("mem_DMA pages_high: 0x%X\n", mem_zone[DMA].pages_high);
    printk_info("mem_DMA need_balance: 0x%X\n", mem_zone[DMA].need_balance);
    printk_info("mem_DMA all_pages: 0x%X\n", mem_zone[DMA].all_pages);
    printk_info("mem_NORMAL free_pages: 0x%X\n", mem_zone[NORMAL].free_pages);
    printk_info("mem_NORMAL pages_min: 0x%X\n", mem_zone[NORMAL].pages_min);
    printk_info("mem_NORMAL pages_low: 0x%X\n", mem_zone[NORMAL].pages_low);
    printk_info("mem_NORMAL pages_high: 0x%X\n", mem_zone[NORMAL].pages_high);
    printk_info("mem_NORMAL need_balance: 0x%X\n",
                mem_zone[NORMAL].need_balance);
    printk_info("mem_NORMAL all_pages: 0x%X\n", mem_zone[NORMAL].all_pages);
    printk_info("mem_HIGHMEM free_pages: 0x%X\n", mem_zone[HIGHMEM].free_pages);
    printk_info("mem_HIGHMEM pages_min: 0x%X\n", mem_zone[HIGHMEM].pages_min);
    printk_info("mem_HIGHMEM pages_low: 0x%X\n", mem_zone[HIGHMEM].pages_low);
    printk_info("mem_HIGHMEM pages_high: 0x%X\n", mem_zone[HIGHMEM].pages_high);
    printk_info("mem_HIGHMEM need_balance: 0x%X\n",
                mem_zone[HIGHMEM].need_balance);
    printk_info("mem_HIGHMEM all_pages: 0x%X\n", mem_zone[HIGHMEM].all_pages);
    printk_info("Total free phy pages: 0x%X\n",
                mem_zone[DMA].free_pages + mem_zone[NORMAL].free_pages +
                    mem_zone[HIGHMEM].free_pages);
    printk_info("pmm_init\n");
    return;
}

ptr_t pmm_alloc(uint32_t byte, int8_t zone) {
    ptr_t page;
    page = pmm_manager->pmm_manage_alloc(byte, zone);
    return page;
}

ptr_t pmm_alloc_page(uint32_t pages, int8_t zone) {
    ptr_t page;
    page = pmm_manager->pmm_manage_alloc(PMM_PAGE_SIZE * pages, zone);
    return page;
}

void pmm_free(ptr_t addr, uint32_t byte, int8_t zone) {
    pmm_manager->pmm_manage_free(addr, byte, zone);
    return;
}

void pmm_free_page(ptr_t addr, uint32_t pages, int8_t zone) {
    pmm_manager->pmm_manage_free(addr, pages * PMM_PAGE_SIZE, zone);
    return;
}

uint32_t pmm_free_pages_count(int8_t zone) {
    return pmm_manager->pmm_manage_free_pages_count(zone);
}

#ifdef __cplusplus
}
#endif
