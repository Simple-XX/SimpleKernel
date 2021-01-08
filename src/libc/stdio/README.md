# src/include/libc/stdio

- printk.c

  实现了输出函数printk，分别支持info、debug、test、err等情况下的不同颜色显示。

- vsprintf.c

  实现了输出函数vsprintk函数，这个函数用于固定参数输出到字符串。可以参考

  https://blog.csdn.net/heybeaman/article/details/80495846

  http://www.cplusplus.com/reference/cstdio/vsprintf/

  