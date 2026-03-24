/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#include <cstddef>
#include <new>

#include "sk_stdlib.h"

void* operator new(size_t size) {
  if (size == 0) {
    size = 1;
  }
  return malloc(size);
}

void* operator new[](size_t size) {
  if (size == 0) {
    size = 1;
  }
  return malloc(size);
}

void* operator new(size_t size, size_t alignment) noexcept {
  if (size == 0) {
    size = 1;
  }

  // 确保对齐参数是 2 的幂
  if (alignment == 0 || (alignment & (alignment - 1)) != 0) {
    return nullptr;
  }

  // 如果对齐要求小于等于默认对齐，使用普通 malloc
  if (alignment <= alignof(std::max_align_t)) {
    return malloc(size);
  }

  return aligned_alloc(alignment, size);
}

void* operator new[](size_t size, size_t alignment) noexcept {
  if (size == 0) {
    size = 1;
  }

  // 确保对齐参数是 2 的幂
  if (alignment == 0 || (alignment & (alignment - 1)) != 0) {
    return nullptr;
  }

  // 如果对齐要求小于等于默认对齐，使用普通 malloc
  if (alignment <= alignof(std::max_align_t)) {
    return malloc(size);
  }

  return aligned_alloc(alignment, size);
}

void* operator new(size_t size, std::align_val_t alignment) {
  return operator new(size, static_cast<size_t>(alignment));
}

void* operator new[](size_t size, std::align_val_t alignment) {
  return operator new[](size, static_cast<size_t>(alignment));
}

auto operator delete(void* ptr) noexcept -> void {
  if (ptr != nullptr) {
    free(ptr);
  }
}

auto operator delete(void* ptr, size_t) noexcept -> void {
  if (ptr != nullptr) {
    free(ptr);
  }
}

auto operator delete[](void* ptr) noexcept -> void {
  if (ptr != nullptr) {
    free(ptr);
  }
}

auto operator delete[](void* ptr, size_t) noexcept -> void {
  if (ptr != nullptr) {
    free(ptr);
  }
}

auto operator delete(void* ptr, size_t, size_t) noexcept -> void {
  if (ptr != nullptr) {
    aligned_free(ptr);
  }
}

auto operator delete[](void* ptr, size_t, size_t) noexcept -> void {
  if (ptr != nullptr) {
    aligned_free(ptr);
  }
}

auto operator delete(void* ptr, std::align_val_t) noexcept -> void {
  if (ptr != nullptr) {
    aligned_free(ptr);
  }
}

auto operator delete[](void* ptr, std::align_val_t) noexcept -> void {
  if (ptr != nullptr) {
    aligned_free(ptr);
  }
}

auto operator delete(void* ptr, size_t, std::align_val_t) noexcept -> void {
  if (ptr != nullptr) {
    aligned_free(ptr);
  }
}

auto operator delete[](void* ptr, size_t, std::align_val_t) noexcept -> void {
  if (ptr != nullptr) {
    aligned_free(ptr);
  }
}
