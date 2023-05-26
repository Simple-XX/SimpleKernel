#include <uefi.h>

/**
 * Print out arguments. This source can be compiled with and without UEFI_NO_UTF8
 */
int main(int argc, char_t **argv)
{
    int i;

    printf(CL("I got %d argument%s:\n"), argc, argc > 1 ? CL("s") : CL(""));
    for(i = 0; i < argc; i++)
        printf(CL("  argv[%d] = '%s'\n"), i, argv[i]);

    return 0;
}
