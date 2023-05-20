
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

#include "efi.h"
#include "efilib.h"

//#include "uefi.h"

#include "kernel.h"

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
    kernel_main();
    return EFI_SUCCESS;
}

//int main(int, char**){
//    printf("1111\n");
//    return 0;
//}
