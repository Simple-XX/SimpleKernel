/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#include "io_buffer.hpp"

#include <cassert>

#include "kernel_log.hpp"
#include "sk_stdlib.h"

IoBuffer::IoBuffer(size_t size, size_t alignment) {
  assert(size > 0 && "IoBuffer size must be greater than 0");
  assert((alignment & (alignment - 1)) == 0 &&
         "IoBuffer alignment must be a power of 2");

  auto* data = static_cast<uint8_t*>(aligned_alloc(alignment, size));
  assert(data != nullptr && "IoBuffer aligned_alloc failed");
  data_ = data;
  size_ = size;
}

IoBuffer::~IoBuffer() {
  assert((data_ == nullptr) == (size_ == 0) &&
         "IoBuffer invariant violated: data_ and size_ must be consistent");
  aligned_free(data_);
}

IoBuffer::IoBuffer(IoBuffer&& other) : data_(other.data_), size_(other.size_) {
  other.data_ = nullptr;
  other.size_ = 0;
}

auto IoBuffer::operator=(IoBuffer&& other) noexcept -> IoBuffer& {
  assert(this != &other && "Self-move assignment is not allowed");

  if (data_ != nullptr) {
    aligned_free(data_);
  }
  data_ = other.data_;
  size_ = other.size_;
  other.data_ = nullptr;
  other.size_ = 0;

  return *this;
}

auto IoBuffer::GetBuffer() const -> std::span<const uint8_t> {
  return {data_, size_};
}

auto IoBuffer::GetBuffer() -> std::span<uint8_t> { return {data_, size_}; }

auto IoBuffer::IsValid() const -> bool { return data_ != nullptr; }

auto IoBuffer::ToDmaRegion(VirtToPhysFunc v2p) const -> DmaRegion {
  return DmaRegion{
      .virt = data_,
      .phys = v2p(reinterpret_cast<uintptr_t>(data_)),
      .size = size_,
  };
}
