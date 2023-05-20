#include <uefi.h>

/**
 * Dump memory at given address, should accept 0x prefixes from the command line
 */
int main(int argc, char **argv)
{
    efi_physical_address_t address =
        (argc < 2 ? (efi_physical_address_t)IM : (efi_physical_address_t)atol(argv[1]));

    printf("%4D", address);
    return 0;
}
