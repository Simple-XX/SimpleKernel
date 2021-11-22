
/**
 * @file tui.cpp
 * @brief TUI 接口头文件
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2021-09-18
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2021-09-18<td>digmouse233<td>迁移到 doxygen
 * </table>
 */

#ifndef _TUI_H_
#define _TUI_H_

#include "stdint.h"
#include "stddef.h"
#include "color.h"

/**
 * @brief 位置信息
 */
class pos_t {
public:
    /**
     * @brief 构造一个位置对象
     * @param  _col             列
     * @param  _row             行
     */
    pos_t(const uint8_t _col, const uint8_t _row);
    ~pos_t(void);
    // 列
    uint8_t col;
    // 行
    uint8_t row;
};

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

/**
 * @brief TUI 接口
 */
class TUI {
private:
    /// CRT 控制寄存器-地址
    static constexpr const uint32_t TUI_ADDR = 0x3D4;
    /// CRT 控制寄存器-数据
    static constexpr const uint32_t TUI_DATA = 0x3D5;
    /// 光标高位
    static constexpr const uint32_t TUI_CURSOR_H = 0xE;
    /// 光标低位
    static constexpr const uint32_t TUI_CURSOR_L = 0xF;
    /// TUI 缓存基址
    /// @todo 从 grub 获取
    static constexpr const uint32_t TUI_MEM_BASE = 0xB8000;
    /// TUI 缓存大小
    /// @todo 从 grub 获取
    static constexpr size_t TUI_MEM_SIZE = 0x8000;
    /// 规定显示行数
    /// @todo 从 grub 获取
    static constexpr const size_t WIDTH = 80;
    /// 规定显示列数
    /// @todo 从 grub 获取
    static constexpr const size_t HEIGHT = 25;
    // TUI 缓存
    char_t *const buffer = (char_t *)TUI_MEM_BASE;
    /// 记录当前位置
    static pos_t pos;
    /// 记录当前命令行颜色
    static col_t color;
    /**
     * @brief 在指定位置输出
     * @param  _c              字符
     * @param  _color          颜色
     * @param  _x              行
     * @param  _y              列
     */
    void put_entry_at(const char _c, const col_t _color, const size_t _x,
                      const size_t _y);
    /**
     * @brief 转义字符处理
     * @param  _c              要处理的字符
     * @return true            _c 是一个转义字符
     * @return false           _c 不是一个转义字符
     */
    bool escapeconv(const char _c);
    /**
     * @brief 滚动显示
     */
    void scroll(void);

protected:
public:
    TUI(void);
    ~TUI(void);
    /**
     * @brief 设置颜色
     * @param  _color           要设置的颜色
     * @todo 支持分别字体与背景色
     */
    void set_color(const COLOR::color_t _color);
    /**
     * @brief 获取颜色
     * @return COLOR::color_t  正在使用的颜色
     */
    COLOR::color_t get_color(void) const;
    /**
     * @brief 设置光标位置
     * @param  _pos            要设置的位置
     */
    void set_pos(const pos_t _pos);
    /**
     * @brief 设置行
     * @param  _row            要设置的行
     */
    void set_pos_row(const size_t _row);
    /**
     * @brief 设置列
     * @param  _col            要设置的列
     */
    void set_pos_col(const size_t _col);
    /**
     * @brief 获取光标位置
     * @return pos_t           光标的位置
     */
    pos_t get_pos(void) const;
    /**
     * @brief 写 TUI 缓存
     * @param  _idx            要写的位置
     * @param  _data           要写的数据
     */
    void write(const size_t _idx, const char_t _data);
    /**
     * @brief 读 TUI 缓存
     * @param  _idx            要读的位置
     * @return char_t          该位置处的 char_t 对象
     */
    char_t read(const size_t _idx) const;
    /**
     * @brief 写字符
     * @param  _c              要写的字符
     */
    void put_char(const char _c);
    /**
     * @brief 写字符串
     * @param  _s              要写的字符串
     */
    void write_string(const char *_s);
    /**
     * @brief 写指定长度的字符串
     * @param  _s              要写的字符串
     * @param  _len            要写的长度
     */
    void write(const char *_s, const size_t _len);
    /**
     * @brief 清屏，清空 TUI 缓存
     */
    void clear(void);
    /**
     * @brief 读一个字符
     * @return uint8_t         读取到的字符
     * @note TUI 不支持，这里仅仅是保持接口一致
     */
    uint8_t get_char(void) const;
};

#endif /* _TUI_H_ */
