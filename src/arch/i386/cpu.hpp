
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).

// port.h for MRNIU/SimpleKernel.

#ifndef _CPU_HPP_
#define _CPU_HPP_

#include "stdint.h"
#include "stdbool.h"

#define FL_CF           0x00000001  // Carry Flag
#define FL_PF           0x00000004  // Parity Flag
#define FL_AF           0x00000010  // Auxiliary carry Flag
#define FL_ZF           0x00000040  // Zero Flag
#define FL_SF           0x00000080  // Sign Flag
#define FL_TF           0x00000100  // Trap Flag
#define FL_IF           0x00000200  // Interrupt Flag
#define FL_DF           0x00000400  // Direction Flag
#define FL_OF           0x00000800  // Overflow Flag
#define FL_IOPL_MASK    0x00003000  // I/O Privilege Level bitmask
#define FL_IOPL_0       0x00000000  // IOPL == 0
#define FL_IOPL_1       0x00001000  // IOPL == 1
#define FL_IOPL_2       0x00002000  // IOPL == 2
#define FL_IOPL_3       0x00003000  // IOPL == 3
#define FL_NT           0x00004000  // Nested Task
#define FL_RF           0x00010000  // Resume Flag
#define FL_VM           0x00020000  // Virtual 8086 mode
#define FL_AC           0x00040000  // Alignment Check
#define FL_VIF          0x00080000  // Virtual Interrupt Flag
#define FL_VIP          0x00100000  // Virtual Interrupt Pending
#define FL_ID           0x00200000  // ID flag

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
  asm volatile("cli":::"memory");
}

// 读取 EFLAGS
static inline uint32_t read_eflags(void){
  uint32_t eflags;
  asm volatile("pushf;pop %0"
               :"=r"(eflags));
  return eflags;
}

static inline bool FL_ID_status(void){
  uint32_t eflags= read_eflags();
  return (eflags&FL_ID);
}

static inline bool FL_VIP_status(void){
  uint32_t eflags= read_eflags();
  return (eflags&FL_VIP);
}

static inline bool FL_VIF_status(void){
  uint32_t eflags= read_eflags();
  return (eflags&FL_VIF);
}

static inline bool FL_AC_status(void){
  uint32_t eflags= read_eflags();
  return (eflags&FL_AC);
}

static inline bool FL_VM_status(void){
  uint32_t eflags= read_eflags();
  return (eflags&FL_VM);
}

static inline bool FL_RF_status(void){
  uint32_t eflags= read_eflags();
  return (eflags&FL_RF);
}

static inline bool FL_NT_status(void){
  uint32_t eflags= read_eflags();
  return (eflags&FL_NT);
}

static inline uint32_t get_IOPL(void){
  uint32_t eflags= read_eflags();
  uint32_t level=0;
  if(eflags&FL_IOPL_0)
    level=0;
  else if(eflags&FL_IOPL_1)
    level=1;
  else if(eflags&FL_IOPL_2)
    level=2;
  else if(eflags&FL_IOPL_3)
    level=3;
  else return 2333;
  return level;
}

static inline bool FL_OF_status(void){
  uint32_t eflags= read_eflags();
  return (eflags&FL_OF);
}

static inline bool FL_DF_status(void){
  uint32_t eflags= read_eflags();
  return (eflags&FL_DF);
}

static inline bool FL_IF_status(void){
  uint32_t eflags= read_eflags();
  return (eflags&FL_IF);
}

static inline bool FL_TF_status(void){
  uint32_t eflags= read_eflags();
  return (eflags&FL_TF);
}

static inline bool FL_SF_status(void){
  uint32_t eflags= read_eflags();
  return (eflags&FL_SF);
}

static inline bool FL_ZF_status(void){
  uint32_t eflags= read_eflags();
  return (eflags&FL_ZF);
}

static inline bool FL_AF_status(void){
  uint32_t eflags= read_eflags();
  return (eflags&FL_AF);
}

static inline bool FL_PF_status(void){
  uint32_t eflags= read_eflags();
  return (eflags&FL_PF);
}

static inline bool FL_CF_status(void){
  uint32_t eflags= read_eflags();
  return (eflags&FL_CF);
}


#endif
