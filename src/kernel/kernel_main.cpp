
/**
 * @file kernel_main.cpp
 * @brief 内核主要逻辑
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2021-09-18
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2021-09-18<td>digmouse233<td>迁移到 doxygen
 * </table>
 */

#include "common.h"
#include "iostream"
#include "kernel.h"
#include "stdio.h"

#include "assert.h"

#include "efi.h"

#include "efilib.h"
EFI_SYSTEM_TABLE* systemTable111;

/**
 * efi_main - The entry point for the EFI application
 * @image: firmware-allocated handle that identifies the image
 * @SystemTable: EFI system table
 */
extern "C" EFI_STATUS
efi_main(EFI_HANDLE image, EFI_SYSTEM_TABLE* systemTable) {
    systemTable111 = systemTable;
    uefi_call_wrapper(InitializeLib, 2, image, systemTable);
    EFI_STATUS status = uefi_call_wrapper(systemTable->ConOut->ClearScreen, 1,
                                          systemTable->ConOut);

    status            = uefi_call_wrapper(systemTable->ConOut->OutputString, 2,
                                          systemTable->ConOut, L"Hello UEFI!\n");
    kernel_main();
    return EFI_SUCCESS;
}

/**
 * @brief 内核主要逻辑
 * @note 这个函数不会返回
 */
void kernel_main(void) {
    EFI_STATUS status
      = uefi_call_wrapper(systemTable111->ConOut->OutputString, 2,
                          systemTable111->ConOut, L"Hello UEFI111!\n");

    // 显示基本信息
    show_info();
    // 进入死循环
    while (1) {
        ;
    }
    return;
}

/**
 * @brief 输出系统信息
 */
void show_info(void) {
    // // 内核实际大小
    // auto kernel_size = COMMON::KERNEL_END_ADDR - COMMON::KERNEL_START_ADDR;
    // // 内核实际占用页数
    // auto kernel_pages
    //   = (COMMON::ALIGN(COMMON::KERNEL_END_ADDR, COMMON::PAGE_SIZE)
    //      - COMMON::ALIGN(COMMON::KERNEL_START_ADDR, COMMON::PAGE_SIZE))
    //   / COMMON::PAGE_SIZE;
    // info("Kernel start: 0x%p, end 0x%p, size: 0x%X bytes, 0x%X pages.\n",
    //      COMMON::KERNEL_START_ADDR, COMMON::KERNEL_END_ADDR, kernel_size,
    //      kernel_pages);
    // info("Kernel start4k: 0x%p, end4k: 0x%p.\n",
    //      COMMON::ALIGN(COMMON::KERNEL_START_ADDR, 4 * COMMON::KB),
    //      COMMON::ALIGN(COMMON::KERNEL_END_ADDR, 4 * COMMON::KB));
    // std::cout << "Simple Kernel." << std::endl;
    return;
}
