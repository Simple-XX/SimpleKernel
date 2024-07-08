
/**
 * @file memory.cpp
 * @brief 内存相关
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

#include "load_elf.h"
#include "out_stream.hpp"

bool Memory::FlushDesc() {
  memory_map_ =
      LibMemoryMap(&desc_count_, &map_key_, &desc_size_, &desc_version_);
  if (memory_map_ == nullptr) {
    debug << L"LibMemoryMap failed: memory_map == nullptr" << OutStream::endl;
    return false;
  }
  return true;
}

Memory::Memory() {
  auto flush_desc_ret = FlushDesc();
  if (!flush_desc_ret) {
    debug << L"Memory::Memory() FlushDesc failed." << OutStream::endl;
    return;
  }
}

std::pair<uint64_t, size_t> Memory::GetMemory() const {
  uint64_t addr = 0;
  size_t size = 0;
  // 统计所有内存
  for (uint64_t i = 0; i < desc_count_; i++) {
    auto *desc = reinterpret_cast<EFI_MEMORY_DESCRIPTOR *>(
        (reinterpret_cast<uint8_t *>(memory_map_)) + i * desc_size_);
    size += desc->NumberOfPages * EFI_PAGE_SIZE;
  }

  return {addr, size};
}

void Memory::PrintInfo() {
  auto flush_desc_ret = FlushDesc();
  if (!flush_desc_ret) {
    debug << L"Memory::PrintInfo() FlushDesc failed." << OutStream::endl;
    return;
  }

  debug << L"memory_map_: " << OutStream::hex_X << memory_map_
        << L", desc_count_: " << desc_count_ << L", desc_size_: " << desc_size_
        << L", sizeof(EFI_MEMORY_DESCRIPTOR): " << sizeof(EFI_MEMORY_DESCRIPTOR)
        << L"." << OutStream::endl;

  debug << L"Type\t\t\t\tPages\tPhysicalStart\tVirtualStart\tAttribute"
        << OutStream::endl;

  for (uint64_t i = 0; i < desc_count_; i++) {
    auto *desc = reinterpret_cast<EFI_MEMORY_DESCRIPTOR *>(
        (reinterpret_cast<uint8_t *>(memory_map_)) + i * desc_size_);

    switch (desc->Type) {
      case EfiReservedMemoryType: {
        debug << L"iReservedMemoryType\t\t";
        break;
      }
      case EfiLoaderCode: {
        debug << L"EfiLoaderCode\t\t\t";
        break;
      }
      case EfiLoaderData: {
        debug << L"EfiLoaderData\t\t\t";
        break;
      }
      case EfiBootServicesCode: {
        debug << L"EfiBootServicesCode\t\t";
        break;
      }
      case EfiBootServicesData: {
        debug << L"EfiBootServicesData\t\t";
        break;
      }
      case EfiRuntimeServicesCode: {
        debug << L"EfiRuntimeServicesCode\t\t";
        break;
      }
      case EfiRuntimeServicesData: {
        debug << L"EfiRuntimeServicesData\t\t";
        break;
      }
      case EfiConventionalMemory: {
        debug << L"EfiConventionalMemory\t\t";
        break;
      }
      case EfiUnusableMemory: {
        debug << L"EfiUnusableMemory\t\t";
        break;
      }
      case EfiACPIReclaimMemory: {
        debug << L"EfiACPIReclaimMemory\t\t";
        break;
      }
      case EfiACPIMemoryNVS: {
        debug << L"EfiACPIMemoryNVS\t\t";
        break;
      }
      case EfiMemoryMappedIO: {
        debug << L"EfiMemoryMappedIO\t\t";
        break;
      }
      case EfiMemoryMappedIOPortSpace: {
        debug << L"EfiMemoryMappedIOPortSpace\t\t";
        break;
      }
      case EfiPalCode: {
        debug << L"EfiPalCode\t\t";
        break;
      }
      case EfiMaxMemoryType: {
        debug << L"EfiMaxMemoryType\t\t";
        break;
      }
      default: {
        debug << L"Unknown " << OutStream::hex_x << desc->Type << L"\t\t";
        break;
      }
    }

    debug << desc->NumberOfPages << L"\t" << OutStream::hex_X
          << desc->PhysicalStart << L"\t" << OutStream::hex_X
          << desc->VirtualStart << L"\t" << OutStream::hex_X << desc->Attribute
          << OutStream::endl;
  }
  debug << L"map_key: " << OutStream::hex_X << map_key_ << OutStream::endl;
}
