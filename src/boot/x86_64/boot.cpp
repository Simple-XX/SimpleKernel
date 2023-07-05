
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

#include "cstdint"

#include "kernel.h"

#if USE_GNU_UEFI == 1

/**
 * efi_main - The entry point for the EFI application
 * @image: firmware-allocated handle that identifies the image
 * @SystemTable: EFI system table
 */
extern "C" EFI_STATUS
efi_main(EFI_HANDLE image, EFI_SYSTEM_TABLE* systemTable) {
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

#    ifdef __x86_64__
#        define EM_MACH EM_X86_64
#    endif
#    ifdef __aarch64__
#        define EM_MACH EM_AARCH64
#    endif

/**
 * @brief 进行系统初始化，加载内核
 */
extern "C" int main(int argc, char** argv) {
    (void)argc;
    (void)argv;
    FILE*       f;
    char*       buff;
    long int    size;
    Elf64_Ehdr* elf;
    Elf64_Phdr* phdr;
    uintptr_t   entry;
    int         i;

    /* load the file */
    if ((f = fopen("kernel.elf", "r"))) {
        fseek(f, 0, SEEK_END);
        size = ftell(f);
        fseek(f, 0, SEEK_SET);
        buff = (char*)malloc(size + 1);
        if (!buff) {
            fprintf(stderr, "unable to allocate memory\n");
            return 1;
        }
        fread(buff, size, 1, f);
        fclose(f);
    }
    else {
        fprintf(stderr, "Unable to open file\n");
        return 0;
    }

    /* is it a valid ELF executable for this architecture? */
    elf = (Elf64_Ehdr*)buff;
    if (!memcmp(elf->e_ident, ELFMAG, SELFMAG) && /* magic match? */
        elf->e_ident[EI_CLASS] == ELFCLASS64 &&   /* 64 bit? */
        elf->e_ident[EI_DATA] == ELFDATA2LSB &&   /* LSB? */
        elf->e_type == ET_EXEC &&                 /* executable object? */
        elf->e_machine == EM_MACH &&              /* architecture match? */
        elf->e_phnum > 0) {                       /* has program headers? */
        /* load segments */
        for (phdr = (Elf64_Phdr*)(buff + elf->e_phoff), i = 0; i < elf->e_phnum;
             i++, phdr = (Elf64_Phdr*)((uint8_t*)phdr + elf->e_phentsize)) {
            if (phdr->p_type == PT_LOAD) {
                printf("ELF segment %p %d bytes (bss %d bytes)\n",
                       phdr->p_vaddr, phdr->p_filesz,
                       phdr->p_memsz - phdr->p_filesz);
                memcpy((void*)phdr->p_vaddr, buff + phdr->p_offset,
                       phdr->p_filesz);
                memset((void*)(phdr->p_vaddr + phdr->p_filesz), 0,
                       phdr->p_memsz - phdr->p_filesz);
            }
        }
        entry = elf->e_entry;
    }
    else {
        fprintf(stderr, "not a valid ELF executable for this architecture\n");
        return 0;
    }
    /* free resources */
    free(buff);

    /* execute the "kernel" */
    printf("ELF entry point %p\n", entry);

    entry_func entry_fp = (int (*)(int, char**))entry;
    i                   = entry_fp(0, nullptr);

    printf("ELF returned %d\n", i);

    return 0;
}

#endif
