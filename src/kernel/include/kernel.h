
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// kernel.h for Simple-XX/SimpleKernel.

#ifndef _KERNEL_H_
#define _KERNEL_H_

#include "console.h"
#include "debug.h"
#include "gdt.h"
#include "intr.h"
#include "clock.h"

void console_init(void);
void gdt_init(void);
void idt_init(void);
void clock_init(void);
void debug_init(void);
void showinfo(void);

extern addr_t kernel_start[];
extern addr_t kernel_text_start[];
extern addr_t kernel_text_end[];
extern addr_t kernel_data_start[];
extern addr_t kernel_data_end[];
extern addr_t kernel_end[];

extern "C" void kernel_main(void);

class KERNEL : virtual CONSOLE {
private:
    GDT   gdt;
    INTR  intr;
    CLOCK clock;
    DEBUG debug;

protected:
public:
    KERNEL(void);
    ~KERNEL(void);
    int32_t init(void);
    void    show_info(void);
};

#endif /* _KERNEL_H_ */
