
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

    return EFI_SUCCESS;
}

#else

/**
 * Dump memory at given address, should accept 0x prefixes from the command line
 */
extern "C" int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    printf("Hello World!\n");

    efi_status_t             status;
    efi_memory_descriptor_t *memory_map      = NULL, *mement;
    uintn_t                  memory_map_size = 0, map_key = 0, desc_size = 0;
    const char*              types[] = { "EfiReservedMemoryType",
                                         "EfiLoaderCode",
                                         "EfiLoaderData",
                                         "EfiBootServicesCode",
                                         "EfiBootServicesData",
                                         "EfiRuntimeServicesCode",
                                         "EfiRuntimeServicesData",
                                         "EfiConventionalMemory",
                                         "EfiUnusableMemory",
                                         "EfiACPIReclaimMemory",
                                         "EfiACPIMemoryNVS",
                                         "EfiMemoryMappedIO",
                                         "EfiMemoryMappedIOPortSpace",
                                         "EfiPalCode" };

    /* get the memory map */
    status
      = BS->GetMemoryMap(&memory_map_size, NULL, &map_key, &desc_size, NULL);
    if (status != EFI_BUFFER_TOO_SMALL || !memory_map_size) {
        goto err;
    }
    /* in worst case malloc allocates two blocks, and each block might split a
     * record into three, that's 4 additional records */
    memory_map_size += 4 * desc_size;
    memory_map       = (efi_memory_descriptor_t*)malloc(memory_map_size);
    if (!memory_map) {
        fprintf(stderr, "unable to allocate memory\n");
        return 1;
    }
    status = BS->GetMemoryMap(&memory_map_size, memory_map, &map_key,
                              &desc_size, NULL);
    if (EFI_ERROR(status)) {
err:
        fprintf(stderr, "Unable to get memory map\n");
        return 0;
    }

    printf("Address              Size Type\n");
    for (mement = memory_map;
         (uint8_t*)mement < (uint8_t*)memory_map + memory_map_size;
         mement = NextMemoryDescriptor(mement, desc_size)) {
        printf("%016x %8d %02x %s\n", mement->PhysicalStart,
               mement->NumberOfPages, mement->Type, types[mement->Type]);
    }

    free(memory_map);

    printf("DONE\n");

    return 0;
}

#endif
