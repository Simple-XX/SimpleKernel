TARGET = elfload.efi
SRCS = elfload.c
ALSO = kernel.elf

#USE_GCC=1
include uefi/Makefile

kernel.elf:
	@make -C kernel all USE_GCC=$(USE_GCC)
