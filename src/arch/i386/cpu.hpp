
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).

// port.h for MRNIU/SimpleKernel.

// 执行CPU空操作
static inline void cpu_hlt(void){
  asm volatile("hlt");
}

// 开启中断
static inline void cpu_sti(void){
  asm volatile("sti");
}

// 关闭中断
static inline void cpu_cli(void){
  asm volatile("cli");
}
