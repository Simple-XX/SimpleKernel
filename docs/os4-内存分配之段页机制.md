# 内存分配之段页机制

前面我们提到的是在操作系统在与CPU进行交互时所需要处理的一些比较细节的问题，这里我们重新从编写代码的角度来看一下对于一个进程而言的内存是如何管理的。

如下图所示，这是一个进程的逻辑地址，高地址为操作系统内核的固定暂存区，为1GB大小，低地址为用户所能访问的地址区域，为3GB。这样进行分配的好处是可以让程序员能够更好的去管理不同部分的地址，根据程序中功能区来区分出来数据所放置的位置，也就是说分段机制是为了用户所设置的。

<img src=".\images\memory.png" alt="memory" style="zoom:67%;" />

对于程序员来说，一个段表主要包括有，Text 段、Data段、BSS段、堆、内存映射区、栈段以及内核空间，其中Data段主要负责存储静态变量、BSS段主要存储未初始化的变量、堆段负责存储在程序中动态分配的变量，从低地址向高地址生长，内存映射区主要负责一些动态链接库以及大块内存的加载；栈段是分配是从高地址向低地址生长。

但是，长时间以来，随着计算机技术的发展，存储器的容量在不断的高速增加着。但是说起内存(这里指RAM，下同)这个东西，它有一个很奇葩的特性，就是无论它有多大，都总是不够用(P.S.厨房的垃圾桶也一样)。现在我们看似拥有着以前的程序员想都不敢想的"天文数字"的内存，动辄就是几G十几G的。但是相信我，历史总是嘲弄人的。就像当年程序员们质疑32位地址线带来的4GB空间太大没有意义似的，我们也会有一天抱怨现在的内存太小的。

那么，既然内存总是不够用的，那内存不够用了怎么办？还有，使用过程中出现的内存碎片怎么办？假设我们有4GB的物理内存，现在有1、2、3、4一共4个程序分别各占据连续的1G内存，然后2、4退出，此时虽然有空闲的两段内存，却连一个稍大于1GB的程序都无法载入了。我们就需要其他的机制来扩展内存，这里就是通过虚拟内存的方式进行管理，而虚拟内存就是通过页表机制实现的，我们看到了固定大小的物理页、虚拟页、甚至还有磁盘页，也就是可以通过将虚拟页的内存换出到磁盘，然后再将需要的内存空间从磁盘中换入，这样就实现了让内存大小可以继续扩展，也就是虚拟内存的机制。 

​															 <img src=".\images\page.png" alt="page" style="zoom:80%;" />

​																										图1 页表机制 

那么接下来的问题是，怎么表示和存储这个映射关系。这里描述起来简单，但是代码就不是那么直观了，原因很简单，因为需要一组数据结构来管理内存，但是这组数据结构本身也得放在内存里。所以牵扯到一个自己管理自己的问题。而且，开启分页模式之后，CPU立即就会按照分页模式的规则去解释线性地址了。所以，这意味着必须先建立好地址映射的数据结构，才能开启分页，而且必须保证之前的代码地址和数据地址都能映射正确。  

在32位操作系统下使用32位地址总线(暂时原谅我在这里错误的描述吧，其实还有PAE这个东西)，所以寻址空间有2的32次方，也就是4GB。一定要注意，我们强调了很多次了，这个空间里，有一些断断续续的地址实际上是指向了其它的外设，不过大部分还是指向RAM的。具体采取的分页大小可以有多种选择，但是过于小的分页会造成管理结构太大，过于大的分页又浪费内存。现在较为常见的分页是4KB一个页，也就是4096字节一个页。简单计算下，4GB的内存分成4KB一个的页，那就是1MB个页，没错吧？每个虚拟页到物理页的映射需要4个字节来存储的话(别忘了前提是32位环境下)，整个4GB空间的映射需要4MB的数据结构来存储。

目前看起来一切都很好，4MB似乎也不是很大。但是，这只是一个虚拟地址空间的映射，别忘了每个进程都有自己的映射，而且操作系统中通常有N个进程在运行。这样的话，假如有100个进程在运行，就需要400MB的内存来存储管理信息！这就太浪费了。
怎么办？聪明的工程师们提出了分级页表的实现策略，他们提出了页目录，页表的概念。以32位的地址来说，分为3段来寻址，分别是地址的低12位，中间10位和高10位。高10位表示当前地址项在页目录中的偏移，最终偏移处指向对应的页表，中间10位是当前地址在该页表中的偏移，我们按照这个偏移就能查出来最终指向的物理页了，最低的12位表示当前地址在该物理页中的偏移。就这样，我们就实现了分级页表。

<img src=".\images\multipage.png" alt="multipage" style="zoom:80%;" />

​																							图 2 地址转换

现在我们就来看下是如何实现的，我们把内存管理分为物理内存管理和虚拟内存管理两个部分来进行。首先我们讨论物理内存的管理。要实现内存的管理，首先要解决以下三个问题：  

