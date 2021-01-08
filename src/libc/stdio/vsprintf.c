
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
// Based on Linux kernel 0.11
// vsprintf.c for Simple-XX/SimpleKernel.

// TODO
// 修复整数和其它类型数据同时输出时只显示整数的问题

#ifdef __cplusplus
extern "C" {
#endif

#include "stdarg.h"
#include "stddef.h"
#include "stdint.h"
#include "string.h"

// 判断字符是否数字字符
#define is_digit(c) ((c) >= '0' && (c) <= '9')

// 该函数将字符数字转换成整数。输入是数字串指针的指针，返回值是结果数值。另外指针将前移。
static int skip_atoi(const char **s) {
    int i = 0;
    while (is_digit(**s)) {
        i = i * 10 + *((*s)++) - '0';
    }
    return i;
}

// 这里定义转换类型的各种符号常数
// pad with zero 填充 0
#define ZEROPAD 1
// unsigned/signed long 无符号/符号长整数
#define SIGN 2
// show plus 显示加
#define PLUS 4
// space if plus 如是加，则置空格
#define SPACE 8
// left justified 左调整
#define LEFT 16
// 0x
#define SPECIAL 32
// use 'abcdef' instead of 'ABCDEF'  使用小写字母
#define SMALL 64

// 除操作。n 被除数；base 除数。结果 n 为商，函数返回值为余数。
#if defined(i386) || defined(x86_64)
int32_t do_div(int *n, int base) {
    int32_t res = 0;
    res         = *n % base;
    *n          = *n / base;
    return res;
}
#elif RASPI2
#include "math.h"
int32_t do_div(int *n, int base) {
    int32_t res = 0;
    res         = modsi3(*n, base);
    *n          = divsi3(*n, base);
    return res;
}
#endif

// 将整数转换为指定进制的字符串。输入：num-整数；base-进制；size-字符串长度;precision-数字长度
// (精读)；type-类型选项。输出：str-字符串指针
static char *number(char *str, int num, int base, int size, int precision,
                    int type) {
    char        c, sign, tmp[36];
    const char *digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    int         i;
    // 若类型 type
    // 指出用小写字母，则定义小写字母集。若类型指出要左调整(靠左边界)，则屏蔽填零标志。
    // 若进制基数小于 2 或大于 36，则退出处理，也即本程序只能处理基数在 2-32
    // 之间的数
    if (type & SMALL) {
        digits = "0123456789abcdefghijklmnopqrstuvwxyz";
    }
    if (type & LEFT) {
        type &= ~ZEROPAD;
    }
    if (base < 2 || base > 36) {
        return 0;
    }
    // 若类型指出要填零，则置字符变量 c='0' (即 ''),否则 c
    // 等于空格字符。若类型指出是带符号数并且 数值 num 小于 0，则置符号变量
    // sign=负号，并使 num 取绝对值。否则如果类型指出是加号， 则置
    // sign=加号，否则若类型带空格标志则 sign=空格，否则置 0
    c = (type & ZEROPAD) ? '0' : ' ';
    if (type & SIGN && num < 0) {
        sign = '-';
        num -= num;
    }
    else {
        sign = (type & PLUS) ? '+' : ((type & SPACE) ? ' ' : 0);
    }
    // 若带符号，则宽度值减 1.若类型指出是特殊转换，则对于十六进制宽度再减少 2
    // 位(用于 0x)，对于 八进制宽度减 1 (用于八进制转换结果)
    if (sign) {
        size--;
    }
    if (type & SPECIAL) {
        if (base == 16) {
            size -= 2;
        }
        else if (base == 8) {
            size--;
        }
    }
    // 如果数值 num 为 0，则临时字符串='0'；否则根据给定的基数将数值 num
    // 转换成字符形式
    i = 0;
    if (num == 0) {
        tmp[i++] = '0';
    }
    else {
        while (num != 0) {
            tmp[i++] = digits[do_div(&num, base)];
        }
    }
    // 若数值字符个数大于精读值，则精度值扩展为数字个数值。宽度值减去用于存放数值字符的个数。
    if (i > precision) {
        precision = i;
    }
    size -= precision;
    // 从这里真正开始形成所需要的转换结果，并暂时放在字符串 str
    // 中。若类型中没有填零(ZEROPAD) 和左调整标志，则在 str
    // 中首先填放剩余宽度值指出的空格数。若需带符号位，则存入符号。
    if (!(type & (ZEROPAD + LEFT))) {
        while (size-- > 0) {
            *str++ = ' ';
        }
    }
    if (sign) {
        *str++ = sign;
    }
    // 若类型指出是特殊转换，则对于八进制转换结果头一位放置一个
    // '0';而对于十六进制则存放 '0x'.
    if (type & SPECIAL) {
        if (base == 8) {
            *str++ = '0';
        }
        else if (base == 16) {
            *str++ = '0';
            // 'X' 或 'x'
            *str++ = digits[33];
        }
    }
    // 若类型中没有左调整(左靠齐)标志，则在剩余宽度中存放 c 字符('0' 或空格)，见
    // 49 行。
    if (!(type & LEFT)) {
        while (size-- > 0) {
            *str++ = c;
        }
    }
    // 此时 i 存有数值 num 的数字个数。若数字个数小于精读值，则 str
    // 中放入(精度值 - i)个 '0'.
    while (i < precision--) {
        *str++ = '0';
    }
    // 将转换好的数字字符填入 str 中。共 i 个。
    while (i-- > 0) {
        *str++ = tmp[i];
    }
    // 若宽度值仍大于零，则表示类型标志中有左靠齐标志标志。则在剩余宽度中放入空格。
    while (size-- > 0) {
        *str++ = ' ';
    }
    // 返回转换好的字符串。
    return str;
}

// fmt 是格式字符串；args 是个数变化的值；buf 是输出字符缓冲区。
// 下面函数是送格式化输出到字符串。为了能在内核中使用格式化的输出，Linus
// 在内核实现了该 C 标准函数。 其中参数 fmt 是格式字符串；args
// 是个数变化的值；buf 是输出字符缓冲区。请参见本代码列表后面
// 的有关格式转换字符的介绍。
int32_t vsprintf(char *buf, const char *fmt, va_list args);
int32_t vsprintf(char *buf, const char *fmt, va_list args) {
    int32_t len;
    int32_t i;
    // 用于存放转换过程中的字符串
    char *   str;
    char *   s;
    int32_t *ip;
    // flags to number()
    uint32_t flags;
    // width of output field
    int32_t field_width;
    // min. # of digits for integers;max number of chars for from
    int32_t precision;
    // fields.  min.整数数字个数；max. 字符串中字符个数
    // 'h','l',or 'L' for integer fields
    int32_t qualifier __attribute__((unused));

    // 首先将字符指针指向
    // buf，然后扫描格式字符串，对各个格式转换只是进行相应的处理.
    // 格式转换指示字符串均以 '%' 开始，这里从 fmt 格式字符串中扫描
    // '%'，寻找格式转换字符串的开始。 不是格式指示的一般字符均被依次存入 str
    for (str = buf; *fmt; ++fmt) {
        if (*fmt != '%') {
            *str++ = *fmt;
            continue;
        }
        // 下面取得格式指示字符串中的标志域，并将标志常量放入 flags 变量中
        // prcess flags.
        flags = 0;
    repeat:
        // this also skips first '%'
        ++fmt;
        switch (*fmt) {
            // 左靠齐调整
            case '-': {
                flags |= LEFT;
                goto repeat;
            }
            // 放加号
            case '+': {
                flags |= PLUS;
                goto repeat;
            }
            // 放空格
            case ' ': {
                flags |= SPACE;
                goto repeat;
            }
            // 是特殊转换
            case '#': {
                flags |= SPECIAL;
                goto repeat;
            }
            // 要填零(即 ‘0’)
            case '0': {
                flags |= ZEROPAD;
                goto repeat;
            }
        }
        // 去参数字段宽度阈值放入 field_width
        // 变量中。若宽度域中是数值则直接取其为宽度值。若宽度域是 字符 '*'
        // ，表示下一参数指定宽度，调用 va_arg 取宽度值。若此时宽度值 <
        // 0，则该负数表示其带有 标志域 '-'
        // 标志(左靠齐)，因此需在标志变量中添入该标志，并将字段宽度值取为其绝对值。
        // get field field_width
        field_width = -1;
        if (is_digit(*fmt)) {
            field_width = skip_atoi(&fmt);
        }
        else if (*fmt == '*') {
            // it's the next argument     这里应该插入 ++fmt;
            field_width = va_arg(args, int);
            if (field_width < 0) {
                field_width = -field_width;
                flags |= LEFT;
            }
        }
        // 下面代码取格式转换串精度域，并放入 precision
        // 变量中。精度域的开始标志是  '.'。其处理过程与上面
        // 宽度域类似。若精度域中是数值则直接取其为精度值。若精度域中是字符
        // '*'，表示下一个参数指定精度。 因此调用 va_arg
        // 取精度值。若此时宽度值小于 0，则将字段精度值取为 0. get the
        // precision
        precision = -1;
        if (*fmt == '.') {
            ++fmt;
            if (is_digit(*fmt)) {
                precision = skip_atoi(&fmt);
            }
            else if (*fmt == '*') {
                // it's the next argument
                precision = va_arg(args, int);
            }
            if (precision < 0) {
                precision = 0;
            }
        }
        // 下面这段代码分析长度修饰符，并将其存入 qualifier 变量。(h,l,L
        // 的含义参见列表后的说明) get the conversion qualifier
        qualifier = -1;
        if (*fmt == 'h' || *fmt == 'l' || *fmt == 'L') {
            qualifier = *fmt;
            ++fmt;
        }
        // 下面分析转换指示符。如果转换指示符是
        // 'c'，则表示对应参数应是字符。此时如果标志域表明不是左
        // 对齐，则该字段前面放入宽度域值 -1
        // 个空格字符，然后再被放入参数字符。如果宽度域还大于 0，
        // 则表示为左对齐，则在参数字符后面添加宽度值 -1 个空格字符
        switch (*fmt) {
            case 'c': {
                if (!(flags & LEFT)) {
                    while (--field_width > 0) {
                        *str++ = ' ';
                    }
                }
                *str++ = (uint8_t)va_arg(args, int);
                while (--field_width > 0) {
                    *str++ = ' ';
                }
                break;
            }
            // 如果转换指示符是
            // 's',则表示对应参数是字符串。首先取参数字符串的长度，若其超过了精度域值，则
            // 拓展精度域=字符串长度。此时如果标志域表明不是左靠齐，则该字段前放入(宽度值-字符串长度)个空格
            // 字符。然后再放入参数字符串。如果宽度域还大于
            // 0，则表示为左靠齐，则在参数字符串后面添加(宽度值-
            // 字符串长度)个空格字符。
            case 's': {
                s   = va_arg(args, char *);
                len = strlen(s);
                if (precision < 0) {
                    precision = len;
                }
                else if (len > precision) {
                    len = precision;
                }
                if (!(flags & LEFT)) {
                    while (len < field_width--) {
                        *str++ = ' ';
                    }
                }
                for (i = 0; i < len; ++i) {
                    *str++ = *s++;
                }
                while (len < field_width--) {
                    *str++ = ' ';
                }
                break;
            }
            // 如果格式转换符是
            // 'o',则表示需要将对应的参数转换成八进制数的字符串。调用 number()
            // 函数处理。
            case 'o': {
                str = number(str, va_arg(args, uint32_t), 8, field_width,
                             precision, flags);
                break;
            }
            // 如果格式转换符是
            // 'p'，表示对应参数的一个指针类型。此时若该参数没有设置宽度域，则默认宽度为
            // 8， 并且需要添零。然后调用 number() 函数进行处理。
            case 'p': {
                if (field_width == -1) {
                    field_width = 8;
                    flags |= ZEROPAD;
                }
                str = number(str, (uint32_t)va_arg(args, void *), 16,
                             field_width, precision, flags);
                break;
            }
            // 若格式转换指示是 'x'  或
            // 'X'，则表示对应参数需打印成十六进制数输出。'x'
            // 表示用小写字母表示。
            case 'x': {
                flags |= SMALL;
                __attribute__((fallthrough));
            }
            case 'X': {
                str = number(str, va_arg(args, uint32_t), 16, field_width,
                             precision, flags);
                break;
            }
            // 如果格式转换字符是 'd','i' 或 'u'，则表示对应参数是整数。'd','i'
            // 代表符号整数， 因此需要加上带符号标志。'u' 代表无符号整数。
            case 'd':
            case 'i': {
                flags |= SIGN;
                __attribute__((fallthrough));
            }
            case 'u': {
                str = number(str, va_arg(args, uint32_t), 10, field_width,
                             precision, flags);
                break;
            }
            // 若格式转换指示符是
            // 'n'，则表示要把到目前为止转换输出的字符保存到对应参数指针指定的位置中。
            // 首先利用 va_arg()
            // 取得该参数指针，然后将已经转换好的字符存入该指针所指的位置。
            case 'n': {
                ip  = va_arg(args, int *);
                *ip = (str - buf);
                break;
            }
            // 若格式转换符不是 '%' ，则表示格式字符串有错，直接将一个 '%'
            // 写入输出中。如果格式转换符的
            // 位置处还有字符，则也直接将该字符写入输出串中，并返回到 117
            // 行处继续处理格式字符串。否则表示
            // 已经处理到格式字符串的结尾处，则退出循环。
            default: {
                if (*fmt != '%') {
                    *str++ = '%';
                }
                if (*fmt) {
                    *str++ = *fmt;
                }
                else {
                    --fmt;
                }
                break;
            }
        }
    }
    // 最后在转换好的字符串结尾处添上 null
    *str = '\0';
    // 返回转换好的字符串长度值
    return str - buf;
}

#ifdef __cplusplus
}
#endif
