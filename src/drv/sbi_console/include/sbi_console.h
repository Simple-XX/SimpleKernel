
/**
 * @file sbi_console.h
 * @brief opensbi console 接口头文件
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

#ifndef _SBI_CONSOLE_H_
#define _SBI_CONSOLE_H_

#include "stdint.h"
#include "color.h"

class SBI_CONSOLE {
private:
    /// 当前命令行颜色
    static COLOR::color_t color;

protected:
public:
    SBI_CONSOLE(void);
    ~SBI_CONSOLE(void);

    /**
     * @brief 写字符
     * @param  _c              要写的字符
     */
    void put_char(const char _c) const;

    /**
     * @brief 写字符串
     * @param  _s              要写的字符串
     */
    void write_string(const char *_s) const;

    /**
     * @brief 写指定长度的字符串
     * @param  _s              要写的字符串
     * @param  _len            要写的长度
     */
    void write(const char *_s, size_t _len) const;

    /**
     * @brief 读一个字符
     * @return uint8_t         读取到的字符
     * @todo
     */
    uint8_t get_char(void) const;

    /**
     * @brief 设置颜色
     * @param  _color          要设置的颜色
     * @todo
     */
    void set_color(const COLOR::color_t _color) const;

    /**
     * @brief 获取正在使用的颜色
     * @return COLOR::color_t  使用的颜色
     * @todo
     */
    COLOR::color_t get_color(void) const;
};

#endif /* _SBI_CONSOLE_H_ */
