
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
#include "ostream.hpp"
#include "project_config.h"

uintptr_t ImageBase = 0;

extern "C" [[maybe_unused]] EFI_STATUS EFIAPI
efi_main(EFI_HANDLE _image_handle,
         [[maybe_unused]] EFI_SYSTEM_TABLE *_system_table) {
  EFI_STATUS status = EFI_SUCCESS;
  uint64_t kernel_addr = 0;
  try {
    // 输出 efi 信息
    EFI_LOADED_IMAGE *loaded_image = nullptr;
    status = LibLocateProtocol(&LoadedImageProtocol,
                               reinterpret_cast<void **>(&loaded_image));
    if (EFI_ERROR(status)) {
      debug << L"LibLocateProtocol: " << status << ostream::endl;
    }

    debug << L"Revision:        " << ostream::hex_X << loaded_image->Revision
          << ostream::endl;
    debug << L"ParentHandle:    " << ostream::hex_X
          << loaded_image->ParentHandle << ostream::endl;
    debug << L"SystemTable:     " << ostream::hex_X << loaded_image->SystemTable
          << ostream::endl;
    debug << L"DeviceHandle:    " << ostream::hex_X
          << loaded_image->DeviceHandle << ostream::endl;
    debug << L"FilePath:        " << ostream::hex_X << loaded_image->FilePath
          << ostream::endl;
    debug << L"Reserved:        " << ostream::hex_X << loaded_image->Reserved
          << ostream::endl;
    debug << L"LoadOptionsSize: " << ostream::hex_X
          << loaded_image->LoadOptionsSize << ostream::endl;
    debug << L"LoadOptions:     " << ostream::hex_X << loaded_image->LoadOptions
          << ostream::endl;
    debug << L"ImageBase:       " << ostream::hex_X << loaded_image->ImageBase
          << ostream::endl;
    debug << L"ImageSize:       " << ostream::hex_X << loaded_image->ImageSize
          << ostream::endl;
    debug << L"ImageCodeType:   " << ostream::hex_X
          << loaded_image->ImageCodeType << ostream::endl;
    debug << L"ImageDataType:   " << ostream::hex_X
          << loaded_image->ImageDataType << ostream::endl;
    debug << L"Unload:          " << ostream::hex_X << loaded_image->Unload
          << ostream::endl;

    // 初始化 Graphics
    auto graphics = Graphics();
    // 打印图形信息
    graphics.print_info();
    // 设置为 1920*1080
    graphics.set_mode();
    // 初始化 Memory
    auto memory = Memory();
    memory.print_info();
    // 加载内核
    auto elf = Elf(KERNEL_NAME);
    //    kernel_addr = elf.load_kernel_image();
    kernel_addr = elf.load();
  } catch (const std::exception &_e) {
    debug << L"Fatal Error: " << _e.what() << ostream::endl;
    return EFI_LOAD_ERROR;
  }
  debug << L"Set Kernel Entry Point to: [" << ostream::hex_X << kernel_addr
        << L"]" << ostream::endl;
  // 退出 boot service
  uint64_t desc_count = 0;
  EFI_MEMORY_DESCRIPTOR *memory_map = nullptr;
  uint64_t map_key = 0;
  uint64_t desc_size = 0;
  uint32_t desc_version = 0;
  memory_map = LibMemoryMap(&desc_count, &map_key, &desc_size, &desc_version);
  if (memory_map == nullptr) {
    debug << L"LibMemoryMap failed: memory_map == nullptr" << ostream::endl;
  }
  status = uefi_call_wrapper(gBS->ExitBootServices, 2, _image_handle, map_key);
  if (EFI_ERROR(status)) {
    debug << L"ExitBootServices failed, Memory Map has Changed " << status
          << ostream::endl;
  }

  auto kernel_entry = (void (*)())kernel_addr;
  kernel_entry();

  return EFI_SUCCESS;
}
