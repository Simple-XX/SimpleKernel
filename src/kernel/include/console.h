
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// console.h for Simple-XX/SimpleKernel.

#ifndef _CONSOLE_H_
#define _CONSOLE_H_

#include "stddef.h"
#include "stdint.h"
#include "stdarg.h"

template <class video_t>
class CONSOLE {
private:
    // 命令行行数
    size_t rows;
    // 命令行列数
    size_t cols;
    // 当前位置
    size_t curr_row;
    size_t curr_col;
    // 当前命令行颜色
    color_t color;
    // 清屏
    void clear(void);
    // 转义字符处理
    void escapeconv(const char c);
    // 滚动显示
    void scroll(void);
    // 指定位置输出
    void     put_entry_at(const char c, const color_t color, const size_t x,
                          const size_t y);
    video_t &video;

protected:
public:
    CONSOLE(video_t &video);
    ~CONSOLE(void);
    int32_t init(void);
    // 写字符
    void put_char(const char c);
    // 写字符串
    void write_string(const char *s);
    // 写字符串
    void write(const char *s, size_t len);
    // 设置颜色
    void set_color(const color_t color);
    // 获取颜色
    color_t get_color(void);
    // 设置行列
    void set_row(size_t row);
    void set_col(size_t col);
    void set_rows(size_t rows);
    void set_cols(size_t cols);
};

template <class video_t>
CONSOLE<video_t>::CONSOLE(video_t &video) : video(video) {
    rows     = video.get_height();
    cols     = video.get_width();
    color    = video.gen_color(LIGHT_GREY, BLACK);
    curr_row = 0;
    curr_col = 0;
    return;
}

template <class video_t>
CONSOLE<video_t>::~CONSOLE(void) {
    return;
}

template <class video_t>
int32_t CONSOLE<video_t>::init(void) {
    clear();
    this->write_string("console init\n");
    return 0;
}

template <class video_t>
void CONSOLE<video_t>::put_entry_at(const char c, const color_t color,
                                    const size_t x, const size_t y) {
    const size_t index = y * cols + x;
    video.write(index, video.gen_char(c, color));
    return;
}

template <class video_t>
void CONSOLE<video_t>::clear(void) {
    // 从左上角开始
    for (size_t y = 0; y < rows; y++) {
        for (size_t x = 0; x < cols; x++) {
            const size_t index = y * cols + x;
            // 用 ' ' 填满屏幕
            // 字体为灰色，背景为黑色
            video.write(index, video.gen_char(' ', color));
        }
    }
    video.set_cursor_pos(0, 0);
    return;
}

template <class video_t>
void CONSOLE<video_t>::scroll(void) {
    if (curr_row >= rows) {
        // 将所有行的显示数据复制到上一行
        for (size_t i = 0; i < (rows - 1) * cols; i++) {
            video.write(i, video.read(i + cols));
        }
        // 最后的一行数据现在填充空格，不显示任何字符
        for (size_t i = (rows - 1) * cols; i < rows * cols; i++) {
            video.write(i, video.gen_char(' ', color));
        }
        // 向上移动了一行，所以 cursor_y 现在是 24
        curr_row = 24;
    }
    return;
}

template <class video_t>
void CONSOLE<video_t>::write_string(const char *data) {
    write(data, strlen(data));
    return;
}

template <class video_t>
void CONSOLE<video_t>::write(const char *data, size_t size) {
    for (size_t i = 0; i < size; i++) {
        put_char(data[i]);
    }
    return;
}

template <class video_t>
void CONSOLE<video_t>::put_char(const char c) {
    put_entry_at(c, color, curr_col, curr_row);
    // 如果到达最后一列则换行
    if (++curr_col >= cols) {
        curr_col = 0;
        curr_row++;
    }
    // 转义字符处理
    escapeconv(c);
    // 屏幕滚动
    scroll();
    video.set_cursor_pos(curr_col, curr_row);
    return;
}

template <class video_t>
void CONSOLE<video_t>::escapeconv(const char c) {
    switch (c) {
        case '\n': {
            curr_col = 0;
            curr_row++;
            break;
        }
        case '\t': {
            // 取整对齐
            curr_col = (curr_col + 7) & ~7;
            // 如果到达最后一列则换行
            if (++curr_col >= cols) {
                curr_col = 0;
                curr_row++;
            }
            break;
        }
        case '\b': {
            if (curr_col) {
                curr_col -= 2;
            }
            break;
        }
    }
    return;
}

template <class video_t>
void CONSOLE<video_t>::set_color(const color_t color) {
    this->color = color;
    return;
}

template <class video_t>
color_t CONSOLE<video_t>::get_color(void) {
    return this->color;
}

template <class video_t>
void CONSOLE<video_t>::set_row(size_t row) {
    curr_row = row;
    return;
}

template <class video_t>
void CONSOLE<video_t>::set_col(size_t col) {
    curr_col = col;
    return;
}

template <class video_t>
void CONSOLE<video_t>::set_rows(size_t rows) {
    this->rows = rows;
    return;
}

template <class video_t>
void CONSOLE<video_t>::set_cols(size_t cols) {
    this->cols = cols;
    return;
}

#endif /* _CONSOLE_H_ */
