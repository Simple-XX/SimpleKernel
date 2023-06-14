#include <uefi.h>

/**
 * Get mouse pointer's coordinates
 */
int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    efi_status_t status;
    efi_guid_t ptrGuid = EFI_SIMPLE_POINTER_PROTOCOL_GUID;
    efi_simple_pointer_protocol_t *ptr = NULL;
    efi_simple_pointer_state_t state = {0};
    int x = 0, y = 0;

    status = BS->LocateProtocol(&ptrGuid, NULL, (void**)&ptr);
    if(!EFI_ERROR(status) && ptr) {
        /* if we got the interface, loop until a key pressed */
        printf("move mouse around, press any key to exit\n");
        while(!getchar_ifany()) {
            ptr->GetState(ptr, &state);
            x += state.RelativeMovementX;
            y += state.RelativeMovementY;
            printf("\rx %4d y %4d btns %d %d", x, y, state.LeftButton, state.RightButton);
            /* let the CPU rest a bit */
            usleep(100);
        }
        printf("\n");
    } else
        fprintf(stderr, "unable to get simple pointer protocol\n");
    return 0;
}
