/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

#include "expected.hpp"

/**
 * @brief 虚拟地址到物理地址转换回调类型
 * @param virt 需要转换的虚拟地址
 * @return 对应的物理地址
 */
using VirtToPhysFunc = auto (*)(uintptr_t virt) -> uintptr_t;

/**
 * @brief 恒等映射：物理地址 == 虚拟地址（早期启动 / 无 MMU 时的默认实现）
 * @param virt 虚拟地址
 * @return 与输入相同的物理地址
 */
[[nodiscard]] inline auto IdentityVirtToPhys(uintptr_t virt) -> uintptr_t {
  return virt;
}

/**
 * @brief DMA 可访问内存区域的非拥有描述符
 *
 * 将虚拟地址、物理（总线）地址和大小封装为单一值类型。
 * 不拥有内存所有权——生命周期由分配器（如 IoBuffer）管理。
 *
 * @note 类 POD 类型，可平凡拷贝，可安全按值或 const 引用传递。
 */
struct DmaRegion {
  /// 虚拟（CPU 可访问）基地址
  void* virt{nullptr};
  /// 物理（总线/DMA）基地址
  uintptr_t phys{0};
  /// 区域大小（字节）
  size_t size{0};

  /// 检查区域是否有效（非空指针且大小非零）
  [[nodiscard]] auto IsValid() const -> bool {
    return virt != nullptr && size > 0;
  }

  /// @brief 获取虚拟基地址的类型化指针
  /// @return uint8_t* 缓冲区数据指针
  [[nodiscard]] auto Data() const -> uint8_t* {
    return static_cast<uint8_t*>(virt);
  }

  /**
   * @brief 在指定偏移处创建子区域
   *
   * @param offset 相对于当前区域起始处的字节偏移
   * @param len 子区域的字节长度
   * @return 成功返回子区域，越界则返回错误
   *
   * @pre offset + len <= size
   * @post 返回的区域与当前区域共享同一底层内存
   */
  [[nodiscard]] auto SubRegion(size_t offset, size_t len) const
      -> Expected<DmaRegion> {
    if (offset > size || len > size - offset) {
      return std::unexpected(Error{ErrorCode::kInvalidArgument});
    }
    return DmaRegion{
        .virt = static_cast<uint8_t*>(virt) + offset,
        .phys = phys + offset,
        .size = len,
    };
  }
};

/**
 * @brief 动态分配、对齐 IO 缓冲区的 RAII 封装
 *
 * @pre  无
 * @post 缓冲区内存已正确分配并对齐，析构时自动释放
 */
class IoBuffer {
 public:
  /// IO 缓冲区的默认对齐大小（如页大小）
  static constexpr size_t kDefaultAlignment = 4096;

  /**
   * @brief 获取缓冲区数据与大小 (只读)
   * @return std::span<const uint8_t> 缓冲区数据的只读视图
   * @pre None
   * @post 返回指向缓冲区数据的常量 span
   */
  [[nodiscard]] auto GetBuffer() const -> std::span<const uint8_t>;

  /**
   * @brief 获取缓冲区数据与大小
   * @return std::span<uint8_t> 缓冲区数据的可变视图
   * @pre None
   * @post 返回指向缓冲区数据的 span
   */
  [[nodiscard]] auto GetBuffer() -> std::span<uint8_t>;

  /**
   * @brief 检查缓冲区是否有效
   * @return bool 有效则返回 true
   * @pre None
   * @post 返回缓冲区是否已分配且有效的状态
   */
  [[nodiscard]] auto IsValid() const -> bool;

  /**
   * @brief 创建此缓冲区的 DmaRegion 视图
   *
   * @param v2p 地址转换函数（默认：恒等映射）
   * @return 描述此缓冲区内存的 DmaRegion
   *
   * @pre IsValid() == true
   * @post 返回的 DmaRegion 不拥有内存所有权
   */
  [[nodiscard]] auto ToDmaRegion(VirtToPhysFunc v2p = IdentityVirtToPhys) const
      -> DmaRegion;

  /// @name 构造/析构函数
  /// @{
  IoBuffer() = default;

  /**
   * @brief 构造函数
   * @param  size 缓冲区大小
   * @param  alignment 对齐要求
   * @pre size > 0, alignment 必须是 2 的幂
   * @post 缓冲区内存已正确分配并对齐
   */
  explicit IoBuffer(size_t size, size_t alignment = kDefaultAlignment);

  ~IoBuffer();

  IoBuffer(const IoBuffer&) = delete;
  auto operator=(const IoBuffer&) -> IoBuffer& = delete;

  IoBuffer(IoBuffer&& other);
  auto operator=(IoBuffer&& other) noexcept -> IoBuffer&;
  /// @}

 private:
  /// 缓冲区数据指针
  uint8_t* data_{nullptr};
  /// 缓冲区大小
  size_t size_{0};
};
