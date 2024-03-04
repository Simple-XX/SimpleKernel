
/**
 * @file cpu.hpp
 * @brief x86_64 cpu 相关定义
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2024-03-05
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2024-03-05<td>Zone.N (Zone.Niuzh@hotmail.com)<td>创建文件
 * </table>
 */

#ifndef SIMPLEKERNEL_SRC_KERNEL_ARCH_X86_64_CPU_HPP
#define SIMPLEKERNEL_SRC_KERNEL_ARCH_X86_64_CPU_HPP

#include "cstdint"

class CPU {
 public:
  /**
   * @brief  读一个字节
   * @param  _port           要读的端口
   * @return uint8_t         读取到的数据
   */
  static inline uint8_t inb(const uint32_t _port) {
    uint8_t data;
    __asm__ __volatile__("inb %1, %0" : "=a"(data) : "dN"(_port));
    return data;
  }

  /**
   * @brief  读一个字
   * @param  _port           要读的端口
   * @return uint16_t        读取到的数据
   */
  static inline uint16_t inw(const uint32_t _port) {
    uint16_t data;
    __asm__ __volatile__("inw %1, %0" : "=a"(data) : "dN"(_port));
    return data;
  }

  /**
   * @brief  读一个双字
   * @param  _port           要读的端口
   * @return uint32_t        读取到的数据
   */
  static inline uint32_t inl(const uint32_t _port) {
    uint32_t data;
    __asm__ __volatile__("inl %1, %0" : "=a"(data) : "dN"(_port));
    return data;
  }

  /**
   * @brief  写一个字节
   * @param  _port           要写的端口
   * @param  _data           要写的数据
   */
  static inline void outb(const uint32_t _port, const uint8_t _data) {
    __asm__ __volatile__("outb %1, %0" : : "dN"(_port), "a"(_data));
  }

  /**
   * @brief  写一个字
   * @param  _port           要写的端口
   * @param  _data           要写的数据
   */
  static inline void outw(const uint32_t _port, const uint16_t _data) {
    __asm__ __volatile__("outw %1, %0" : : "dN"(_port), "a"(_data));
  }

  /**
   * @brief  写一个双字
   * @param  _port           要写的端口
   * @param  _data           要写的数据
   */
  static inline void outl(const uint32_t _port, const uint32_t _data) {
    __asm__ __volatile__("outl %1, %0" : : "dN"(_port), "a"(_data));
  }
};

#endif  // SIMPLEKERNEL_SRC_KERNEL_ARCH_X86_64_CPU_HPP
