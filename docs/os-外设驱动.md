

一般而言最主要的IO设备包括显示器，键盘鼠标以及磁盘等。

<img src=".\images\IO general.png" alt="IO general" style="zoom:80%;" />

在输入输出设备的控制交互过程的中，如上图所示，是所有IO进行交互的一个通用方式，当键盘鼠标想要进行控制电脑时，敲击键盘就会发出写命令，这时负责CPU中断的设备会向CPU发出中断，cpu就把数据读到内存中进行相应处理。

首先我们先从键盘的逻辑来：

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
		register_interrupt_handler(IRQ1, &keyboard_read);
		enable_irq(IRQ1);
		printk_info("keyboard_init\n");
	}
	local_intr_restore(intr_flag);

	return;
}
```

