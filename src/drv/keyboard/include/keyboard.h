
/**
 * @file keyboard.h
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

#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

#include "stdint.h"
#include "intr.h"

/**
 * @brief 键盘接口
 */
class KEYBOARD {
private:
    /// 键盘缓冲区大小
    static constexpr const uint32_t KB_BUFSIZE = 128;

    static constexpr const uint32_t KB_DATA   = 0x60;
    static constexpr const uint32_t KB_WRITE  = 0x60;
    static constexpr const uint32_t KB_STATUS = 0x64;
    static constexpr const uint32_t KB_CMD    = 0x64;

    /// 特殊键的扫描码
    static constexpr const uint32_t KB_BACKSPACE = 0x0E;
    static constexpr const uint32_t KB_ENTER     = 0x1C;
    static constexpr const uint32_t KB_TAB       = 0x0F;
    static constexpr const uint32_t KB_ESC       = 0x01;
    static constexpr const uint32_t KB_SHIFT_L   = 0x2A;
    static constexpr const uint32_t KB_SHIFT_R   = 0x36;
    static constexpr const uint32_t KB_ALT_L     = 0x38;
    static constexpr const uint32_t KB_CAPS_LOCK = 0x3A;
    static constexpr const uint32_t KB_CTRL_L    = 0x1D;
    static constexpr const uint32_t KB_NUM_LOCK  = 0x45;

    /// Number of columns in keyma
    static constexpr const uint32_t MAP_COLS = 3;
    /// Number of scan codes (rows in keymap)
    static constexpr const uint32_t NR_SCAN_CODES = 0x80;
    static constexpr const uint32_t SC_MAX        = NR_SCAN_CODES * MAP_COLS;

    /// 8 位的键盘扫描码的接通码使用前7位
    /// 其最高位置 1 即是其对应的断开码
    /// 该宏可以和获取的扫描码用来判断一个键是按下还是抬起
    static constexpr const uint32_t RELEASED_MASK = 0x80;

    /// Normal function keys
    static constexpr const uint32_t FLAG_EXT = 0x00;

    /// Special keys
    /// Esc
    static constexpr const uint32_t ESC = (0x01 & FLAG_EXT);
    /// Tab
    static constexpr const uint32_t TAB = (0x02 & FLAG_EXT);
    /// Enter
    static constexpr const uint32_t ENTER = (0x03 & FLAG_EXT);
    /// BackSpace
    static constexpr const uint32_t BACKSPACE = (0x04 & FLAG_EXT);

    /// L GUI
    static constexpr const uint32_t GUI_L = (0x05 & FLAG_EXT);
    /// R GUI
    static constexpr const uint32_t GUI_R = (0x06 & FLAG_EXT);
    /// APPS
    static constexpr const uint32_t APPS = (0x07 & FLAG_EXT);

    /// Shift, Ctrl, Alt
    /// L Shift
    static constexpr const uint32_t SHIFT_L = (0x08 & FLAG_EXT);
    /// R Shift
    static constexpr const uint32_t SHIFT_R = (0x09 & FLAG_EXT);
    /// L Ctrl
    static constexpr const uint32_t CTRL_L = (0x0A & FLAG_EXT);
    /// R Ctrl
    static constexpr const uint32_t CTRL_R = (0x0B & FLAG_EXT);
    /// L Alt
    static constexpr const uint32_t ALT_L = (0x0C & FLAG_EXT);
    /// R Alt
    static constexpr const uint32_t ALT_R = (0x0D & FLAG_EXT);

    /// Lock keys
    /// Caps Lock
    static constexpr const uint32_t CAPS_LOCK = (0x0E & FLAG_EXT);
    /// Number Lock
    static constexpr const uint32_t NUM_LOCK = (0x0F & FLAG_EXT);
    /// Scroll Lock
    static constexpr const uint32_t SCROLL_LOCK = (0x10 & FLAG_EXT);

    /// Function keys
    /// F1
    static constexpr const uint32_t F1 = (0x11 & FLAG_EXT);
    /// F2
    static constexpr const uint32_t F2 = (0x12 & FLAG_EXT);
    /// F3
    static constexpr const uint32_t F3 = (0x13 & FLAG_EXT);
    /// F4
    static constexpr const uint32_t F4 = (0x14 & FLAG_EXT);
    /// F5
    static constexpr const uint32_t F5 = (0x15 & FLAG_EXT);
    /// F6
    static constexpr const uint32_t F6 = (0x16 & FLAG_EXT);
    /// F7
    static constexpr const uint32_t F7 = (0x17 & FLAG_EXT);
    /// F8
    static constexpr const uint32_t F8 = (0x18 & FLAG_EXT);
    /// F9
    static constexpr const uint32_t F9 = (0x19 & FLAG_EXT);
    /// F10
    static constexpr const uint32_t F10 = (0x1A & FLAG_EXT);
    /// F11
    static constexpr const uint32_t F11 = (0x1B & FLAG_EXT);
    /// F12
    static constexpr const uint32_t F12 = (0x1C & FLAG_EXT);

