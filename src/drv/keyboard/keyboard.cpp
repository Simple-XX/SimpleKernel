
/**
 * @file keyboard.cpp
 * @brief 中断抽象头文件
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2021-09-18
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * Based on Orange's 一个操作系统的实现
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2021-09-18<td>digmouse233<td>迁移到 doxygen
 * </table>
 */

#include "stddef.h"
#include "stdbool.h"
#include "io.h"
#include "stdio.h"
#include "keyboard.h"

/**
 * @brief 默认处理函数
 */
static void default_keyboard_handle(INTR::intr_context_t *) {
    keyboard.read();
    return;
}

KEYBOARD::KEYBOARD(void) {
    shift = false;
    caps  = false;
    ctrl  = false;
    num   = true;
    alt   = false;
    return;
}

KEYBOARD::~KEYBOARD(void) {
    return;
}

uint8_t KEYBOARD::read(void) {
    uint8_t scancode = io.inb(KB_DATA);
    // 判断是否出错
    if (!scancode) {
        warn("scancode error.\n");
        return '\0';
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
            letter = '\b';
            break;
        case KB_ENTER:
            io.put_char('\n');
            letter = '\n';
            break;
        case KB_TAB:
            io.put_char('\t');
            letter = '\t';
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
    return letter;
}

int32_t KEYBOARD::init(void) {
    set_handle(&default_keyboard_handle);
    INTR::enable_irq(INTR::IRQ1);
    printf("keyboard init.\n");
    return 0;
}

int32_t KEYBOARD::set_handle(INTR::interrupt_handler_t h) {
    INTR::register_interrupt_handler(INTR::IRQ1, h);
    return 0;
}

KEYBOARD keyboard;
