
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// stdarg.h for Simple-XX/SimpleKernel.

#ifndef _STDARG_H_
#define _STDARG_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef char *va_list;

// 用来得到 TYPE
// 元素类型的字节大小，若不足4字节(short,char),那么认为这个元素的大小为4字节，
// 简单的说就是检测元素的大小，不足4字节的当作4字节看待.
#define __va_SIZE(TYPE)                                                        \
    (((sizeof(TYPE) + sizeof(int) - 1) / sizeof(int)) * sizeof(int))

// AP 一般都是 va_list，LASTARG 则是指向参数变长函数的格式化字符串的指针,
// va_start 宏根据 (va_list)&LASTARG
// 得到第一个可变参数前的一个固定参数在堆栈中的内存地址， 加上
// __va_SIZEOF(TYPE)即 TYPE
// 所占内存大小后，使ap指向固定参数后下个参数(第一个可变参数地址)。 va_start
// 的作用就很明显了。取得变长参数列表的第一个参数的地址。
#define va_start(AP, LASTARG)                                                  \
    (AP = ((va_list) & (LASTARG) + __va_SIZE(LASTARG)))

// 取得 TYPE 类型的可变参数值。首先 AP+=__va_SIZE(TYPE)，即 AP
// 跳过当前可变参数而指向下个变参的地址； 然后
// AP-__va_SIZE(TYPE)得到当前变参的内存地址，类型转换后返回当前变参值。 #define
// va_arg(AP, TYPE) (*(TYPE *)((AP+=__va_SIZE(TYPE))-__va_SIZE(TYPE)))
#define va_arg(AP, TYPE)                                                       \
    (AP += __va_SIZE(TYPE), *((TYPE *)(AP - __va_SIZE(TYPE))))

// 把指针 va_list 置 0
#define va_end(AP) (AP = (va_list)0)

#ifdef __cplusplus
}
#endif

#endif /* _STDARG_H_ */
