
/**
 * @file graphics.cpp
 * @brief 图形相关
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

Graphics::Graphics() {
  auto status = LibLocateProtocol(&GraphicsOutputProtocol,
                                  reinterpret_cast<void **>(&gop));
  if (EFI_ERROR(status)) {
    debug << L"Graphics::Graphics() Could not locate GOP: " << status
          << OutStream::endl;
    return;
  }
  if (gop == nullptr) {
    debug << L"Graphics::Graphics() LibLocateProtocol(GraphicsOutputProtocol, "
             L"&gop) returned "
          << status << " but gop is nullptr" << OutStream::endl;
    return;
  }
}

void Graphics::set_mode(EFI_GRAPHICS_PIXEL_FORMAT _format, uint32_t _width,
                        uint32_t _height) const {
  EFI_STATUS status = EFI_SUCCESS;

  for (uint32_t i = 0; i < gop->Mode->MaxMode; i++) {
    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *mode_info = nullptr;
    uint64_t mode_info_size = 0;
    status = uefi_call_wrapper(gop->QueryMode, 4, gop, i, &mode_info_size,
                               &mode_info);
    if (EFI_ERROR(status)) {
      debug << L"Graphics::set_mode QueryMode failed: " << status
            << OutStream::endl;
      return;
    }

    if ((mode_info->PixelFormat == _format) &&
        (mode_info->HorizontalResolution == _width) &&
        (mode_info->VerticalResolution == _height)) {
      status = uefi_call_wrapper(gop->SetMode, 2, gop, i);
      if (EFI_ERROR(status)) {
        debug << L"Graphics::set_mode SetMode failed: " << status
              << OutStream::endl;
        return;
      }
    }
    status = uefi_call_wrapper(gBS->FreePool, 1, mode_info);
    if (EFI_ERROR(status)) {
      debug << L"Graphics::set_mode FreePool failed: " << status
            << OutStream::endl;
      return;
    }
  }

  debug << L"Current Mode: " << gop->Mode->Mode << L", Version: "
        << OutStream::hex_x << gop->Mode->Info->Version << L", Format: "
        << gop->Mode->Info->PixelFormat << L", Horizontal: "
        << gop->Mode->Info->HorizontalResolution << L", Vertical: "
        << gop->Mode->Info->VerticalResolution << L", ScanLine: "
        << gop->Mode->Info->PixelsPerScanLine << L", FrameBufferBase: "
        << OutStream::hex_X << gop->Mode->FrameBufferBase
        << L", FrameBufferSize: " << OutStream::hex_X
        << gop->Mode->FrameBufferSize << OutStream::endl;
}

auto Graphics::get_framebuffer() const -> std::pair<uint64_t, uint32_t> {
  return {gop->Mode->FrameBufferBase, gop->Mode->FrameBufferSize};
}

void Graphics::print_info() const {
  debug << L"Current Mode: " << gop->Mode->Mode << L", Version: "
        << OutStream::hex_x << gop->Mode->Info->Version << L", Format: "
        << gop->Mode->Info->PixelFormat << L", Horizontal: "
        << gop->Mode->Info->HorizontalResolution << L", Vertical: "
        << gop->Mode->Info->VerticalResolution << L", ScanLine: "
        << gop->Mode->Info->PixelsPerScanLine << L", FrameBufferBase: "
        << OutStream::hex_X << gop->Mode->FrameBufferBase
        << L", FrameBufferSize: " << OutStream::hex_X
        << gop->Mode->FrameBufferSize << OutStream::endl;

  for (uint32_t i = 0; i < gop->Mode->MaxMode; i++) {
    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *mode_info = nullptr;
    uint64_t mode_info_size = 0;
    auto status = uefi_call_wrapper(gop->QueryMode, 4, gop, i, &mode_info_size,
                                    &mode_info);
    if (EFI_ERROR(status)) {
      debug << L"Graphics::print_info() QueryMode failed: " << status
            << OutStream::endl;
      return;
    }

    debug << L"Mode: " << i << L", Version: " << OutStream::hex_x
          << gop->Mode->Info->Version << L", Format: "
          << gop->Mode->Info->PixelFormat << L", Horizontal: "
          << gop->Mode->Info->HorizontalResolution << L", Vertical: "
          << gop->Mode->Info->VerticalResolution << L", ScanLine: "
          << gop->Mode->Info->PixelsPerScanLine << OutStream::endl;

    status = uefi_call_wrapper(gBS->FreePool, 1, mode_info);
    if (EFI_ERROR(status)) {
      debug << L"Graphics::print_info() FreePool failed: " << status
            << OutStream::endl;
      return;
    }
  }
}
