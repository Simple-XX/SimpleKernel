# 程序启动之实模式到保护模式

**为什么要有保护模式**

我们前面提到，CPU在启动过程中要从实模式转变到保护模式，其原因是因为实模式具有很大弊端，实模式下的用户程序可以破坏存储代码的内存区域，所以要添加个内存段类型属性来阻止这种行为。实模式下的用户程序和操作系统是同一级别的，所以要添加个特权级属性来区分用户程序和操作系统的地位。其次，实模式下的寻址空间只有16位寻址，所以为了解决这些问题，保护模式应运而生。  

我们知道，对CPU来讲，系统中的所有储存器中的储存单元都处于一个统一的逻辑储存器中，它的容量受CPU寻址能力的限制，这个逻辑储存器就是我们所说的线性地址空间。每个新的cpu发布想要增强其功能时需要考虑到向前兼容，所以对于这就是为什么CPU有这么多种模式，这就是为了保证能够提升性能的同时保留兼容性。在8086时有20位地址线，拥有1MB的线性地址空间。而80386有32位地址线，拥有4GB的线性地址空间。那么如何能够增强其寻址能力呢，这里使用了分段的机制，分段机制是通过两个寄存器基地址+段内偏移量的方式进行寻址，从而使得寻址能力加强。

但是80386依旧保留了8086采用的地址分段的方式，只是增加了一个折中的方案，即只分一个段，段基址0×00000000，段长0xFFFFFFFF（4GB）也是基地址加偏移地址的策略，这样的话整个线性空间可以看作就一个段，这就是所谓的**平坦模型**（Flat Mode）。

**实模式下的分段策略：**

在实模式对于内存段并没有访问控制，而且用户所指向的都是真正的物理地址，也就是逻辑地址和物理地址相同，实实在在的指哪打哪，能够实现对任意的程序可以修改任意地址的变量，所以就对于用户模式下就十分危险，如果用户不小心将操作系统内核的数据篡改了那么就会造成非常严重的后果。所以就需要有保护模式，使得物理地址不能被直接访问，而是要通过程序的地址（即虚拟地址）转化为物理地址后再去访问，而程序本身对此一无所知，这样就能大大提高程序的安全性。这个过程是软硬件协同实现的，CPU负责提供地址转换的部件，而操作系统提供转换需要的页表。而保护模式需要对内存段的性质和允许的操作给出定义，以实现对特定内存段的访问检测和数据保护。考虑到各种属性和需要设置的操作，32位保护模式下对一个内存段的描述需要8个字节，其称之为段描述符（Segment Descriptor）。

<img src=".\images\gdt.png" alt="gdt" style="zoom: 80%;" />

​																							 图1 段描述符的结构  

段描述符分为数据段描述符、指令段描述符和系统段描述符三种，这几个描述符可以方便的把操作系统的各种功能相对独立的放置在多个内存中，这样有利于管理程序，显然，寄存器不足以存放N多个内存段的描述符集合，所以这些描述符的集合（称之为描述符表）被放置在内存里了。在很多描述符表中，最重要的就是所谓的全局描述符表（Global Descriptor Table，GDT），它为整个软硬件系统服务。一个CPU只能有一个全局描述符表。下面介绍一下这个全局描述符表，也就是说，为了实现保护模式所以就需要这样一个GDT来作为辅助工具。



#### **全局描述符表GDT（Global Descriptor Table）**

在整个系统中，全局描述符表GDT只有一张(一个处理器对应一个GDT)，GDT可以被放在内存的任何位置，但CPU必须知道GDT的入口，也就是基地址放在哪里，Intel的设计者门提供了一个**寄存器GDTR**用来存放GDT的入口地址，程序员将GDT设定在内存中某个位置之后，可以通过**LGDT指令**将GDT的入口地址装入此寄存器，从此以后，CPU就根据此寄存器中的内容作为GDT的入口来访问GDT了。GDTR中存放的是GDT在内存中的基地址和其表长界限，也就是GDT的起始地址和GDT表的界限。

<img src=".\images\gdtr.jpg" alt="gdtr" style="zoom: 150%;" />

现在有了描述符的数组了（GDT），也有了“数组指针”（GDTR）了，怎么表示我们要访问哪个段呢？还记得8086时代的段寄存器吧？不过此时它们改名字了，叫段选择器（段选择子）。此时的CS等寄存器不再保存段基址了，而是保存其指向段的索引信息，CPU会根据这些信息在内存中获取到段信息。  

地址合成的过程如下图所示：  

<img src=".\images\logic add to linear add.png" alt="logic add to linear add" style="zoom: 80%;" />

./src/arch/x86_64/gdt/include/gdt.h 中有相关数据结构的定义，分别为gdtr和gdt：

```c
typedef
    struct gdt_ptr_t {
	uint16_t	limit; // 全局描述符表限长
	uint32_t	base; // 全局描述符表 32位 基地址
} __attribute__( (packed) ) gdt_ptr_t;


typedef
    struct gdt_entry_t {
	uint16_t	limit_low; // 段界限   15～0
	uint16_t	base_low; // 段基地址 15～0
	uint8_t		base_middle; // 段基地址 23～16
	uint8_t		access;   // 段存在位、描述符特权级、描述符类型、描述符子类别
	uint8_t		granularity; // 其他标志、段界限 19～16 (unsigned limit_high: 4;unsigned flags: 4;)
	uint8_t		base_high; // 段基地址 31～24
} __attribute__( (packed) ) gdt_entry_t;
```

./src/arch/x86_64/gdt/gdt.c 实现了全局描述符表的定义，主要是设置了寄存器的参数以及gdt表的段参数。

