
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel). Based on Orange's
// 一个操作系统的实现 kb.c for Simple-XX/SimpleKernel.

#ifdef __cplusplus
extern "C" {
#endif

#include "stddef.h"
#include "stdio.h"
#include "stdbool.h"
#include "intr.h"
#include "port.hpp"
#include "keyboard.h"

static uint8_t keymap[NR_SCAN_CODES * MAP_COLS] = {
    /* scan-code			!Shift		Shift		E0 XX
     */
    /* ==================================================================== */
    /* 0x00 - none	*/ 0,
    0,
    0,
    /* 0x01 - ESC		*/ ESC,
    ESC,
    0,
    /* 0x02 - '1'		*/ '1',
    '!',
    0,
    /* 0x03 - '2'		*/ '2',
    '@',
    0,
    /* 0x04 - '3'		*/ '3',
    '#',
    0,
    /* 0x05 - '4'		*/ '4',
    '$',
    0,
    /* 0x06 - '5'		*/ '5',
    '%',
    0,
    /* 0x07 - '6'		*/ '6',
    '^',
    0,
    /* 0x08 - '7'		*/ '7',
    '&',
    0,
    /* 0x09 - '8'		*/ '8',
    '*',
    0,
    /* 0x0A - '9'		*/ '9',
    '(',
    0,
    /* 0x0B - '0'		*/ '0',
    ')',
    0,
    /* 0x0C - '-'		*/ '-',
    '_',
    0,
    /* 0x0D - '='		*/ '=',
    '+',
    0,
    /* 0x0E - BS		*/ BACKSPACE,
    BACKSPACE,
    0,
    /* 0x0F - TAB		*/ TAB,
    TAB,
    0,
    /* 0x10 - 'q'		*/ 'q',
    'Q',
    0,
    /* 0x11 - 'w'		*/ 'w',
    'W',
    0,
    /* 0x12 - 'e'		*/ 'e',
    'E',
    0,
    /* 0x13 - 'r'		*/ 'r',
    'R',
    0,
    /* 0x14 - 't'		*/ 't',
    'T',
    0,
    /* 0x15 - 'y'		*/ 'y',
    'Y',
    0,
    /* 0x16 - 'u'		*/ 'u',
    'U',
    0,
    /* 0x17 - 'i'		*/ 'i',
    'I',
    0,
    /* 0x18 - 'o'		*/ 'o',
    'O',
    0,
    /* 0x19 - 'p'		*/ 'p',
    'P',
    0,
    /* 0x1A - '['		*/ '[',
    '{',
    0,
    /* 0x1B - ']'		*/ ']',
    '}',
    0,
    /* 0x1C - CR/LF		*/ ENTER,
    ENTER,
    PAD_ENTER,
    /* 0x1D - l. Ctrl	*/ CTRL_L,
    CTRL_L,
    CTRL_R,
    /* 0x1E - 'a'		*/ 'a',
    'A',
    0,
    /* 0x1F - 's'		*/ 's',
    'S',
    0,
    /* 0x20 - 'd'		*/ 'd',
    'D',
    0,
    /* 0x21 - 'f'		*/ 'f',
    'F',
    0,
    /* 0x22 - 'g'		*/ 'g',
    'G',
    0,
    /* 0x23 - 'h'		*/ 'h',
    'H',
    0,
    /* 0x24 - 'j'		*/ 'j',
    'J',
    0,
    /* 0x25 - 'k'		*/ 'k',
    'K',
    0,
    /* 0x26 - 'l'		*/ 'l',
    'L',
    0,
    /* 0x27 - ';'		*/ ';',
    ':',
    0,
    /* 0x28 - '\''		*/ '\'',
    '"',
    0,
    /* 0x29 - '`'		*/ '`',
    '~',
    0,
    /* 0x2A - l. SHIFT	*/ SHIFT_L,
    SHIFT_L,
    0,
    /* 0x2B - '\'		*/ '\\',
    '|',
    0,
    /* 0x2C - 'z'		*/ 'z',
    'Z',
    0,
    /* 0x2D - 'x'		*/ 'x',
    'X',
    0,
    /* 0x2E - 'c'		*/ 'c',
    'C',
    0,
    /* 0x2F - 'v'		*/ 'v',
    'V',
    0,
    /* 0x30 - 'b'		*/ 'b',
    'B',
    0,
    /* 0x31 - 'n'		*/ 'n',
    'N',
    0,
    /* 0x32 - 'm'		*/ 'm',
    'M',
    0,
    /* 0x33 - ','		*/ ',',
    '<',
    0,
    /* 0x34 - '.'		*/ '.',
    '>',
    0,
    /* 0x35 - '/'		*/ '/',
    '?',
    PAD_SLASH,
    /* 0x36 - r. SHIFT	*/ SHIFT_R,
    SHIFT_R,
    0,
    /* 0x37 - '*'		*/ '*',
    '*',
    0,
    /* 0x38 - ALT		*/ ALT_L,
    ALT_L,
    ALT_R,
    /* 0x39 - ' '		*/ ' ',
    ' ',
    0,
    /* 0x3A - CapsLock	*/ CAPS_LOCK,
    CAPS_LOCK,
    0,
    /* 0x3B - F1		*/ F1,
    F1,
    0,
    /* 0x3C - F2		*/ F2,
    F2,
    0,
    /* 0x3D - F3		*/ F3,
    F3,
    0,
    /* 0x3E - F4		*/ F4,
    F4,
    0,
    /* 0x3F - F5		*/ F5,
    F5,
    0,
    /* 0x40 - F6		*/ F6,
    F6,
    0,
    /* 0x41 - F7		*/ F7,
    F7,
    0,
    /* 0x42 - F8		*/ F8,
    F8,
    0,
    /* 0x43 - F9		*/ F9,
    F9,
    0,
    /* 0x44 - F10		*/ F10,
    F10,
    0,
    /* 0x45 - NumLock	*/ NUM_LOCK,
    NUM_LOCK,
    0,
    /* 0x46 - ScrLock	*/ SCROLL_LOCK,
    SCROLL_LOCK,
    0,
    /* 0x47 - Home		*/ PAD_HOME,
    '7',
    HOME,
    /* 0x48 - CurUp		*/ PAD_UP,
    '8',
    UP,
    /* 0x49 - PgUp		*/ PAD_PAGEUP,
    '9',
    PAGEUP,
    /* 0x4A - '-'		*/ PAD_MINUS,
    '-',
    0,
    /* 0x4B - Left		*/ PAD_LEFT,
    '4',
    LEFT,
    /* 0x4C - MID		*/ PAD_MID,
    '5',
    0,
    /* 0x4D - Right		*/ PAD_RIGHT,
    '6',
    RIGHT,
    /* 0x4E - '+'		*/ PAD_PLUS,
    '+',
    0,
    /* 0x4F - End		*/ PAD_END,
    '1',
    END,
    /* 0x50 - Down		*/ PAD_DOWN,
    '2',
    DOWN,
    /* 0x51 - PgDown	*/ PAD_PAGEDOWN,
    '3',
    PAGEDOWN,
    /* 0x52 - Insert	*/ PAD_INS,
    '0',
    INSERT,
    /* 0x53 - Delete	*/ PAD_DOT,
    '.',
    DELETE,
    /* 0x54 - Enter		*/ 0,
    0,
    0,
    /* 0x55 - ???		*/ 0,
    0,
    0,
    /* 0x56 - ???		*/ 0,
    0,
    0,
    /* 0x57 - F11		*/ F11,
    F11,
    0,
    /* 0x58 - F12		*/ F12,
    F12,
    0,
    /* 0x59 - ???		*/ 0,
    0,
    0,
    /* 0x5A - ???		*/ 0,
    0,
    0,
    /* 0x5B - ???		*/ 0,
    0,
    GUI_L,
    /* 0x5C - ???		*/ 0,
    0,
    GUI_R,
    /* 0x5D - ???		*/ 0,
    0,
    APPS,
    /* 0x5E - ???		*/ 0,
    0,
    0,
    /* 0x5F - ???		*/ 0,
    0,
    0,
    /* 0x60 - ???		*/ 0,
    0,
    0,
    /* 0x61 - ???		*/ 0,
    0,
    0,
    /* 0x62 - ???		*/ 0,
    0,
    0,
    /* 0x63 - ???		*/ 0,
    0,
    0,
    /* 0x64 - ???		*/ 0,
    0,
    0,
    /* 0x65 - ???		*/ 0,
    0,
    0,
    /* 0x66 - ???		*/ 0,
    0,
    0,
    /* 0x67 - ???		*/ 0,
    0,
    0,
    /* 0x68 - ???		*/ 0,
    0,
    0,
    /* 0x69 - ???		*/ 0,
    0,
    0,
    /* 0x6A - ???		*/ 0,
    0,
    0,
    /* 0x6B - ???		*/ 0,
    0,
    0,
    /* 0x6C - ???		*/ 0,
    0,
    0,
    /* 0x6D - ???		*/ 0,
    0,
    0,
    /* 0x6E - ???		*/ 0,
    0,
    0,
    /* 0x6F - ???		*/ 0,
    0,
    0,
    /* 0x70 - ???		*/ 0,
    0,
    0,
    /* 0x71 - ???		*/ 0,
    0,
    0,
    /* 0x72 - ???		*/ 0,
    0,
    0,
    /* 0x73 - ???		*/ 0,
    0,
    0,
    /* 0x74 - ???		*/ 0,
    0,
    0,
    /* 0x75 - ???		*/ 0,
    0,
    0,
    /* 0x76 - ???		*/ 0,
    0,
    0,
    /* 0x77 - ???		*/ 0,
    0,
    0,
    /* 0x78 - ???		*/ 0,
    0,
    0,
    /* 0x78 - ???		*/ 0,
    0,
    0,
    /* 0x7A - ???		*/ 0,
    0,
    0,
    /* 0x7B - ???		*/ 0,
    0,
    0,
    /* 0x7C - ???		*/ 0,
    0,
    0,
    /* 0x7D - ???		*/ 0,
    0,
    0,
    /* 0x7E - ???		*/ 0,
    0,
    0,
    /* 0x7F - ???		*/ 0,
    0,
    0};

static bool shift = false;
static bool caps  = false;
static bool ctrl  = false;
static bool num   = true;
static bool alt   = false;

// 改进方向：
// 将输入输出与 tty 结合起来
// 维护一个保存 tty 信息的结构体，包括 缓冲区信息，sh 设置，颜色等

kb_input_t kb_in;

void keyboard_handler() {
    // 从8042读取数据
    uint8_t scan_code = inb(KB_DATA);
    if (kb_in.count < KB_BUFSIZE) {
        *(kb_in.tail) = scan_code;
        ++kb_in.tail;
        if (kb_in.tail == kb_in.buff + KB_BUFSIZE)
            kb_in.tail = kb_in.buff;
        ++kb_in.count;
    }
    // 缓冲区满了直接丢弃
}

// 从缓冲区读取
uint8_t keyboard_read_from_buff() {
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

void keyboard_read(pt_regs_t *regs __UNUSED__) {
    keyboard_handler();
    if (kb_in.count > 0) {
        uint8_t scancode = keyboard_read_from_buff();
        // 判断是否出错
        if (!scancode) {
            printk_color(red, "scancode error.\n");
            return;
        }
        uint8_t letter = 0;
        // 在 default 中用到
        uint8_t str[2] = {'\0', '\0'};
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
            printk("\b");
            break;
        case KB_ENTER:
            printk("\n");
            break;
        case KB_TAB:
            printk("\t");
            break;
        // 一般字符输出
        default:
            // 首先排除释放按键
            if (!(scancode & RELEASED_MASK)) {
                // 计算在 keymap 中的位置
                letter = keymap[(uint8_t)(scancode * 3) + (uint8_t)shift];
                str[0] = letter;
                str[1] = '\0';
                printk("%s", str);
                break;
            }
            else {
                break;
            }
        }
    }
}

void keyboard_init(void) {
    kb_in.count = 0;
    kb_in.head = kb_in.tail = kb_in.buff;
    register_interrupt_handler(IRQ1, &keyboard_read);
    enable_irq(IRQ1);
    printk_info("keyboard_init\n");
    return;
}

#ifdef __cplusplus
}
#endif
