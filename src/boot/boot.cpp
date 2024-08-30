
/**
 * @file boot.cpp
 * @brief boot cpp
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
#include "project_config.h"

// efi 使用的全局变量
#if defined(__x86_64__) || defined(__aarch64__)
uintptr_t ImageBase = 0;
#endif

extern "C" [[maybe_unused]] EFI_STATUS EFIAPI
efi_main(EFI_HANDLE image_handle, EFI_SYSTEM_TABLE *system_table) {
  EFI_STATUS status = EFI_SUCCESS;

  InitializeLib(image_handle, system_table);

  // 输出 efi 信息
  EFI_LOADED_IMAGE *loaded_image = nullptr;
  /// @bug 在 aarch64 下会出错
  status = LibLocateProtocol(&LoadedImageProtocol,
                             reinterpret_cast<void **>(&loaded_image));
  if (EFI_ERROR(status)) {
    debug << L"LibLocateProtocol: " << status << OutStream::endl;
    return status;
  }

  debug << L"Revision:        " << OutStream::hex_X << loaded_image->Revision
        << OutStream::endl;
  debug << L"ParentHandle:    " << OutStream::hex_X
        << loaded_image->ParentHandle << OutStream::endl;
  debug << L"SystemTable:     " << OutStream::hex_X << loaded_image->SystemTable
        << OutStream::endl;
  debug << L"DeviceHandle:    " << OutStream::hex_X
        << loaded_image->DeviceHandle << OutStream::endl;
  debug << L"FilePath:        " << OutStream::hex_X << loaded_image->FilePath
        << OutStream::endl;
  debug << L"Reserved:        " << OutStream::hex_X << loaded_image->Reserved
        << OutStream::endl;
  debug << L"LoadOptionsSize: " << OutStream::hex_X
        << loaded_image->LoadOptionsSize << OutStream::endl;
  debug << L"LoadOptions:     " << OutStream::hex_X << loaded_image->LoadOptions
        << OutStream::endl;
  debug << L"ImageBase:       " << OutStream::hex_X << loaded_image->ImageBase
        << OutStream::endl;
  debug << L"ImageSize:       " << OutStream::hex_X << loaded_image->ImageSize
        << OutStream::endl;
  debug << L"ImageCodeType:   " << OutStream::hex_X
        << loaded_image->ImageCodeType << OutStream::endl;
  debug << L"ImageDataType:   " << OutStream::hex_X
        << loaded_image->ImageDataType << OutStream::endl;
  debug << L"Unload:          " << OutStream::hex_X << loaded_image->Unload
        << OutStream::endl;

  // 初始化 Memory
  auto memory = Memory();
  // 加载内核
  auto elf = Elf(KERNEL_NAME);
  auto [kernel_addr, elf_info] = elf.Load();
  if (kernel_addr == 0) {
    debug << L"Failed to load kernel" << OutStream::endl;
    return EFI_LOAD_ERROR;
  }

  // 输出内存映射 Memory
  memory.PrintInfo();

  debug << L"Set Kernel Entry Point to: [" << OutStream::hex_X << kernel_addr
        << L"]." << OutStream::endl;
  debug << L"Elf addr: [" << OutStream::hex_X << elf_info.first << L"]."
        << OutStream::endl;
  debug << L"Elf size: [" << OutStream::hex_X << elf_info.second << L"]."
        << OutStream::endl;
  // 退出 boot service
  uint64_t desc_count = 0;
  EFI_MEMORY_DESCRIPTOR *memory_map = nullptr;
  uint64_t map_key = 0;
  uint64_t desc_size = 0;
  uint32_t desc_version = 0;
  memory_map = LibMemoryMap(&desc_count, &map_key, &desc_size, &desc_version);
  if (memory_map == nullptr) {
    debug << L"LibMemoryMap failed: memory_map == nullptr" << OutStream::endl;
  }

  // 退出后不能使用输出相关方法
  status = uefi_call_wrapper(gBS->ExitBootServices, 2, image_handle, map_key);
  if (EFI_ERROR(status)) {
    debug << L"ExitBootServices failed, Memory Map has Changed " << status
          << OutStream::endl;
    return status;
  }

  /// 基础信息
  /// @note 结构与 basic_info.hpp 同步
  struct {
    uint64_t physical_memory_addr;
    size_t physical_memory_size;
    uint64_t kernel_addr;
    size_t kernel_size;
    uint64_t elf_addr;
    size_t elf_size;
  } basic_info;
  std::fill_n(reinterpret_cast<uint8_t *>(&basic_info), sizeof(basic_info), 0);

  // 获取物理内存信息
  auto [physical_memory_addr, physical_memory_size] = memory.GetMemory();

  // 填充信息
  basic_info.physical_memory_addr = physical_memory_addr;
  basic_info.physical_memory_size = physical_memory_size;
  basic_info.elf_addr = elf_info.first;
  basic_info.elf_size = elf_info.second;

  auto kernel_entry = (void (*)(uint32_t, uint8_t *))kernel_addr;
  kernel_entry(1, reinterpret_cast<uint8_t *>(&basic_info));

  // 不会执行到这里
  return EFI_SUCCESS;
}
