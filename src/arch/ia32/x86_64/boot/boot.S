
/**
 * @file boot.S
 * @brief 启动代码，进行一些设置后跳转到 kernel_main
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2021-01-01
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2021-01-02<td>MRNIU<td>迁移到 doxygen
 * </table>
 */

.text
.align 4

.globl _start
_start:
    subq $8, %rsp
    pushq %rcx
    pushq %rdx
0:
    lea ImageBase(%rip), %rdi
    lea _DYNAMIC(%rip), %rsi

    popq %rcx
    popq %rdx
    pushq %rcx
    pushq %rdx
    call _relocate

    popq %rdi
    popq %rsi

    call efi_main
    addq $8, %rsp

.exit:
    ret

// hand-craft a dummy .reloc section so EFI knows it's a relocatable executable:

.data
dummy:	.long	0

#define IMAGE_REL_ABSOLUTE	0
.section .reloc, "a"
label1:
    .long	dummy-label1				// Page RVA
    .long	12					// Block Size (2*4+2*2), must be aligned by 32 Bits
    .word	(IMAGE_REL_ABSOLUTE<<12) +  0		// reloc for dummy
    .word	(IMAGE_REL_ABSOLUTE<<12) +  0		// reloc for dummy

#if defined(__ELF__) && defined(__linux__)
    .section .note.GNU-stack,"",%progbits
#endif

