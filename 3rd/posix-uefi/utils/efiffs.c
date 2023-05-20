/*
 * utils/efiffs.c
 *
 * Copyright (C) 2022 bzt (bztsrc@gitlab)
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * This file is part of the POSIX-UEFI package.
 * @brief small tool to convert an .efi file to an .ffs file
 *
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct {
    uint32_t Data1;
    uint16_t Data2;
    uint16_t Data3;
    uint8_t  Data4[8];
} __attribute__((packed)) guid_t;

/**
 * Print usage
 */
void usage(char *cmd)
{
    printf("POSIX-UEFI utils - efiffs by bztsrc@gitlab MIT\r\n\r\n");
    printf("%s [-g <guid>] [-n <name>] [-v <ver>] [-t <type>] [-p <type>] infile [outfile]\r\n\r\n", cmd);
    printf("  -g <guid>   specify the GUID (defaults to random)\r\n");
    printf("  -n <name>   specify the driver's name (eg 'FAT')\r\n");
    printf("  -v <ver>    specify the driver's version (eg '1.0')\r\n");
    printf("  -t <type>   specify the ffs type (defaults to 7, EFI_FV_FILETYPE_DRIVER)\r\n");
    printf("  -p <type>   specify the pe section type (defaults to 16, EFI_SECTION_PE32)\r\n");
    printf("  infile      input .efi file\r\n");
    printf("  outfile     output file name (default generated from infile)\r\n");
    exit(1);
}

/**
 * Convert hex string into integer
 */
unsigned int gethex(char *ptr, int len)
{
    unsigned int ret = 0;
    for(;len--;ptr++) {
        if(*ptr>='0' && *ptr<='9') {          ret <<= 4; ret += (unsigned int)(*ptr-'0'); }
        else if(*ptr >= 'a' && *ptr <= 'f') { ret <<= 4; ret += (unsigned int)(*ptr-'a'+10); }
        else if(*ptr >= 'A' && *ptr <= 'F') { ret <<= 4; ret += (unsigned int)(*ptr-'A'+10); }
        else break;
    }
    return ret;
}

/**
 * Parse a GUID in string into binary representation
 */
void getguid(char *ptr, guid_t *guid)
{
    int i;

    if(!ptr || !*ptr || ptr[8] != '-' || ptr[13] != '-' || ptr[18] != '-') {
        fprintf(stderr, "efiffs: bad GUID format\r\n");
        return;
    }
    guid->Data1 = gethex(ptr, 8); ptr += 9;
    guid->Data2 = gethex(ptr, 4); ptr += 5;
    guid->Data3 = gethex(ptr, 4); ptr += 5;
    guid->Data4[0] = gethex(ptr, 2); ptr += 2;
    guid->Data4[1] = gethex(ptr, 2); ptr += 2; if(*ptr == '-') ptr++;
    for(i = 2; i < 8; i++, ptr += 2) guid->Data4[i] = gethex(ptr, 2);
}

/**
 * Parse ffs type
 */
int gettype(char *str)
{
    static const char *types[] = {
        NULL, "RAW", "FREEFORM", "SECURITY_CORE", "PEI_CORE", "DXE_CORE", "PEIM", "DRIVER", "COMBINED_PEIM_DRIVER",
        "APPLICATION", "SMM", "FIRMWARE_VOLUME_IMAGE", "COMBINED_SMM_DXE", "SMM_CORE", NULL };
    int i;

    i = atoi(str);
    if(i > 0 && i < (int)(sizeof(types)/sizeof(types[0]))) return i;

    if(!memcmp(str, "EFI_FV_FILETYPE_", 16)) str += 16;
    for(i = 1; types[i] && strcmp(str, types[i]); i++);
    if(!types[i]) {
        fprintf(stderr, "efiffs: invalid ffs type, available values:\r\n");
        for(i = 1; types[i]; i++)
            fprintf(stderr, "  EFI_FV_FILETYPE_%s\r\n", types[i]);
        return 7; /* EFI_FV_FILETYPE_DRIVER */
    }
    return i;
}

/**
 * Parse section type
 */
int getsec(char *str)
{
    static const char *types[] = {
        "PE32", "PIC", "TE", "DXE_DEPEX", "VERSION", "USER_INTERFACE", "COMPATIBILITY16",
        "FIRMWARE_VOLUME_IMAGE", "FREEFORM_SUBTYPE_GUID", "RAW", "RESERVED", "PEI_DEPEX", "SMM_DEPEX", NULL };
    int i;

    i = atoi(str);
    if(i == 1 || i == 2 || i >= 16) return i;

    if(!memcmp(str, "EFI_SECTION_", 12)) str += 12;
    if(!strcmp(str, "COMPRESSION")) return 1;
    if(!strcmp(str, "GUID_DEFINED")) return 2;
    for(i = 0; types[i] && strcmp(str, types[i]); i++);
    if(!types[i]) {
        fprintf(stderr, "efiffs: invalid section type, available values:\r\n");
        fprintf(stderr, "  EFI_SECTION_COMPRESSION\r\n  EFI_SECTION_GUID_DEFINED\r\n");
        for(i = 0; types[i]; i++)
            fprintf(stderr, "  EFI_SECTION_%s\r\n", types[i]);
        return 16; /* EFI_SECTION_PE32 */
    }
    return i + 16;
}

