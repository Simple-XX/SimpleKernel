
/**
 * @file kernel.h
 * @brief 内核头文件
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

#ifndef SIMPLEKERNEL_SRC_KERNEL_INCLUDE_KERNEL_H_
#define SIMPLEKERNEL_SRC_KERNEL_INCLUDE_KERNEL_H_

#include <cstddef>
#include <cstdint>

#include "singleton.hpp"

struct BasicInfo {
  /// 最大内存表数量
  static constexpr const uint32_t kMemoryMapMaxCount = 256;
  struct MemoryMap {
    /// 内存类型
    enum {
      /// 可用
      kTypeRam = 1,
      /// 保留
      kTypeReserved,
      /// ACPI
      kTypeAcpi,
      /// NV
      kTypeNvs,
      /// 不可用
      kTypeUnUsable
    };
    /// 地址
    uint64_t base_addr;
    /// 长度
    uint64_t length;
    /// 类型
    uint32_t type;
    /// 保留
    uint32_t reserved;
  } memory_map[kMemoryMapMaxCount];
  /// 内存表数量
  uint32_t memory_map_count;

  /// 显示缓冲区
  struct FrameBuffer {
    /// 地址
    uint64_t base;
    /// 长度
    uint32_t size;
    /// 屏幕宽像素个数
    uint32_t width;
    /// 屏幕高像素个数
    uint32_t height;
    /// 每行字节数
    uint32_t pitch;
    /// 每像素字节数
    uint8_t bpp;
    /// 显示类型
    uint8_t type;
    /// 保留
    uint8_t reserved;
  } framebuffer;

  /// elf 地址
  uint64_t elf_addr;
  /// elf 大小
  size_t elf_size;
};

/**
 * @brief 负责 crtbegin 的工作
 * @param  argc 由 bootloader 传递的参数，在不同架构有不同的含义
 * @param  argv 由 bootloader 传递的参数，在不同架构有不同的含义
 */
extern "C" [[maybe_unused]] [[noreturn]] void _start(uint32_t argc,
                                                     uint8_t* argv);

/**
 * @brief 内核入口
 * @param argc 参数个数
 * @param argv 参数指针
 * @return uint32_t 正常返回 0
 */
uint32_t main(uint32_t argc, uint8_t* argv);

/// 保存内核基本信息
[[maybe_unused]] static Singleton<BasicInfo> kBasicInfo;

#endif /* SIMPLEKERNEL_SRC_KERNEL_INCLUDE_KERNEL_H_ */
