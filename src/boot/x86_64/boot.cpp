
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

#if USE_GNU_UEFI == 1
#    include "efi.h"
#    include "efilib.h"
#else
#    include "elf.h"
#    include "uefi.h"
#endif

#include "kernel.h"

/// @todo 内核名称，由 cmake 传入
#define KERNEL_ELF_OUTPUT_NAME "kernel.elf"

/// 确定架构
#ifdef __x86_64__
#    define EM_MACH EM_X86_64
#endif
#ifdef __aarch64__
#    define EM_MACH EM_AARCH64
#endif

/**
 * List memory map
 */
int get_memory_map(void) {
    efi_status_t             status;
    efi_memory_descriptor_t* memory_map = nullptr;
    efi_memory_descriptor_t* mement;
    uintn_t                  memory_map_size = 0;
    uintn_t                  map_key         = 0;
    uintn_t                  desc_size       = 0;
    const char*              types[]         = { "EfiReservedMemoryType",
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
      = BS->GetMemoryMap(&memory_map_size, nullptr, &map_key, &desc_size, nullptr);
    if (status != EFI_BUFFER_TOO_SMALL || !memory_map_size) {
        fprintf(stderr, "status != EFI_BUFFER_TOO_SMALL [0x%X] || !memory_map_size [%d], desc_size [%d]\n",status,memory_map_size,desc_size);
        return 0;
    }
    /* in worst case malloc allocates two blocks, and each block might split a
     * record into three, that's 4 additional records */
    memory_map_size += 4 * desc_size;
    memory_map       = (efi_memory_descriptor_t*)malloc(memory_map_size);
    if (!memory_map) {
        fprintf(stderr, "!memory_map\n");
        return 1;
    }
    status = BS->GetMemoryMap(&memory_map_size, memory_map, &map_key,
                              &desc_size, nullptr);
    if (EFI_ERROR(status)) {
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
    return 0;

// #if defined(__x86_64__)
//     EFI_SYSTEM_TABLE* systemTable = (EFI_SYSTEM_TABLE*)_systemtable;
//     EFI_STATUS        status
//       = uefi_call_wrapper(systemTable->ConOut->OutputString, 2,
//                           systemTable->ConOut, L"Hello UEFI111!\n");

//     EFI_STATUS             Status;
//     EFI_MEMORY_DESCRIPTOR* EfiMemoryMap;
//     ptrdiff_t              EfiMemoryMapSize;
//     ptrdiff_t              EfiMapKey;
//     ptrdiff_t              EfiDescriptorSize;
//     UINT32                 EfiDescriptorVersion;

//     //
//     // Get the EFI memory map.
//     //
//     EfiMemoryMapSize = 0;
//     EfiMemoryMap     = nullptr;
//     Status = uefi_call_wrapper(systemTable->BootServices->GetMemoryMap, 5,
//                                &EfiMemoryMapSize, EfiMemoryMap, &EfiMapKey,
//                                &EfiDescriptorSize, &EfiDescriptorVersion);
//     ASSERT(Status == EFI_BUFFER_TOO_SMALL);

//     //
//     // Use size returned for the AllocatePool.
//     //
//     EfiMemoryMap
//       = (EFI_MEMORY_DESCRIPTOR*)AllocatePool(EfiMemoryMapSize
//                                              + 2 * EfiDescriptorSize);
//     ASSERT(EfiMemoryMap != nullptr);
//     Status = uefi_call_wrapper(systemTable->BootServices->GetMemoryMap, 5,
//                                &EfiMemoryMapSize, EfiMemoryMap, &EfiMapKey,
//                                &EfiDescriptorSize, &EfiDescriptorVersion);
//     if (EFI_ERROR(Status)) {
//         FreePool(EfiMemoryMap);
//     }

//     //
//     // Get descriptors
//     //
//     EFI_MEMORY_DESCRIPTOR* EfiEntry = EfiMemoryMap;
//     do {
//         // ... do something with EfiEntry ...
//         EfiEntry = NextMemoryDescriptor(EfiEntry, EfiDescriptorSize);

//     } while ((UINT8*)EfiEntry < (UINT8*)EfiMemoryMap + EfiMemoryMapSize);

// #endif

}

/**
 * @brief 获取内核入口地址
 * @return entry_func       内核入口地址
 */
static entry_func get_kernel_entry(void) {
    // 打开内核 elf 文件
    auto kernel_file = fopen(KERNEL_ELF_OUTPUT_NAME, "r");
    if (kernel_file == nullptr) {
        fprintf(stderr, "Unable to open file [%s]\n", KERNEL_ELF_OUTPUT_NAME);
        return nullptr;
    }

    // 获取内核大小
    fseek(kernel_file, 0, SEEK_END);
    auto size = ftell(kernel_file);
    // 将内核读入 buffer
    fseek(kernel_file, 0, SEEK_SET);
    auto kernel_buff = malloc(size + 1);
    if (kernel_buff == nullptr) {
        fprintf(stderr, "unable to allocate memory\n");
        return nullptr;
    }
    fread(kernel_buff, size, 1, kernel_file);
    fclose(kernel_file);

    // 验证 elf 信息
    auto kernel_elf_header = (Elf64_Ehdr*)kernel_buff;
    // magic 匹配
    auto is_magic = (memcmp(kernel_elf_header->e_ident, ELFMAG, SELFMAG) == 0);
    if (is_magic == false) {
        printf("is_magic == false\n");
        return nullptr;
    }
    // 是否为 64 位
    auto is_64bit = kernel_elf_header->e_ident[EI_CLASS] == ELFCLASS64;
    if (is_64bit == false) {
        printf("is_magic == false\n");
        return nullptr;
    }
    // 是否为 LSB
    auto is_lsb = kernel_elf_header->e_ident[EI_DATA] == ELFDATA2LSB;
    if (is_lsb == false) {
        printf("is_magic == false\n");
        return nullptr;
    }
    // 是否可执行
    auto is_executable = kernel_elf_header->e_type == ET_EXEC;
    if (is_executable == false) {
        printf("is_magic == false\n");
        return nullptr;
    }
    // 架构是否匹配
    auto is_architecture = kernel_elf_header->e_machine == EM_MACH;
    if (is_architecture == false) {
        printf("is_magic == false\n");
        return nullptr;
    }
    // 是否包含程序头
    auto is_has_headers = kernel_elf_header->e_phnum > 0;
    if (is_has_headers == false) {
        printf("is_magic == false\n");
        return nullptr;
    }

    // 加载段
    auto phdr
      = (Elf64_Phdr*)((uint8_t*)kernel_buff + kernel_elf_header->e_phoff);
    for (auto i = 0; i < kernel_elf_header->e_phnum; i++) {
        if (phdr->p_type == PT_LOAD) {
            printf("ELF segment %p %d bytes (bss %d bytes)\n", phdr->p_vaddr,
                   phdr->p_filesz, phdr->p_memsz - phdr->p_filesz);
            // 将内核的各个段复制到对应的虚拟地址
            memcpy((void*)phdr->p_vaddr, (uint8_t*)kernel_buff + phdr->p_offset,
                   phdr->p_filesz);
            memset((void*)(phdr->p_vaddr + phdr->p_filesz), 0,
                   phdr->p_memsz - phdr->p_filesz);
        }
        phdr = (Elf64_Phdr*)((uint8_t*)phdr + kernel_elf_header->e_phentsize);
    }

    // 保存入口地址
    auto entry = (entry_func)kernel_elf_header->e_entry;

    free(kernel_buff);

    printf("ELF entry point %p\n", entry);

    return entry;
}

#if USE_GNU_UEFI == 1

/**
 * efi_main - The entry point for the EFI application
 * @image: firmware-allocated handle that identifies the image
 * @SystemTable: EFI system table
 */
extern "C" EFI_STATUS
efi_main(void* systemTable) {
    // uefi_call_wrapper(InitializeLib, 2, image, systemTable);
    // EFI_STATUS status = uefi_call_wrapper(systemTable->ConOut->ClearScreen,
    // 1,
    //                                       systemTable->ConOut);

    // status            = uefi_call_wrapper(systemTable->ConOut->OutputString,
    // 2,
    //                                       systemTable->ConOut, L"Hello
    //                                       UEFI!\n");

    return EFI_SUCCESS;
}

#else

/**
 * @brief 进行系统初始化，加载内核
 */
extern "C" int main(int _argc, char** _argv) {
    get_memory_map();
    auto kernel_entry = get_kernel_entry();
    return kernel_entry(_argc, _argv);
}

#endif
