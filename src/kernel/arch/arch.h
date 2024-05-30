
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

#ifndef SIMPLEKERNEL_SRC_KERNEL_ARCH_ARCH_H_
#define SIMPLEKERNEL_SRC_KERNEL_ARCH_ARCH_H_

#include <cstdint>

struct BootInfo {
  static constexpr const uint32_t kMemoryMapMaxCount = 256;
  struct MemoryMap {
    enum { kTypeRam = 1, kTypeReserved, kTypeAcpi, kTypeNvs, kTypeUnUsable };
    uint64_t base_addr;
    uint64_t length;
    uint32_t type;
    uint32_t reserved;
  } memory_map[kMemoryMapMaxCount];
  uint32_t memory_map_count;

  struct FrameBuffer {
    uint64_t base;
    uint32_t size;
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    uint8_t bpp;
    uint8_t type;
    uint8_t reserved;
  } framebuffer;
};

uint32_t ArchInit(uint32_t argc, uint8_t *argv);

#endif /* SIMPLEKERNEL_SRC_KERNEL_ARCH_ARCH_H_ */
