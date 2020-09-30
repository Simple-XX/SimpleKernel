# 外设驱动



一般而言最主要是一些输入输出IO设备，包括显示器，键盘鼠标打印机以及磁盘等。外设将数据通过PCI总线把数据发送到CPU，CPU在得到数据后进行一个对应的操作。

设备驱动程序允许操作系统与硬件设备和系统 BIOS 直接通信。例如，设备驱动程序可能接收来自 OS 的请求来读取一些数据，而满足该请求的方法是，通过执行设备固件中的代码，用设备特有的方式来读取数据。

设备驱动程序有两种安装方法：一种是在特定硬件设备连接到系统之前，或者设备已连接并且识别之后。对于后一种方法，OS 识别设备名称和签名，然后在计算机上定位并安装设备驱动软件。

<img src=".\images\io.gif" alt="IO general" style="zoom:100%;" />

现在，通过展示应用程序在屏幕上显示字符串的过程，来了解 I/O 层次结构如上图所示。

- 应用程序调用 高级语言库函数，将字符串写入标准输出。
- 库函数（第 3 层）调用操作系统函数，传递一个字符串指针。
- 操作系统函数（第 2 层）用循环的方法调用 BIOS 子程序，向其传递每个字符的 ASCII 码和颜色。操作系统调用另一个 BIOS 子程序，将光标移动到屏幕的下一个位置上。
- BIOS 子程序（第 1 层）接收一个字符，将其映射到一个特定的系统字体，并把该字符发送到与视频控制卡相连的硬件端口。
- 视频控制卡（第 0 层）为视频显示产生定时硬件信号，来控制光栅扫描并显示像素。





<img src=".\images\IO general.png" alt="IO general" style="zoom:80%;" />

在输入输出设备的控制交互过程的中，如上图所示，是所有IO进行交互的一个通用方式，当键盘鼠标想要进行控制电脑时，敲击键盘就会发出写命令，这时负责CPU中断的设备会向CPU发出中断，cpu就把数据读到内存中进行相应处理。





首先我们先从键盘的逻辑来：

当我们使用键盘时无论是按下键，或是松开键，当键的状态改变后，键盘中的 8048 芯片把按键对应的扫描码（通码或断码）发送到主板上的 8042 芯片，由 8042 处理后保存在自己的寄存器中，然后向 8259A 发送中断信号，这样处理器便去执行键盘中断处理程序，将 8042 处理过的扫描码从它的寄存器中读取出来，继续进行下一步处理。这个键盘中断处理程序是咱们程序员负责编写的，值得注意的是我们只能得到键的扫描码，并不会得到键的 ASCII 码，扫描码是硬件提供的编码集， ASCII 是软件中约定的编码集，这两个是不同的编码方案。
我们的键盘中断处理程序是同硬件打交道的，因此只能得到硬件提供的扫描码，但我们可以将得到的“硬件”扫描码转换成对应的“软件” ASCII 码。假如我们在键盘上按下了空格键，我们在键盘中断处理程序中只能得到空格键的扫描码，该扫描码是 Ox39 ，而不是空格键的 ASCII 码 Ox20 。

按键的表现行为是字符处理程序负责的，键盘的中断处理程序便充当了字符处理程序。 一般的字符处理程序使用字符编码来处理字符，比如 ASCII 码，因此我们可以在中断处理程序中将空格的扫描码 Ox39转换成 ASCII 码 Ox20，然后将 ASCII 码 Ox20 交给我们的 put_char 函数，将 ASCII 码写入显存，也就是输出到屏幕。因此，按下空格键可以在屏幕上输出一个空格，就是这么来的。  

下面所示为SimpleKernel中的键盘实现，可以看到在keyboard_init中负责进行初始化，注册中断，当获取中断后调用keyboard_read函数来进行输出。

```c

static bool shift = false;
static bool caps = false;
static bool ctrl = false;
static bool num = true;
static bool alt = false;

// 改进方向：
// 将输入输出与 tty 结合起来
// 维护一个保存 tty 信息的结构体，包括 缓冲区信息，sh 设置，颜色等

kb_input_t kb_in;

void keyboard_handler() {
	// 从8042读取数据
	uint8_t scan_code = inb(KB_DATA);
	if(kb_in.count < KB_BUFSIZE) {
		*(kb_in.tail) = scan_code;
		++kb_in.tail;
		if(kb_in.tail == kb_in.buff + KB_BUFSIZE)
			kb_in.tail = kb_in.buff;
		++kb_in.count;
	}
	// 缓冲区满了直接丢弃
}

// 从缓冲区读取
uint8_t keyboard_read_from_buff() {
	uint8_t scancode;
	while(kb_in.count <= 0) { }  // 等待下一个字节到来
	// 进入临界区
	bool intr_flag = false;
	local_intr_store(intr_flag);
	{
		scancode = *(kb_in.head);
		kb_in.head++;
		if(kb_in.head == kb_in.buff + KB_BUFSIZE)
			kb_in.head = kb_in.buff;
		kb_in.count--;
	}
	local_intr_restore(intr_flag);
	return scancode;
}

void keyboard_read(pt_regs_t * regs __UNUSED__) {
	keyboard_handler();
	if(kb_in.count > 0) {
		uint8_t scancode = keyboard_read_from_buff();
		// 判断是否出错
		if(!scancode) {
			printk_color(red, "scancode error.\n");
			return;
		}
		uint8_t letter = 0;
		uint8_t str[2] = { '\0', '\0' }; // 在 default 中用到
		// 开始处理
		switch(scancode) {
		    // 如果是特殊字符，则单独处理
		    case KB_SHIFT_L:
			    shift = true;
			    break;
		    case KB_SHIFT_L | RELEASED_MASK: // 扫描码 + 0x80 即为松开的编码
			    shift = false;
			    break;
		    case KB_SHIFT_R:
			    shift = true;
			    break;
		    case KB_SHIFT_R | RELEASED_MASK:
			    shift = false;
			    break;
		    case KB_CTRL_L:
			    ctrl = true;
			    break;
		    case KB_CTRL_L | RELEASED_MASK:
			    ctrl = false;
			    break;
		    case KB_ALT_L:
			    alt = true;
			    break;
		    case KB_ALT_L | RELEASED_MASK:
			    alt = false;
			    break;
		    case KB_CAPS_LOCK:
			    caps = ( (!caps) & 0x01); // 与上次按下的状态相反
			    break;
		    case KB_NUM_LOCK:
			    num = ( (!num) & 0x01);
			    break;
		    case KB_BACKSPACE:
			    printk("\b");
			    break;
		    case KB_ENTER:
			    printk("\n");
			    break;
		    case KB_TAB:
			    printk("\t");
			    break;
		    default: // 一般字符输出
			    // 首先排除释放按键
			    if(!(scancode & RELEASED_MASK) ) {
				    letter = keymap[(uint8_t)(scancode * 3) + (uint8_t)shift]; // 计算在 keymap 中的位置
				    str[0] = letter;
				    str[1] = '\0';
				    printk("%s", str);
				    break;
			    } else {
				    break;
			    }
		}
	}
}

void keyboard_init(void) {
	bool intr_flag = false;
	local_intr_store(intr_flag);
	{
		kb_in.count = 0;
		kb_in.head = kb_in.tail = kb_in.buff;
        // 注册中断
		register_interrupt_handler(IRQ1, &keyboard_read);
		enable_irq(IRQ1);
		printk_info("keyboard_init\n");
	}
	local_intr_restore(intr_flag);

	return;
}
```



Reference：

http://c.biancheng.net/view/3462.html

操作系统真相还原