
/**
 * @file arch.h
 * @brief arch 头文件
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2023-07-15
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2023-07-15<td>Zone.N (Zone.Niuzh@hotmail.com)<td>创建文件
 * </table>
 */

#ifndef SIMPLEKERNEL_SRC_KERNEL_ARCH_ARCH_H
#define SIMPLEKERNEL_SRC_KERNEL_ARCH_ARCH_H

#include "cstdint"

struct boot_info_t {
  struct framebuffer_t {
    uint64_t base;
    uint32_t size;
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    uint8_t bpp;
    uint8_t type;
    uint8_t reserved;
  } framebuffer;
  struct memory_map_t {
    uint64_t base;
    uint64_t length;
    uint32_t type;
    uint32_t reserved;
  } memory_map;
};

int32_t arch_init(uint32_t _argc, uint8_t **_argv);

#endif /* SIMPLEKERNEL_SRC_KERNEL_ARCH_ARCH_H */
