#include <uefi.h>

/* Scalable Screen Font (https://gitlab.com/bztsrc/scalable-font2) */
#define SSFN_IMPLEMENTATION     /* get the normal renderer implementation */
#define SSFN_MAXLINES 4096      /* configure for static memory management */
#include "ssfn.h"

ssfn_buf_t dst = {0};           /* framebuffer properties */
ssfn_t     ctx = {0};           /* renderer context */

/**
 * Display string using the SSFN library. This works with both bitmap and vector fonts
 */
void printString(int x, int y, char *s)
{
    int ret;
    dst.x = x; dst.y = y;
    while((ret = ssfn_render(&ctx, &dst, s)) > 0)
        s += ret;
}

/**
 * Display vector fonts
 */
int main(int argc, char **argv)
{
    efi_status_t status;
    efi_guid_t gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    efi_gop_t *gop = NULL;
    FILE *f;
    ssfn_font_t *font;
    long int size;

    /* load font */
    if((f = fopen("\\0B_vecfont\\font.sfn", "r"))) {
        fseek(f, 0, SEEK_END);
        size = ftell(f);
        fseek(f, 0, SEEK_SET);
        font = (ssfn_font_t*)malloc(size + 1);
        if(!font) {
            fprintf(stderr, "unable to allocate memory\n");
            return 1;
        }
        fread(font, size, 1, f);
        fclose(f);
        ssfn_load(&ctx, font);
    } else {
        fprintf(stderr, "Unable to load font\n");
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
        /* set up destination buffer */
        dst.ptr = (unsigned char*)gop->Mode->FrameBufferBase;
        dst.w = gop->Mode->Information->HorizontalResolution;
        dst.h = gop->Mode->Information->VerticalResolution;
        dst.p = sizeof(unsigned int) * gop->Mode->Information->PixelsPerScanLine;
        dst.fg = 0xFFFFFFFF;
    } else {
        fprintf(stderr, "unable to get graphics output protocol\n");
        return 0;
    }

    /* select typeface to use */
    ssfn_select(&ctx, SSFN_FAMILY_ANY, NULL, SSFN_STYLE_REGULAR | SSFN_STYLE_NOCACHE, 40);

    /* display multilingual text */
    printString(10, ctx.size, "Hello!  Здравствуйте!  Καλως ηρθες!");

    /* free resources exit */
    ssfn_free(&ctx);
    free(font);
    return 0;
}
