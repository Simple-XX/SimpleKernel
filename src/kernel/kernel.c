
// // This file is a part of Simple-XX/SimpleKernel
// // (https://github.com/Simple-XX/SimpleKernel).
// //
// // kernel.c for Simple-XX/SimpleKernel.

// #ifdef __cplusplus
// extern "C" {
// #endif

// #include "kernel.h"
// #include "cpu.hpp"

// // 内核入口
// void kernel_main() {
//     // 系统初始化
//     debug_init();
//     // 控制台初始化
//     console_init();
//     // GDT 初始化
//     gdt_init();
//     // IDT 初始化
//     idt_init();
//     // 时钟初始化
//     clock_init();

//     showinfo();

//     cpu_sti();
//     printk_color(white, "sti\n");
//     if (EFLAGS_IF_status()) {
//         printk_color(white, "interrupt accept!\n");
//     }
//     else {
//         printk_color(light_red, "interrupt not accept!\n");
//     }

//     // close intr
//     cpu_cli();
//     for (int i = 0; i < 5; i++) {
//         if (EFLAGS_IF_status()) {
//             printk_color(white, "before hlt, interrupt accept!\n");
//         }
//         else {
//             printk_color(light_red, "before hlt, interrupt not accept!\n");
//         }

//         cpu_hlt();

//         if (EFLAGS_IF_status()) {
//             printk_color(white, "after hlt, interrupt accept!\n\n");
//         }
//         else {
//             printk_color(light_red, "after hlt, interrupt not accept!\n\n");
//         }
//     }
//     // cpu_hlt();
//     // cpu_cli();

//     if (EFLAGS_IF_status()) {
//         printk_color(white, "interrupt accept!\n");
//     }
//     else {
//         printk_color(light_red, "interrupt not accept!\n");
//     }

//     printk_color(white, "\nEnd.\n");

//     while (1) {
//         ;
//     }

//     return;
// }

// #ifdef __cplusplus
// }
// #endif
