#include <uefi.h>

/* public domain image loader - http://nothings.org/stb_image.h */
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

/**
 * Display PNG image
 */
int main(int argc, char **argv)
{
    efi_status_t status;
    efi_guid_t gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    efi_gop_t *gop = NULL;
    FILE *f;
    unsigned char *buff;
    uint32_t *data;
    int w, h, l;
    long int size;
    stbi__context s;
    stbi__result_info ri;

    /* load image */
    if((f = fopen("\\0C_png\\image.png", "r"))) {
        fseek(f, 0, SEEK_END);
        size = ftell(f);
        fseek(f, 0, SEEK_SET);
        buff = (unsigned char*)malloc(size);
        if(!buff) {
            fprintf(stderr, "unable to allocate memory\n");
            return 1;
        }
        fread(buff, size, 1, f);
        fclose(f);
        ri.bits_per_channel = 8;
        s.read_from_callbacks = 0;
        s.img_buffer = s.img_buffer_original = buff;
        s.img_buffer_end = s.img_buffer_original_end = buff + size;
        data = (uint32_t*)stbi__png_load(&s, &w, &h, &l, 4, &ri);
        if(!data) {
            fprintf(stdout, "Unable to decode png: %s\n", stbi__g_failure_reason);
            return 0;
        }
    } else {
        fprintf(stderr, "Unable to load image\n");
        return 0;
    }

    /* set video mode */
    status = BS->LocateProtocol(&gopGuid, NULL, (void**)&gop);
    if(!EFI_ERROR(status) && gop) {
        status = gop->SetMode(gop, 0);
        ST->ConOut->Reset(ST->ConOut, 0);
        ST->StdErr->Reset(ST->StdErr, 0);
        if(EFI_ERROR(status)) {
            fprintf(stderr, "unable to set video mode\n");
            return 0;
        }
    } else {
        fprintf(stderr, "unable to get graphics output protocol\n");
        return 0;
    }

    /* png is RGBA, but UEFI needs BGRA */
    if(gop->Mode->Information->PixelFormat == PixelBlueGreenRedReserved8BitPerColor ||
        (gop->Mode->Information->PixelFormat == PixelBitMask && gop->Mode->Information->PixelInformation.BlueMask != 0xff0000)) {
        for(l = 0; l < w * h; l++)
            data[l] = ((data[l] & 0xff) << 16) | (data[l] & 0xff00) | ((data[l] >> 16) & 0xff);
    }
    /* display image */
    gop->Blt(gop, data, EfiBltBufferToVideo, 0, 0, (gop->Mode->Information->HorizontalResolution - w) / 2,
        (gop->Mode->Information->VerticalResolution - h) / 2, w, h, 0);

    /* free resources exit */
    free(data);
    free(buff);
    return 0;
}
