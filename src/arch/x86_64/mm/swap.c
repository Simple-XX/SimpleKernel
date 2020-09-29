#ifdef __cplusplus
extern "C" {
#endif

#include "mem/swap.h"
#include "stdio.h"
#include "cpu.hpp"
#include "sync.hpp"
#include "mem/slab.h"
#include "string.h"
#include "mem/pmm.h"
#include "mem/vmm.h"
//保留换到硬盘中的物理页号同样存储到队列中,等以后换到内存
static struct disk_page disk_queue[MAX_DISK_PAGE];
//内存中分配的页面队列
static int memory_queue[MAX_MEMORY_PAGE];
//内存中的页面数量
static int mm_num=0;
//硬盘中的页面数量
static int dk_num=0;
void initQueue()  //初始化
{
    for(int i=0;i<MAX_DISK_PAGE;i++){
        disk_queue[i].disk_num=0;
        disk_queue[i].page_num=0;
    }
    for(int i=0;i<MAX_MEMORY_PAGE;i++){
        memory_queue[i]=0;
    }
}
void enQueue(int page_num)//入队
{
    if(mm_num<=MAX_MEMORY_PAGE){
        memory_queue[mm_num++]=page_num;
    }
}
void deQueue() //出队
{
    //将内存队列头部的对应页号送入保存队列
    if(dk_num<=MAX_DISK_PAGE){
        disk_queue[dk_num++].page_num=memory_queue[0];
        mm_num--;
    }
    //获取换到硬盘的页号
    int k=memory_queue[0];
    //printk_info("k= \n");
    //将数据移到硬盘中，并记录对应的硬盘块号 
    /*   待补充    */
    //物理页清零，属性设置为可用
    ptr_t addr=k*PMM_PAGE_SIZE;
    //printk_info("addr:%08X\n",addr);
    if(addr<NORMAL_start_addr){
        //printk_info("addr:%08X\n",(ptr_t)VMM_PA_LA(addr));
        pmm_free_page((ptr_t)VMM_PA_LA(addr),PMM_PAGE_SIZE,DMA);
        bzero((ptr_t)VMM_PA_LA(addr),PMM_PAGE_SIZE);
    }
    else if(addr<HIGHMEM_start_addr){
        pmm_free_page((ptr_t)VMM_PA_LA(addr),PMM_PAGE_SIZE,NORMAL);
        bzero((ptr_t)VMM_PA_LA(addr),PMM_PAGE_SIZE);
    }
    else{
        pmm_free_page((ptr_t)VMM_PA_LA(addr),PMM_PAGE_SIZE,HIGHMEM);
        bzero((ptr_t)VMM_PA_LA(addr),PMM_PAGE_SIZE);
    }
         
    //free(k*PM)
    //kfree((ptr_t)temp);
    //队列向前进一步
    for(int i=0;i<mm_num;i++){
        memory_queue[i]=memory_queue[i+1];
    }
}
void  test_swap()
{
    
    initQueue();

    //模拟内存物理页面队列被全部使用的情况
    //假设有20个页面 4096*20
    //只能设置为静态变量进入数据区，若声明成局部变量会使得堆栈溢出
    static char a[VMM_PAGE_SIZE*MAX_MEMORY_PAGE];
    for(int i=0;i<VMM_PAGE_SIZE*MAX_MEMORY_PAGE;i++){
        a[i]=0xff;
    }
    //printk_info("a[0]:%08X\n", &a[0]);
    for(ptr_t i=(ptr_t)VMM_LA_PA((ptr_t)&a[0]);i<(ptr_t)VMM_LA_PA((ptr_t)&
                a[VMM_PAGE_SIZE*MAX_MEMORY_PAGE]);i+=VMM_PAGE_SIZE){
        enQueue(i/PMM_PAGE_SIZE);
    }
    char b;
    b=a[0];
    printk_info("the first byte of memory page is %02X\n",b);
    //此时发生缺页中断，比如需换出2个页面。
    deQueue();
    //查看换出的2个页面是否清零
    char c;
    c=a[0];
    //printk_info("case 3\n");
    printk_info("the first byte of disk page is %02X\n",c);
    if(b!=c){
        printk_info("swap_test successful!\n");
    }
}

#ifdef __cplusplus
}
#endif
    

