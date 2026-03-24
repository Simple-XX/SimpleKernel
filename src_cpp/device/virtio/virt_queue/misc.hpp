/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#pragma once

#include <cstddef>
#include <cstdint>

namespace virtio {

/**
 * @brief 将值向上对齐到指定边界
 *
 * @param value 要对齐的值（字节数）
 * @param align 对齐边界（字节数，必须为 2 的幂）
 * @return 对齐后的值
 * @see virtio-v1.2#2.7.5 The Virtqueue Descriptor Table
 */
[[nodiscard]] constexpr auto AlignUp(size_t value, size_t align) -> size_t {
  return (value + align - 1) & ~(align - 1);
}

/**
 * @brief 检查值是否为 2 的幂
 *
 * @param value 要检查的值
 * @return true 表示 value 是 2 的幂，false 表示不是
 * @note value 为 0 时返回 false
 * @see virtio-v1.2#2.6 Split Virtqueues
 */
[[nodiscard]] constexpr auto IsPowerOfTwo(size_t value) -> bool {
  return value != 0 && (value & (value - 1)) == 0;
}

/**
 * @brief Scatter-Gather IO 物理内存向量
 *
 * 描述一段用于 DMA 传输的物理内存区域。
 *
 * @see virtio-v1.2#2.7.5 The Virtqueue Descriptor Table
 */
struct IoVec {
  /// 物理地址（DMA 地址）
  uintptr_t phys_addr;
  /// 长度（字节）
  size_t len;
};

}  // namespace virtio
