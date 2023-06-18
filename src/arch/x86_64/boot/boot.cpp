
/**
 * @file boot.cpp
 * @brief uefi 引导
 * @author kehroche (kehroche@gmail.com)
 * @version 1.0
 * @date 2023-05-20
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2023-05-20<td>kehroche<td>创建文件
 * </table>
 */

#include "cassert"
#include "kernel.h"

#if USE_GNU_UEFI == 1
#    include "efi.h"
#    include "efilib.h"
#else
#    include "uefi.h"
#endif

#if USE_GNU_UEFI == 1

/**
 * efi_main - The entry point for the EFI application
 * @image: firmware-allocated handle that identifies the image
 * @SystemTable: EFI system table
 */
extern "C" EFI_STATUS
efi_main(EFI_HANDLE image, EFI_SYSTEM_TABLE* systemTable) {
    uefi_call_wrapper(InitializeLib, 2, image, systemTable);
    EFI_STATUS status = uefi_call_wrapper(systemTable->ConOut->ClearScreen, 1,
                                          systemTable->ConOut);

    status            = uefi_call_wrapper(systemTable->ConOut->OutputString, 2,
                                          systemTable->ConOut, L"Hello UEFI!\n");
    assert(status != EFI_SUCCESS);

    kernel_main();

    return EFI_SUCCESS;
}

#else

/**
 * Dump memory at given address, should accept 0x prefixes from the command line
 */
extern "C" int main(int argc, char** argv) {
    efi_physical_address_t address
      = (argc < 2 ? (efi_physical_address_t)IM
                  : (efi_physical_address_t)atol(argv[1]));

    printf("%4D", address);

    kernel_main();

    return EFI_SUCCESS;
}

#endif