```c
static gdt_ptr_t gdt_ptr;
// 全局描述符表定义
static gdt_entry_t gdt_entries[GDT_LENGTH] __attribute__( (aligned(8) ) );

void gdt_set_gate(int32_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
	gdt_entries[num].base_low     = (base & 0xFFFF);
	gdt_entries[num].base_middle  = (base >> 16) & 0xFF;
	gdt_entries[num].base_high    = (base >> 24) & 0xFF;

	gdt_entries[num].limit_low    = (limit & 0xFFFF);
	gdt_entries[num].granularity  = (limit >> 16) & 0x0F;

	gdt_entries[num].granularity |= gran & 0xF0;
	gdt_entries[num].access       = access;
	return;
}

// 初始化全局描述符表
void gdt_init(void) {
	bool intr_flag = false;
	local_intr_store(intr_flag);
	{
		// 全局描述符表界限  从 0 开始，所以总长要 - 1
		gdt_ptr.limit = sizeof(gdt_entry_t) * GDT_LENGTH - 1;
		gdt_ptr.base = (uint32_t)&gdt_entries;

		// 采用 Intel 平坦模型
		// 0xC0: 粒度为 4096?
		gdt_set_gate(SEG_NULL, 0x0, 0x0, 0x0, 0x0);      // Intel 文档要求首个描述符全 0
		gdt_set_gate(SEG_KTEXT, 0x0, 0xFFFFFFFF, KREAD_EXEC, 0xC0); // 内核指令段
		gdt_set_gate(SEG_KDATA, 0x0, 0xFFFFFFFF, KREAD_WRITE, 0xC0); // 内核数据段
		gdt_set_gate(SEG_UTEXT, 0x0, 0xFFFFFFFF, UREAD_EXEC, 0xC0); // 用户模式代码段
		gdt_set_gate(SEG_UDATA, 0x0, 0xFFFFFFFF, UREAD_WRITE, 0xC0); // 用户模式数据段
		tss_set_gate(SEG_TSS, KERNEL_DS, 0);

		// 加载全局描述符表地址到 GDTR 寄存器
		gdt_load( (uint32_t)&gdt_ptr);
		// 加载任务寄存器
		tss_load();

		printk_info("gdt_init\n");
		local_intr_restore(intr_flag);
	}
	return;
}
```



```asm
.section .text
.global gdt_load
gdt_load:
    cli
    mov 4(%esp), %eax   #参数存入 eax 寄存器
    lgdt (%eax)      # 加载到 GDTR [修改原先GRUB设置]

    mov $0x10, %ax  # 加载数据段描述符
    mov %ax, %ds  # 更新所有可以更新的段寄存器
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss
    jmp $0x08, $flush # 远跳转，0x08是代码段描述符
    # 远跳目的是清空流水线并串行化处理器

flush:
    ret
```

我想这个汇编函数中唯一需要解释的就是jmp跳转那一句了，首先0×08是我们跳转目标段的段选择子(这个不陌生吧？)，其对应段描述符第2项。后面的跳转目标标号可能会让你诧异，因为它就是下一行代码。这是为何？当然有深意了，第一，Intel不允许直接修改段寄存器cs的值，我们只好这样通过这种方式更新cs段寄存器；第二，x86以后CPU所增加的指令流水线和高速缓存可能会在新的全局描述符表加载之后依旧保持之前的缓存，那么修改GDTR之后最安全的做法就是立即清空流水线和更新高速缓存。说的这么牛逼的样子，其实只要一句jmp跳转就能强迫CPU自动更新了。

通过将GDT告诉给CPU后，CPU就知道了操作系统中段的设置，从而可以通过段选择子得到线性地址，这里的段选择子就是用来在段描述符表中进行选择的，可以把段描述符表理解为一个之前在实模式下的，在后面实现分页管理后，可进一步将线性地址转换为物理地址（不过当前连物理址有多大都没法知道，在后面会解决）。

除了GDT之外，IA-32还允许程序员构建与GDT类似的数据结构，它们被称作LDT（Local Descriptor Table，局部描述符表），它主要用来进行进程管理，但与GDT不同的是，LDT在系统中可以存在多个，并且从LDT的名字可以得知，LDT不是全局可见的，它们只对引用它们的任务可见，每个任务最多可以拥有一个LDT。另外，每一个LDT自身作为一个段存在，它们的段描述符被放在GDT中，intel厂商本想着每一个任务都有一个LDT用于管理段的地址，不过现在很少这么使用。

之后我们就进入到内核之中进行一系列初始化操作：

```
// 内核入口函数
void kernel_entry(ptr_t magic, ptr_t addr) {
	mm_init();
	cpu_switch_stack(KERNEL_STACK_BOTTOM);
	kernel_main(magic, KERNEL_BASE + addr);
	return;
}
```

其中内核主要初始化过程如下：

```
// 内核入口
void kernel_main(ptr_t magic, ptr_t addr) {
	bool intr_flag = false;
	local_intr_store(intr_flag);
	{
		// 控制台初始化
		console_init();
		// 从 multiboot 获得系统初始信息
		multiboot2_init(magic, addr);
		// GDT、IDT 初始化
		arch_init();
		// 时钟初始化
		clock_init();
		// 键盘初始化
		keyboard_init();
		// 调试模块初始化
		debug_init(magic, addr);
		// 物理内存初始化
		pmm_init();
		// 虚拟内存初始化
		vmm_init();
		// 堆初始化
		heap_init();
		// 任务初始化
		task_init();
		// 调度初始化
		// sched_init();

		// showinfo();
		test();
	}
	local_intr_restore(intr_flag);

	for(int i = 0 ; i < 100 ; i++) {
		printk("8");
	}

	// 永远不会执行到这里
	assert(0, "Never to be seen.\n");
	return;
}
```