在代码src\arch\x86_64\mm中有关于页表的数据结构描述以及物理页表初始化以及分配释放物理页：

```c
// 8KB
#define KERNEL_STACK_SIZE    (0x2000UL)
#define KERNEL_STACK_PAGES   (KERNEL_STACK_SIZE / PMM_PAGE_SIZE)
#define KERNEL_STACK_BOTTOM  (0xC0000000UL)
#define KERNEL_STACK_TOP     (KERNEL_STACK_BOTTOM - KERNEL_STACK_SIZE)
// 512 MB
#define PMM_MAX_SIZE  (0x20000000UL)

// 内核的偏移地址
#define KERNEL_BASE    (0xC0000000UL)
// 内核占用大小 8MB
#define KERNEL_SIZE    (0x800000UL)
// 映射内核需要的页数
#define PMM_PAGES_KERNEL    (KERNEL_SIZE / PMM_PAGE_SIZE)

// 页掩码，用于 4KB 对齐
#define PMM_PAGE_MASK    (0xFFFFF000UL)


// 内存管理结构体
typedef
    struct pmm_manage {
	// 管理算法的名称
	const char *      name;
	// 初始化
	void (* pmm_manage_init)(ptr_t page_start, size_t page_count);
	// 申请物理内存，单位为 Byte，以页为单位对齐
	ptr_t (* pmm_manage_alloc)(size_t bytes);
	// 释放内存页
	void (* pmm_manage_free)(ptr_t addr_start, size_t bytes);
	// 返回当前可用内存页数量
	size_t (* pmm_manage_free_pages_count)(void);
} pmm_manage_t;

// 物理内存初始化
void pmm_phy_init(e820map_t * e820map);
// 物理内存管理初始化
void pmm_mamage_init(e820map_t * e820map);
// 初始化内存管理
void pmm_init(void);
// 分配内存，单位为 byte，返回的是物理地址，以页为单位对齐
ptr_t pmm_alloc(size_t byte);
// 回收内存，单位为 byte
void pmm_free(ptr_t addr, size_t byte);
// 返回可用物理内存页数
size_t pmm_free_pages_count(void);
```

这里我们看到有一个叫做`e820map_t` 的参数，这个参数是在BIOS里面的一个中断，具体说是int 0×15，之所以叫e820是因为在用这个中断时ax必须是0xe820。这个中断的作用是得到系统的内存布局。因为系统内存会有很多段，每段的类型属性也不一样，所以这个查询是"迭代式"的，每次求得一个段。

下面来介绍下虚拟内存管理的实现：

虚拟的页面每页占据4KB，按页为单位进行管理。物理内存也被分页管理，按照4KB分为一个个物理页框。虚拟地址到物理地址通过由页目录和页表组成的二级页表映射，页目录的地址放置在CR3寄存器里。  



因为我们使用了Intel平坦模式的内存模型，所以之前的分段机制是被"绕过去"的，所以分页的管理就成了内存管理的核心了。首先是内核自身地址的映射，Linux采用的方案是把内核映射到线性地址空间3G以上，而应用程序占据线性地址空间0-3G的位置。我们的内核采取和Linux内核一样的映射，把物理地址0从虚拟地址0xC0000000（3G）处开始往上映射，因为我们只管理最多512MB的内存，所以3G-4G之间能完全的映射全部的物理地址。采取这个映射后，物理地址和内核虚拟地址满足以下关系：  物理地址 + 0xC0000000 = 内核虚拟地址  

我们先引入VMA（Virtual Memory Address）和LMA（Load Memory Address）这两个概念。其中VMA是链接器生成可执行文件时的偏移计算地址，而LMA是区段所载入内存的实际地址。通常情况下的VMA是等于LMA的。

在代码src\arch\x86_64\mm中有关于页表的数据结构描述以及虚拟页表初始化以及对页表进行物理页映射设置：

