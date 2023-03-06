# SimpleKernel 基本输出

在内核顺利启动后，为了方便以后的调试，应该优先实现字符的输出功能。

为了满足不同平台的接口一致，抽象出了一个静态类 `IO` 来统一端口读写、MMIO、以及输出调用。

- 相关代码

    ./src/include/io.h

    ./src/kernel/io.cpp

    ./src/drv/sbi_console，riscv 架构使用下基于 opensbi 的串口输出

    ./src/drv/tui，IA32 下使用的 text user interface 输出

    ./src/drv/uart，通用 uart 输出

## IA32

IA32 提供了一套名为 TUI(text user interface) 的东西，它允许我们通过读写端口与内存的方式在屏幕上输出文字,它是我们在 IA32 上实现基本输出的基础。

使用这套系统有三个主要部分：

1. 光标位置

    通过读写 `TUI_ADDR` 与 `TUI_DATA` 两个端口实现光标位置的读取与设置。

    ```c++
    void TUI::set_pos(const pos_t _pos) {
        const uint16_t index = _pos.row * this->WIDTH + _pos.col;
        this->pos            = _pos;
        // 光标的设置，见参考资料
        // 告诉 TUI 我们要设置光标的高字节
        PORT::outb(TUI_ADDR, TUI_CURSOR_H);
        // 发送高 8 位
        PORT::outb(TUI_DATA, index >> 8);
        // 告诉 TUI 我们要设置光标的低字节
        PORT::outb(TUI_ADDR, TUI_CURSOR_L);
        // 发送低 8 位
        PORT::outb(TUI_DATA, index);
        return;
    }
    
    pos_t TUI::get_pos(void) const {
        PORT::outb(TUI_ADDR, TUI_CURSOR_H);
        size_t cursor_pos_h = PORT::inb(TUI_DATA);
        PORT::outb(TUI_ADDR, TUI_CURSOR_L);
        size_t cursor_pos_l = PORT::inb(TUI_DATA);
        // 返回光标位置
        return pos_t(cursor_pos_l, cursor_pos_h);
    }
    ```

    

2. 字符及其颜色

    要在屏幕上输出字符，需要向地址 0xB8000 写入数据，数据格式为

    ```c++
    /**
     * @brief 颜色信息
     */
    class col_t {
    public:
        /**
         * @brief 构造一个颜色对象
         * @param  _fore            前景色
         * @param  _back            背景色
         */
        col_t(const COLOR::color_t _fore, const COLOR::color_t _back);

        ~col_t(void);
        // 前景色
        uint8_t fore : 4;
        // 背景色
        uint8_t back : 4;
    };
 
    /**
     * @brief 内存中的字符对象
     */
    class char_t {
    public:
        char_t(void);
 
        /**
         * @brief 构造一个内存中的字符对象
         * @param  _c               字符
         * @param  _color           颜色
         */
        char_t(const uint8_t _c, const col_t _color);
 
        ~char_t(void);
        // 字符
        uint8_t c;
        // 颜色
        col_t color;
    };
    ```

    共计 2 字节大小。

    其中第一字节为要显示的 ASCII 字符，后面 8 位的高 4 位与低 4 位分别表示前景颜色与背景颜色。

3. 缓存地址

    在我们的设置中，TUI 的缓存地址从 0xB8000 开始，大小为 80\*25\*2=4000 字节。
    
    其中，
    
    80=列数，
    
    25=行数，
    
    2=字符及其颜色数据大小

- 相关代码

    ./src/include/color.h

    ./src/drv/tui/include/tui.h

    ./src/drv/tui/tui.cpp

更多细节请查看注释。

## RISCV

riscv 通过 uart 进行输出，但我们并不需要自己写 uart 驱动，opensbi 提供了现成的接口，直接调用即可。

```c++
void OPENSBI::put_char(const char _c) {
    ecall(_c, 0, 0, 0, 0, 0, FID_CONSOLE_PUTCHAR, EID_CONSOLE_PUTCHAR);
    return;
}
```

- 颜色设置

  由于实际使用的是串口，所以可以通过控制字符来实现对颜色的控制
  ```c++
  void SBI_CONSOLE::set_color(const COLOR::color_t _color) const {
      // 根据 _color 构造字符串
      char *tmp = nullptr;
      switch (_color) {
          case COLOR::BLACK: {
              tmp = (char *)"\033[30m";
              break;
          }
          case COLOR::RED: {
              tmp = (char *)"\033[31m";
              break;
          }
          case COLOR::GREEN: {
              tmp = (char *)"\033[32m";
              break;
          }
          case COLOR::YELLOW: {
              tmp = (char *)"\033[33m";
              break;
          }
          case COLOR::BLUE: {
              tmp = (char *)"\033[34m";
              break;
          }
          case COLOR::PURPLE: {
              tmp = (char *)"\033[35m";
              break;
          }
          case COLOR::CYAN: {
              tmp = (char *)"\033[36m";
              break;
          }
          case COLOR::WHITE: {
              tmp = (char *)"\033[37m";
              break;
          }
          case COLOR::GREY: {
              tmp = (char *)"\033[90m";
              break;
          }
          case COLOR::LIGHT_RED: {
              tmp = (char *)"\033[91m";
              break;
          }
          case COLOR::LIGHT_GREEN: {
              tmp = (char *)"\033[92m";
              break;
          }
          case COLOR::LIGHT_YELLOW: {
              tmp = (char *)"\033[93m";
              break;
          }
          case COLOR::LIGHT_BLUE: {
              tmp = (char *)"\033[94m";
              break;
          }
          case COLOR::LIGHT_PURPLE: {
              tmp = (char *)"\033[95m";
              break;
          }
          case COLOR::LIGHT_CYAN: {
              tmp = (char *)"\033[96m";
              break;
          }
          case COLOR::LIGHT_WHITE: {
              tmp = (char *)"\033[97m";
              break;
          }
      }
      // 写入
      write_string(tmp);
      color = _color;
      return;
  }
  ```

opensbi 接口文档：[riscv-sbi-doc](https://github.com/riscv-non-isa/riscv-sbi-doc/blob/master/riscv-sbi.adoc)

- 相关代码

    ./src/include/color.h

    ./src/drv/opensbi/include/opensbi.h

    ./src/drv/opensbi/opensbi.cpp

    ./src/drv/sbi_console/include/sbi_console.h

    ./src/drv/sbi_console/sbi_console.cpp

更多细节请查看注释。

## 相关文档

opensbi：https://github.com/riscv/opensbi

IA32 prots：https://wiki.osdev.org/I/O_Ports

IA32 VGA：https://wiki.osdev.org/VGA_Hardware

IA32 text ui：https://wiki.osdev.org/Text_UI

IA32 text 模式的光标：https://wiki.osdev.org/Text_Mode_Cursor

IA32 text 模式在屏幕上输出：https://wiki.osdev.org/Printing_To_Screen