/**
 * Calculate checksum
 */
uint8_t checksum8(uint8_t *buff, int len)
{
    uint8_t ret = 0;
    int i;

    for(i = 0; i < len; i++)
        ret = (uint8_t)(ret + buff[i]);
    return (uint8_t)(0x100 - ret);
}

/**
 * Main function
 */
int main(int argc, char **argv)
{
    FILE *f;
    int i, size, len;
    int filetype = 7; /* EFI_FV_FILETYPE_DRIVER */
    int sectype = 16; /* EFI_SECTION_PE32 */
    char *in = NULL, *out = NULL, *name = NULL, *ver = NULL;
    uint8_t *buff = NULL, *sec = NULL;
    guid_t guid;

    /* get a random GUID */
    srand(time(NULL));
    i = rand(); memcpy(&((uint8_t*)&guid)[0], &i, 4);
    i = rand(); memcpy(&((uint8_t*)&guid)[4], &i, 4);
    i = rand(); memcpy(&((uint8_t*)&guid)[8], &i, 4);
    i = rand(); memcpy(&((uint8_t*)&guid)[12], &i, 4);

    /* parse command line */
    for(i = 1; i < argc && argv[i]; i++)
        if(argv[i][0] == '-') {
            switch(argv[i][1]) {
                case 'g': getguid(argv[++i], &guid); break;
                case 'n': name = argv[++i]; break;
                case 'v': ver = argv[++i]; break;
                case 't': filetype = gettype(argv[++i]); break;
                case 'p': sectype = getsec(argv[++i]); break;
                default: usage(argv[0]); break;
            }
        } else
        if(!in) in = argv[i]; else
        if(!out) out = argv[i]; else
            usage(argv[0]);
    if(!in) usage(argv[0]);

    /* get input data */
    f = fopen(in, "rb");
    if(!f) {
        fprintf(stderr, "efiffs: unable to read '%s'\r\n", in);
        return 2;
    }
    fseek(f, 0L, SEEK_END);
    size = (int)ftell(f);
    fseek(f, 0L, SEEK_SET);
    len = 24 + 12 + 4 + size + (name ? 6 + 2 * strlen(name) : 0) + (ver ? 6 + 2 * strlen(ver) : 0);
    buff = (uint8_t*)malloc(len);
    if(!buff) { fprintf(stderr, "efiffs: unable to allocate memory\r\n"); return 2; }
    memset(buff, 0, len);
    sec = buff + 24;
    /* add the PE section */
    sec[0] = size & 0xff;
    sec[1] = (size >> 8) & 0xff;
    sec[2] = (size >> 16) & 0xff;
    sec[3] = sectype;
    fread(sec + 4, 1, size, f);
    fclose(f);
    sec += 4 + ((size + 3) & ~3);
    /* add the name section */
    if(name) {
        size = 6 + 2 * strlen(name);
        sec[0] = size & 0xff;
        sec[1] = (size >> 8) & 0xff;
        sec[2] = (size >> 16) & 0xff;
        sec[3] = 0x15; /* EFI_SECTION_USER_INTERFACE */
        for(i = 0; name[i]; i++)
            sec[4 + 2 * i] = name[i];
        sec += ((size + 3) & ~3);
    }
    /* add the version section */
    if(ver) {
        size = 6 + 2 * strlen(ver);
        sec[0] = size & 0xff;
        sec[1] = (size >> 8) & 0xff;
        sec[2] = (size >> 16) & 0xff;
        sec[3] = 0x14; /* EFI_SECTION_VERSION */
        for(i = 0; ver[i]; i++)
            sec[4 + 2 * i] = ver[i];
        sec += ((size + 3) & ~3);
    }

    /* calculate ffs header fields */
    len = (int)((uintptr_t)sec - (uintptr_t)buff);
    memcpy(buff, &guid, 16);
    buff[0x12] = filetype;
    buff[0x14] = len & 0xff;
    buff[0x15] = (len >> 8) & 0xff;
    buff[0x16] = (len >> 16) & 0xff;
    buff[0x11] = 0xAA;
    buff[0x10] = checksum8(buff, 24);

    /* write out image */
    if(!out) {
        i = strlen(in);
        out = (char*)malloc(i + 5);
        if(!out) { fprintf(stderr, "efiffs: unable to allocate memory\r\n"); return 2; }
        strcpy(out, in);
        strcpy(!strcmp(out + i - 4, ".efi") ? out + i - 4 : out + i, ".ffs");
    }
    f = fopen(out, "wb");
    if(!f) {
        fprintf(stderr, "efiffs: unable to write '%s'\r\n", out);
        return 2;
    }
    fwrite(buff, 1, len, f);
    fclose(f);

    free(buff);
    return 0;
}