    /// Control Pad
    /// Print Screen
    static constexpr const uint32_t PRINTSCREEN = (0x1D & FLAG_EXT);
    /// Pause/Break
    static constexpr const uint32_t PAUSEBREAK = (0x1E & FLAG_EXT);
    /// Insert
    static constexpr const uint32_t INSERT = (0x1F & FLAG_EXT);
    /// Delete
    static constexpr const uint32_t DELETE = (0x20 & FLAG_EXT);
    /// Home
    static constexpr const uint32_t HOME = (0x21 & FLAG_EXT);
    /// End
    static constexpr const uint32_t END = (0x22 & FLAG_EXT);
    /// Page Up
    static constexpr const uint32_t PAGEUP = (0x23 & FLAG_EXT);
    /// Page Down
    static constexpr const uint32_t PAGEDOWN = (0x24 & FLAG_EXT);
    /// Up
    static constexpr const uint32_t UP = (0x25 & FLAG_EXT);
    /// Down
    static constexpr const uint32_t DOWN = (0x26 & FLAG_EXT);
    /// Left
    static constexpr const uint32_t LEFT = (0x27 & FLAG_EXT);
    /// Right
    static constexpr const uint32_t RIGHT = (0x28 & FLAG_EXT);

    /// ACPI keys
    /// Power
    static constexpr const uint32_t POWER = (0x29 & FLAG_EXT);
    /// Sleep
    static constexpr const uint32_t SLEEP = (0x2A & FLAG_EXT);
    /// Wake Up
    static constexpr const uint32_t WAKE = (0x2B & FLAG_EXT);

    /// Num Pad
    /// /
    static constexpr const uint32_t PAD_SLASH = (0x2C & FLAG_EXT);
    /// *
    static constexpr const uint32_t PAD_STAR = (0x2D & FLAG_EXT);
    /// -
    static constexpr const uint32_t PAD_MINUS = (0x2E & FLAG_EXT);
    /// &
    static constexpr const uint32_t PAD_PLUS = (0x2F & FLAG_EXT);
    /// Enter
    static constexpr const uint32_t PAD_ENTER = (0x30 & FLAG_EXT);
    /// .
    static constexpr const uint32_t PAD_DOT = (0x31 & FLAG_EXT);
    /// 0
    static constexpr const uint32_t PAD_0 = (0x32 & FLAG_EXT);
    /// 1
    static constexpr const uint32_t PAD_1 = (0x33 & FLAG_EXT);
    /// 2
    static constexpr const uint32_t PAD_2 = (0x34 & FLAG_EXT);
    /// 3
    static constexpr const uint32_t PAD_3 = (0x35 & FLAG_EXT);
    /// 4
    static constexpr const uint32_t PAD_4 = (0x36 & FLAG_EXT);
    /// 5
    static constexpr const uint32_t PAD_5 = (0x37 & FLAG_EXT);
    /// 6
    static constexpr const uint32_t PAD_6 = (0x38 & FLAG_EXT);
    /// 7
    static constexpr const uint32_t PAD_7 = (0x39 & FLAG_EXT);
    /// 8
    static constexpr const uint32_t PAD_8 = (0x3A & FLAG_EXT);
    /// 9
    static constexpr const uint32_t PAD_9 = (0x3B & FLAG_EXT);
    /// Up
    static constexpr const uint32_t PAD_UP = PAD_8;
    /// Down
    static constexpr const uint32_t PAD_DOWN = PAD_2;
    /// Left
    static constexpr const uint32_t PAD_LEFT = PAD_4;
    /// Right
    static constexpr const uint32_t PAD_RIGHT = PAD_6;
    /// Home
    static constexpr const uint32_t PAD_HOME = PAD_7;
    /// End
    static constexpr const uint32_t PAD_END = PAD_1;
    /// Page Up
    static constexpr const uint32_t PAD_PAGEUP = PAD_9;
    /// Page Down
    static constexpr const uint32_t PAD_PAGEDOWN = PAD_3;
    /// Ins
    static constexpr const uint32_t PAD_INS = PAD_0;
    /// Middle key
    static constexpr const uint32_t PAD_MID = PAD_5;
    /// Del
    static constexpr const uint32_t PAD_DEL = PAD_DOT;
    /// 键盘映射
    static constexpr const uint8_t keymap[NR_SCAN_CODES * MAP_COLS] = {
        /* scan-code			!Shift		Shift		E0 XX
         */
        /* ====================================================================
         */
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
        0,
    };
    bool shift;
    bool caps;
    bool ctrl;
    bool num;
    bool alt;

protected:
public:
    KEYBOARD(void);
    ~KEYBOARD(void);

    /**
     * @brief 初始化
     * @return int32_t         成功返回 0
     */
    int32_t init(void);

    /**
     * @brief 从键盘读
     * @return uint8_t         读到的数据
     */
    uint8_t read(void);

    /**
     * @brief 设置键盘中断处理函数
     * @param  _h               处理函数
     * @return int32_t          设置成功返回 0
     */
    int32_t set_handle(INTR::interrupt_handler_t _h);
};

extern KEYBOARD keyboard;

#endif /* _KEYBOARD_H_ */
