#include <uefi.h>

/**
 * Display file contents
 */
int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    FILE *f;
    char buff[2048], fn[16];
    int i;

    for(i = 0; i < 8; i++) {
        sprintf(fn, "/dev/disk%d", i);
        printf("trying to open '%s'... ",fn);
        if((f = fopen(fn, "r"))) {
            memset(buff, 0, sizeof(buff));
            fread(buff, sizeof(buff), 1, f);
            printf("OK!\n");
            printf("%1D", (efi_physical_address_t)buff);
            fclose(f);
        } else
            printf("nope\n");
    }
    return 0;
}
