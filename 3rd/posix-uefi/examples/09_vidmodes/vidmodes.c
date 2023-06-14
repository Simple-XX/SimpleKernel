#include <uefi.h>

/**
 * List or set GOP video modes
 */
int main(int argc, char **argv)
{
    efi_status_t status;
    efi_guid_t gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    efi_gop_t *gop = NULL;
    efi_gop_mode_info_t *info = NULL;
    uintn_t isiz = sizeof(efi_gop_mode_info_t), currentMode, i;

    status = BS->LocateProtocol(&gopGuid, NULL, (void**)&gop);
    if(!EFI_ERROR(status) && gop) {
        /* if mode given on command line, set it */
        if(argc > 1) {
            status = gop->SetMode(gop, atoi(argv[1]));
            /* changing the resolution might mess up ConOut and StdErr, better to reset them */
            ST->ConOut->Reset(ST->ConOut, 0);
            ST->StdErr->Reset(ST->StdErr, 0);
            if(EFI_ERROR(status)) {
                fprintf(stderr, "unable to set video mode\n");
                return 0;
            }
        }
        /* we got the interface, get current mode */
        status = gop->QueryMode(gop, gop->Mode ? gop->Mode->Mode : 0, &isiz, &info);
        if(status == EFI_NOT_STARTED || !gop->Mode) {
            status = gop->SetMode(gop, 0);
            ST->ConOut->Reset(ST->ConOut, 0);
            ST->StdErr->Reset(ST->StdErr, 0);
        }
        if(EFI_ERROR(status)) {
            fprintf(stderr, "unable to get current video mode\n");
            return 0;
        }
        currentMode = gop->Mode->Mode;
        /* iterate on modes and print info */
        for(i = 0; i < gop->Mode->MaxMode; i++) {
            status = gop->QueryMode(gop, i, &isiz, &info);
            if(EFI_ERROR(status) || info->PixelFormat > PixelBitMask) continue;
            printf(" %c%3d. %4d x%4d (pitch %4d fmt %d r:%06x g:%06x b:%06x)\n",
                i == currentMode ? '*' : ' ', i,
                info->HorizontalResolution, info->VerticalResolution, info->PixelsPerScanLine, info->PixelFormat,
                info->PixelFormat==PixelRedGreenBlueReserved8BitPerColor?0xff:(
                info->PixelFormat==PixelBlueGreenRedReserved8BitPerColor?0xff0000:(
                info->PixelFormat==PixelBitMask?info->PixelInformation.RedMask:0)),
                info->PixelFormat==PixelRedGreenBlueReserved8BitPerColor ||
                info->PixelFormat==PixelBlueGreenRedReserved8BitPerColor?0xff00:(
                info->PixelFormat==PixelBitMask?info->PixelInformation.GreenMask:0),
                info->PixelFormat==PixelRedGreenBlueReserved8BitPerColor?0xff0000:(
                info->PixelFormat==PixelBlueGreenRedReserved8BitPerColor?0xff:(
                info->PixelFormat==PixelBitMask?info->PixelInformation.BlueMask:0)));
        }
    } else
        fprintf(stderr, "unable to get graphics output protocol\n");
    return 0;
}