```c
void vmm_init(void) {
	bool intr_flag = false;
	local_intr_store(intr_flag);
	{
		register_interrupt_handler(INT_PAGE_FAULT, &page_fault);

		// 映射全部内核
		uint32_t pgd_idx = VMM_PGD_INDEX(KERNEL_BASE);
		for(uint32_t i = pgd_idx, j = 0 ; i < VMM_PAGE_DIRECTORIES_KERNEL + pgd_idx ; i++, j++) {
			pgd_kernel[i] = ( (ptr_t)VMM_LA_PA( (ptr_t)pte_kernel[j]) | VMM_PAGE_PRESENT | VMM_PAGE_RW | VMM_PAGE_KERNEL);
		}
		ptr_t * pte = (ptr_t *)pte_kernel;
		for(uint32_t i = 0 ; i < VMM_PAGES_PRE_PAGE_TABLE * VMM_PAGE_TABLES_KERNEL ; i++) {
			pte[i] = (i << 12) | VMM_PAGE_PRESENT | VMM_PAGE_RW | VMM_PAGE_KERNEL;
		}
		// 映射内核栈
		// 0x2FF
		pgd_idx = VMM_PGD_INDEX(KERNEL_STACK_TOP);
		pgd_kernel[pgd_idx] = ( (ptr_t)VMM_LA_PA( (ptr_t)pte_kernel_stack) | VMM_PAGE_PRESENT | VMM_PAGE_RW | VMM_PAGE_KERNEL);
		// i: 0x3F8~0x400
		for(uint32_t i = VMM_PAGES_PRE_PAGE_TABLE - KERNEL_STACK_PAGES, j = VMM_PAGES_PRE_PAGE_TABLE * 2 ; i < VMM_PAGES_PRE_PAGE_TABLE ; i++, j++) {
			pte_kernel_stack[i] = (j << 12) | VMM_PAGE_PRESENT | VMM_PAGE_RW | VMM_PAGE_KERNEL;
		}
		switch_pgd(VMM_LA_PA( (ptr_t)pgd_kernel) );
		printk_info("vmm_init\n");
	}
	local_intr_restore(intr_flag);
	return;
}

// 以页为单位
void map(pgd_t * pgd_now, ptr_t va, ptr_t pa, uint32_t flags) {
	uint32_t pgd_idx = VMM_PGD_INDEX(va);
	uint32_t pte_idx = VMM_PTE_INDEX(va);
	pte_t * pte = (pte_t *)(pgd_now[pgd_idx] & VMM_PAGE_MASK);
	// 转换到内核线性地址
	if(pte == NULL) {
		pgd_now[pgd_idx] = (uint32_t)pte | flags;
		pte = (pte_t *)VMM_PA_LA( (ptr_t)pte);
	} else {
		pte = (pte_t *)VMM_PA_LA( (ptr_t)pte);
	}

	pte[pte_idx] = (pa & VMM_PAGE_MASK) | flags;
	// 通知 CPU 更新页表缓存
	CPU_INVLPG(va);
	return;
}

void unmap(pgd_t * pgd_now, ptr_t va) {
	uint32_t pgd_idx = VMM_PGD_INDEX(va);
	uint32_t pte_idx = VMM_PTE_INDEX(va);
	pte_t * pte = (pte_t *)(pgd_now[pgd_idx] & VMM_PAGE_MASK);
	// 转换到内核线性地址
	pte = (pte_t *)VMM_PA_LA( (ptr_t)pte);
	pte[pte_idx] = 0;
	// 通知 CPU 更新页表缓存
	CPU_INVLPG(va);
	return;
}


```





**段页机制结合：**

操作系统中的内存是通过段页结合的方式进行管理，那么我们是如何将段页结合起来的呢？我们在程序中所使用的地址其实并不是真实地址，而是逻辑地址，也就是对于每个程序看起来都是独立的地址空间。 在程序中我们所得到的逻辑地址通过段机制得到虚拟内存，再通过页表映射访问得到物理地址，这样我们就能得到通过虚拟内存到物理内存的映射。

所以对于用户而言，只能看到虚拟内存，也就是一段寻址空间，而从物理内存的角度中是通过虚拟内存的页表映射到物理内存的，这也就是操作系统所实现的。 

<img src=".\images\virtual memory5.png" alt="virtual memory5" style="zoom:80%;" />

​									图3 通过段机制寻址：用户生成逻辑地址->虚拟内存地址->页表访问->物理地址

但是如果我们把这一系列的操作通过软件编程的方式实现，效率是非常低的，所以在CPU中有一个MMU（Memory Management Unit）硬件来进行这一系列查询操作，那么MMU是怎么知道如何替换的呢，MMU会去读取我们前面提到的在通过设置GDTR和GDT来进行配置。MMU映射单位并不是字节，而是页，这个映射通过查一个常驻内存的数据结构[页表](http://en.wikipedia.org/wiki/Page_table)来实现。现在计算机具体的内存地址映射比较复杂，为了加快速度会引入一系列缓存和优化，例如[TLB](http://en.wikipedia.org/wiki/Translation_lookaside_buffer)等机制。下面给出一个经过简化的内存地址翻译示意图，虽然经过了简化，但是基本原理与现代计算机真实的情况的一致的。

<img src=".\images\mmu.png" alt="mmu" style="zoom:80%;" />

该图是从来自于Intel官方手册的寻址方式，可以看出是先通过段机制得到地址，然后再通过页表方式寻址算出真实物理地址。

<img src=".\images\x8632_addressmode.png" alt="x8632_addressmode" style="zoom:80%;" />

​																						图4 段页结合寻址

**Reference：**

http://wiki.0xffffff.org

Intel® 64 and IA-32 Architectures Software Developer’s Manual Vol-ume3 : System Programming Guide  

https://blog.csdn.net/wang13342322203/article/details/80862382