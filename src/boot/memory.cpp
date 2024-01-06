
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

#include <stdexcept>

#include "load_elf.h"
#include "ostream.hpp"

void Memory::flush_desc() {
  memory_map = LibMemoryMap(&desc_count, &map_key, &desc_size, &desc_version);
  if (memory_map == nullptr) {
    debug << L"LibMemoryMap failed: memory_map == nullptr" << ostream::endl;
    throw std::runtime_error("memory_map == nullptr");
  }
}

Memory::Memory() { flush_desc(); }

void Memory::print_info() {
  flush_desc();
  debug << L"Type\t\t\t\tPages\tPhysicalStart\tVirtualStart\tAttribute"
        << ostream::endl;
  for (uint64_t i = 0; i < desc_count; i++) {
    auto *MMap = reinterpret_cast<EFI_MEMORY_DESCRIPTOR *>(
        (reinterpret_cast<uint8_t *>(memory_map)) + i * desc_size);

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
        debug << L"Unknown " << ostream::hex_x << MMap->Type << L"\t\t";
        break;
      }
    }

    debug << MMap->NumberOfPages << L"\t" << ostream::hex_X
          << MMap->PhysicalStart << L"\t" << ostream::hex_X
          << MMap->VirtualStart << L"\t" << ostream::hex_X << MMap->Attribute
          << ostream::endl;
  }
  debug << L"map_key: " << ostream::hex_X << map_key << ostream::endl;
}
