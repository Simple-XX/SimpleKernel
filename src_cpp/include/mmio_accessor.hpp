/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#pragma once

#include <cstddef>
#include <cstdint>

/**
 * @brief 通用 MMIO 寄存器访问器
 */
struct MmioAccessor {
  /// @brief Read from MMIO register
  /// @tparam T Register value type
  /// @param offset Byte offset from base address
  /// @return Register value
  template <typename T>
  [[nodiscard]] auto Read(size_t offset) const -> T {
    return *reinterpret_cast<volatile T*>(base + offset);
  }

  /// @brief Write to MMIO register
  /// @tparam T Register value type
  /// @param offset Byte offset from base address
  /// @param val Value to write
  template <typename T>
  auto Write(size_t offset, T val) const -> void {
    *reinterpret_cast<volatile T*>(base + offset) = val;
  }

  uint64_t base{0};

  /// @name 构造/析构函数
  /// @{
  explicit MmioAccessor(uint64_t base_addr = 0) : base(base_addr) {}
  MmioAccessor(const MmioAccessor&) = default;
  MmioAccessor(MmioAccessor&&) = default;
  auto operator=(const MmioAccessor&) -> MmioAccessor& = default;
  auto operator=(MmioAccessor&&) -> MmioAccessor& = default;
  ~MmioAccessor() = default;
  /// @}
};
