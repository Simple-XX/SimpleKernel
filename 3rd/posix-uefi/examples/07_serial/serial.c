#include <uefi.h>

/**
 * Use serial port
 */
int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    FILE *f;
    char buff[2];

    if((f = fopen("/dev/serial", "r"))) {
        fwrite("sending bytes as-is with fwrite\r\n", 33, 1, f);
        fprintf(f, "sending characters automatically wchar_t/char converted with fprintf\n");
        printf("Press a key on the other side\n");
        while(!fread(buff, 1, 1, f));
        buff[1] = 0;
        printf("Got key: %02x '%s'\n", buff[0], buff);
        fclose(f);
    } else
        fprintf(stderr, "Unable to open serial port\n");
    return 0;
}
