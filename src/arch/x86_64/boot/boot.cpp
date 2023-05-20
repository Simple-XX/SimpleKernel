
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

static CHAR16* exampleText = L"Example EFI Application. Press any key!";

/**
 * efi_main - The entry point for the EFI application
 * @image: firmware-allocated handle that identifies the image
 * @SystemTable: EFI system table
 */
extern "C" EFI_STATUS
efi_main(EFI_HANDLE image, EFI_SYSTEM_TABLE* systemTable) {
    UINTN                         index;
    EFI_EVENT                     event  = systemTable->ConIn->WaitForKey;

    SIMPLE_INPUT_INTERFACE*       conIn  = systemTable->ConIn;
    SIMPLE_TEXT_OUTPUT_INTERFACE* conOut = systemTable->ConOut;
    conOut->OutputString(conOut, exampleText);
    conOut->OutputString(L"%a", L"fetetet\n");

    systemTable->BootServices->WaitForEvent(1, &event, &index);

    return EFI_SUCCESS;
}
