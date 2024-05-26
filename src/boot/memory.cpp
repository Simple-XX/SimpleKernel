
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

bool Memory::flush_desc() {
  memory_map_ =
      LibMemoryMap(&desc_count_, &map_key_, &desc_size_, &desc_version_);
  if (memory_map_ == nullptr) {
    debug << L"LibMemoryMap failed: memory_map == nullptr" << OutStream::endl;
    return false;
  }
  return true;
}

Memory::Memory() {
  auto flush_desc_ret = flush_desc();
  if (!flush_desc_ret) {
    debug << L"Memory::Memory() flush_desc failed." << OutStream::endl;
    return;
  }
}

void Memory::print_info() {
  auto flush_desc_ret = flush_desc();
  if (!flush_desc_ret) {
    debug << L"Memory::print_info() flush_desc failed." << OutStream::endl;
    return;
  }

  debug << L"Type\t\t\t\tPages\tPhysicalStart\tVirtualStart\tAttribute"
        << OutStream::endl;
  for (uint64_t i = 0; i < desc_count_; i++) {
    auto *MMap = reinterpret_cast<EFI_MEMORY_DESCRIPTOR *>(
        (reinterpret_cast<uint8_t *>(memory_map_)) + i * desc_size_);

    switch (MMap->Type) {
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
        debug << L"Unknown " << OutStream::hex_x << MMap->Type << L"\t\t";
        break;
      }
    }

    debug << MMap->NumberOfPages << L"\t" << OutStream::hex_X
          << MMap->PhysicalStart << L"\t" << OutStream::hex_X
          << MMap->VirtualStart << L"\t" << OutStream::hex_X << MMap->Attribute
          << OutStream::endl;
  }
  debug << L"map_key: " << OutStream::hex_X << map_key_ << OutStream::endl;
}
