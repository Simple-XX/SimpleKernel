
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

#include "kernel/arch/arch.h"
#include "load_elf.h"
#include "out_stream.hpp"
#include "project_config.h"

// efi 使用的全局变量
uintptr_t ImageBase = 0;

extern "C" [[maybe_unused]] EFI_STATUS EFIAPI
efi_main(EFI_HANDLE image_handle, EFI_SYSTEM_TABLE *system_table) {
  EFI_STATUS status = EFI_SUCCESS;
  uint64_t kernel_addr = 0;

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

  // 初始化 Graphics
  auto graphics = Graphics();
  // 打印图形信息
  graphics.PrintInfo();
  // 设置为 kDefaultWidth*kDefaultHeight
  graphics.SetMode();
  // 初始化 Memory
  auto memory = Memory();
  memory.PrintInfo();
  // 加载内核
  auto elf = Elf(KERNEL_NAME);
  kernel_addr = elf.Load();
  if (kernel_addr == 0) {
    debug << L"Failed to load kernel" << OutStream::endl;
    return EFI_LOAD_ERROR;
  }

  debug << L"Set Kernel Entry Point to: [" << OutStream::hex_X << kernel_addr
        << L"]." << OutStream::endl;
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

  // 获取内存信息

  BootInfo boot_info = {};

  boot_info.memory_map_count = memory.GetMemoryMap(boot_info.memory_map);

  // 获取 framebuffer 信息
  auto [framebuffer_base, framebuffer_size, framebuffer_width,
        framebuffer_height, framebuffer_pixel_per_line] =
      graphics.GetFrameBuffer();
  boot_info.framebuffer.base = framebuffer_base;
  boot_info.framebuffer.size = framebuffer_size;
  boot_info.framebuffer.width = framebuffer_width;
  boot_info.framebuffer.height = framebuffer_height;
  boot_info.framebuffer.pitch = framebuffer_pixel_per_line * sizeof(uint32_t);

  uint8_t *argv[] = {
      reinterpret_cast<uint8_t *>(&boot_info),
      nullptr,
  };

  auto kernel_entry = (void (*)(uint32_t, uint8_t **))kernel_addr;
  kernel_entry(1, static_cast<uint8_t **>(argv));

  // 不会执行到这里
  return EFI_SUCCESS;
}
