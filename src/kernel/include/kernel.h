
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// kernel.h for Simple-XX/SimpleKernel.

#ifndef _KERNEL_H_
#define _KERNEL_H_

#include "stdint.h"
#include "stdarg.h"
#include "string.h"
#include "cxxabi.h"
#include "color.h"

#if defined(__i386__) || defined(__x86_64__)
#include "vga.h"
#endif

#include "io.h"
#include "console.h"

extern "C" uint8_t kernel_start[];
extern "C" uint8_t kernel_text_start[];
extern "C" uint8_t kernel_text_end[];
extern "C" uint8_t kernel_data_start[];
extern "C" uint8_t kernel_data_end[];
extern "C" uint8_t kernel_end[];

template <class video_t>
class KERNEL {
private:
    video_t          video;
    CONSOLE<video_t> console;

protected:
public:
    KERNEL(void);
    ~KERNEL(void);
    IO<video_t> io;
    int         init(void);
    void        show_info(void);
};

template <class video_t>
KERNEL<video_t>::KERNEL(void)
    : video(video_t()), console(CONSOLE(video)), io(IO(console)) {
    return;
}

template <class video_t>
KERNEL<video_t>::~KERNEL(void) {
    return;
}

template <class video_t>
void KERNEL<video_t>::show_info(void) {
    io.printf(LIGHT_GREEN, "kernel in memory start: 0x%08X, end 0x%08X\n",
              kernel_start, kernel_end);
    io.printf(LIGHT_GREEN, "kernel in memory size: %d KB, %d pages\n",
              (kernel_end - kernel_start) / 1024,
              (kernel_end - kernel_start + 4095) / 1024 / 4);
    io.printf(LIGHT_GREEN, "Simple Kernel.\n");
    return;
}

template <class video_t>
int32_t KERNEL<video_t>::init(void) {
    cpp_init();
    video.init();
    console.init();
    io.init();
    show_info();
    return 0;
}

#endif /* _KERNEL_H_ */
