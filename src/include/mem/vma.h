#ifndef _VMA_H_
#define _VMA_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "vmm.h"
#define VM_READ 0x00000001
#define VM_WRITE 0x00000002
#define VM_EXEC 0x00000004
struct mm_struct{
    struct vm_area_struct *mmap_cache; //指向当前正在使用的虚拟内存空间，由于操作系统执行的“局部性”原理，当前正在用到的虚拟内存空间在接下来的操作中可能还会用到，这时就不需要查链表，而是直接使用此指针就可找到下一次要用到的虚拟内存空间。由于mmap_cache 的引入，可使得 mm_struct 数据结构的查询加速 30% 以上。
    pgd_t *pgdir; //mm_struct数据结构所维护的页表,通过访问pgdir可以查找某虚拟地址对应的页表项是否存在以及页表项的属性等
    int count; //list_map 里面链接的 vma_struct的个数
    void *sm_priv; //链接记录页访问情况的链表头
    struct mm_struct *prev; //双向链表，链接了所有属于同一页目录表的虚拟内存空间
    struct mm_struct *next; 
};
//虚拟内存区域，指向虚拟存储的一段内存
struct vm_area_struct{
    ptr_t start_addr; //起始地址 
    ptr_t end_addr;  //结束地址
    int flag; //内存段属性
    struct vm_area_struct *prev; //前一节点
    struct vm_area_struct *next; //后一节点
};


#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_VMA_H */