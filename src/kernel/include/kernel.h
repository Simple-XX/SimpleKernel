
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// kernel.h for Simple-XX/SimpleKernel.

#ifndef _KERNEL_H_
#define _KERNEL_H_

#include "stdint.h"
#include "io.h"

#if defined(__i386__) || defined(__x86_64__)
#include "vga.h"
#include "console.h"
#endif

extern "C" uint8_t kernel_start[];
extern "C" uint8_t kernel_text_start[];
extern "C" uint8_t kernel_text_end[];
extern "C" uint8_t kernel_data_start[];
extern "C" uint8_t kernel_data_end[];
extern "C" uint8_t kernel_end[];

extern "C" void kernel_main(void);

class KERNEL {
private:
#if defined(__i386__) || defined(__x86_64__)
    VGA     vga;
    CONSOLE console;
#endif
    IO io;

protected:
public:
    KERNEL(void);
    ~KERNEL(void);
    int  init(void);
    void show_info(void);
    // 格式输出
    int32_t printf(const char *format, ...);
    int32_t printf(color_t color, const char *format, ...);
};

#endif /* _KERNEL_H_ */
