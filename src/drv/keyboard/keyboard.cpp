
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
// Based on Orange's 一个操作系统的实现
// keyboard.cpp for Simple-XX/SimpleKernel.

#include "stddef.h"
#include "stdbool.h"
#include "keyboard.h"

// 改进方向：
// 将输入输出与 tty 结合起来
// 维护一个保存 tty 信息的结构体，包括 缓冲区信息，sh 设置，颜色等

KEYBOARD::kb_input_t KEYBOARD::kb_in;
bool                 KEYBOARD::shift;
bool                 KEYBOARD::caps;
bool                 KEYBOARD::ctrl;
bool                 KEYBOARD::num;
bool                 KEYBOARD::alt;

KEYBOARD::KEYBOARD(void) {
    shift       = false;
    caps        = false;
    ctrl        = false;
    num         = true;
    alt         = false;
    kb_in.count = 0;
    kb_in.head = kb_in.tail = kb_in.buff;
    return;
}

KEYBOARD::~KEYBOARD(void) {
    return;
}

void KEYBOARD::keyboard_handler() {
    // 从8042读取数据
    uint8_t scan_code = io.inb(KB_DATA);
    if (kb_in.count < KB_BUFSIZE) {
        *(kb_in.tail) = scan_code;
        ++kb_in.tail;
        if (kb_in.tail == kb_in.buff + KB_BUFSIZE)
            kb_in.tail = kb_in.buff;
        ++kb_in.count;
    }
    // 缓冲区满了直接丢弃
    return;
}

// 从缓冲区读取
uint8_t KEYBOARD::keyboard_read_from_buff() {
    uint8_t scancode;
    // 等待下一个字节到来
    while (kb_in.count <= 0) {
        ;
    }
    scancode = *(kb_in.head);
    kb_in.head++;
    if (kb_in.head == kb_in.buff + KB_BUFSIZE)
        kb_in.head = kb_in.buff;
    kb_in.count--;
    return scancode;
}

void KEYBOARD::keyboard_read(INTR::pt_regs_t *regs __attribute__((unused))) {
    keyboard_handler();
    if (kb_in.count > 0) {
        uint8_t scancode = keyboard_read_from_buff();
        // 判断是否出错
        if (!scancode) {
            io.printf("scancode error.\n");
            return;
        }
        uint8_t letter = 0;
        // 开始处理
        switch (scancode) {
            // 如果是特殊字符，则单独处理
            case KB_SHIFT_L:
                shift = true;
                break;
            // 扫描码 + 0x80 即为松开的编码
            case KB_SHIFT_L | RELEASED_MASK:
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
                // 与上次按下的状态相反
                caps = ((!caps) & 0x01);
                break;
            case KB_NUM_LOCK:
                num = ((!num) & 0x01);
                break;
            case KB_BACKSPACE:
                io.put_char('\b');
                break;
            case KB_ENTER:
                io.put_char('\n');
                break;
            case KB_TAB:
                io.put_char('\t');
                break;
            // 一般字符输出
            default:
                // 首先排除释放按键
                if (!(scancode & RELEASED_MASK)) {
                    // 计算在 keymap 中的位置
                    letter = keymap[(uint8_t)(scancode * 3) + (uint8_t)shift];
                    io.put_char(letter);
                    break;
                }
                else {
                    break;
                }
        }
    }
}

static void kb_handle(INTR::pt_regs_t *regs) {
    keyboardk.keyboard_read(regs);
    return;
}

int32_t KEYBOARD::init(void) {
    INTR::register_interrupt_handler(INTR::get_irq(1), &kb_handle);
    INTR::enable_irq(INTR::get_irq(1));
    io.printf("keyboard_init\n");
    return 0;
}

KEYBOARD keyboardk;
