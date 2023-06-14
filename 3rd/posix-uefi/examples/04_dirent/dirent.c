#include <uefi.h>

/**
 * List directory contents
 */
int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    DIR *dh;
    struct dirent *de;

    if((dh = opendir("\\04_dirent"))) {
        while ((de = readdir(dh)) != NULL) {
            printf("%c %04x %s\n", de->d_type == DT_DIR ? 'd' : '.', de->d_type, de->d_name);
        }
        closedir(dh);
    } else
        fprintf(stderr, "Unable to open directory\n");
    return 0;
}
