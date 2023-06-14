/*
 * utils/efidsk.c
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
 * @brief small tool to create a disk image with EFI System Partition
 *
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>

typedef struct {
    uint32_t Data1;
    uint16_t Data2;
    uint16_t Data3;
    uint8_t  Data4[8];
} __attribute__((packed)) guid_t;
guid_t dguid, pguid;

/**
 * Print usage
 */
void usage(char *cmd)
{
    printf("POSIX-UEFI utils - efidsk by bztsrc@gitlab MIT\r\n\r\n");
    printf("%s [-p|-c] [-s <size>] indir outfile\r\n\r\n", cmd);
    printf("  -p          save only the partition image without GPT\r\n");
    printf("  -c          save EFI CDROM (ISO9660 El Torito no emulation boot catalog)\r\n");
    printf("  -s <size>   set the size of partition in megabytes (defaults to 33M)\r\n");
    printf("  indir       use the contents of this directory\r\n");
    printf("  outfile     output image file name\r\n");
    exit(1);
}

#define SECTOR_PER_CLUSTER  1
#define FIRST_PARTITION     2048

struct tm *fat_ts;
int fs_len = 0, skipbytes = 0, fat_nextcluster, fat_bpc, fat_spf, fat_lfncnt, fat_numclu = 67584;
unsigned char *fs_base = NULL;
unsigned char *fat_rootdir, *fat_data, fat_lfn[769];
uint32_t *fat_fat32_1, *fat_fat32_2;

/**
 * Add a new cluster
 */
unsigned char *fat_newclu(int parent)
{
    int clu;
    while(parent != fat_nextcluster && fat_fat32_1[parent] && fat_fat32_1[parent] != 0xFFFFFFF)
        parent = fat_fat32_1[parent];
    fat_fat32_1[parent] = fat_fat32_2[parent] = fat_nextcluster;
    fat_fat32_1[fat_nextcluster] = fat_fat32_2[fat_nextcluster] = 0xFFFFFFF;
    clu = fat_nextcluster++;
    if(fat_nextcluster >= fat_numclu) { fprintf(stderr,"efidsk: not enough space on partition\r\n"); exit(1); }
    return fat_data + clu * fat_bpc;
}

/**
 * Read in file name
 */
unsigned char *fat_readlfn(unsigned char *dir, int *clu, int *size, int parent)
{
    uint16_t uc2[256], *u;
    unsigned char *s, *d;
    int i = 0, n;
    memset(fat_lfn, 0, sizeof(fat_lfn));
    if(!dir[0]) return dir;
    while(dir[0] == '.') dir += 32;
    fat_lfncnt++;
    if(parent != 2 && !((uint64_t)(dir - fs_base) & (fat_bpc - 1))) {
        parent = fat_fat32_1[parent];
        if(!parent || parent == 0xFFFFFFF) return NULL;
        dir = fat_data + parent * fat_bpc;
    }
    if(dir[0xB] != 0xF) {
        for(s = dir, d = fat_lfn, i = 0; *s && *s != ' ' && i < 8; i++)
            *d++ = *s++;
        if(dir[8] && dir[8] != ' ') {
            *d++ = '.';
            for(s = dir + 8; *s != ' ' && i < 3; i++)
                *d++ = *s++;
        }
    } else {
        memset(uc2, 0, sizeof(uc2));
        n = dir[0] & 0x3F;
        u = uc2 + (n - 1) * 13;
        while(n--) {
            for(i = 0; i < 5; i++)
                u[i] = dir[i*2+2] << 8 | dir[i*2+1];
            for(i = 0; i < 6; i++)
                u[i+5] = dir[i*2+0xF] << 8 | dir[i*2+0xE];
            u[11] = dir[0x1D] << 8 | dir[0x1C];
            u[12] = dir[0x1F] << 8 | dir[0x1E];
            u -= 13;
            dir += 32;
            if(!((uint64_t)(dir - fs_base) & (fat_bpc - 1))) {
                parent = fat_fat32_1[parent];
                if(!parent || parent == 0xFFFFFFF) return NULL;
                dir = fat_data + parent * fat_bpc;
            }
        }
        for(d = fat_lfn, u = uc2; *u; u++)
            if(*u < 0x80) {
                *d++ = *u;
            } else if(*u < 0x800) {
                *d++ = ((*u>>6)&0x1F)|0xC0;
                *d++ = (*u&0x3F)|0x80;
            } else {
                *d++ = ((*u>>12)&0x0F)|0xE0;
                *d++ = ((*u>>6)&0x3F)|0x80;
                *d++ = (*u&0x3F)|0x80;
            }
    }
    *clu = (dir[0x15] << 24) | (dir[0x14] << 16) | (dir[0x1B] << 8) | dir[0x1A];
    *size = (dir[0x1F] << 24) | (dir[0x1E] << 16) | (dir[0x1D] << 8) | dir[0x1C];
    return dir + 32;
}

/**
 * Write file name
 */
unsigned char *fat_writelfn(unsigned char *dir, char *name, int type, int size, int parent, int clu)
{
    uint16_t uc2[256], *u;
    unsigned char *s, c = 0, sfn[12];
    int i, n;
    if(name[0] == '.') {
        memset(dir, ' ', 11);
        memcpy(dir, name, strlen(name));
    } else {
        memset(uc2, 0, sizeof(uc2));
        for(n = 0, u = uc2, s = (unsigned char*)name; *s; n++, u++) {
            if((*s & 128) != 0) {
                if((*s & 32) == 0) { *u = ((*s & 0x1F)<<6)|(*(s+1) & 0x3F); s += 2; } else
                if((*s & 16) == 0) { *u = ((*s & 0xF)<<12)|((*(s+1) & 0x3F)<<6)|(*(s+2) & 0x3F); s += 3; }
                else { fprintf(stderr,"efidsk: unable to encode file name '%s'\r\n", name); exit(1); }
            } else
                *u = *s++;
        }
        /* don't convert "Microsoft" to "MICROS~1   ", that's patented... */
        sprintf((char*)sfn, "~%07xLFN", fat_lfncnt++);
        for(i = 0; i < 11; i++)
            c = (((c & 1) << 7) | ((c & 0xfe) >> 1)) + sfn[i];
        n = (n + 12) / 13;
        u = uc2 + (n - 1) * 13;
        i = 0x40;
        while(n--) {
            if(parent > 2 && !((uint64_t)(dir - fs_base) & (fat_bpc - 1)))
                dir = fat_newclu(parent);
            dir[0] = i | (n + 1);
            dir[11] = 0xF;
            dir[0xD] = c;
            memcpy(dir + 1, (unsigned char*)u, 10);
            memcpy(dir + 14, (unsigned char*)u + 10, 12);
            memcpy(dir + 28, (unsigned char*)u + 22, 4);
            i = 0;
            u -= 13;
            dir += 32;
        }
        if(parent > 2 && !((uint64_t)(dir - fs_base) & (fat_bpc - 1)))
            dir = fat_newclu(parent);
        memcpy(dir, sfn, 11);
    }
    if(type) {
        dir[0xB] = 0x10;
    } else {
        dir[0x1C] = size & 0xFF; dir[0x1D] = (size >> 8) & 0xFF;
        dir[0x1E] = (size >> 16) & 0xFF; dir[0x1F] = (size >> 24) & 0xFF;
    }
    if(!clu) clu = size > 0 || type ? fat_nextcluster : 0;
    if(clu < 3) clu = 0;
    dir[0x1A] = clu & 0xFF; dir[0x1B] = (clu >> 8) & 0xFF;
    dir[0x14] = (clu >> 16) & 0xFF; dir[0x15] = (clu >> 24) & 0xFF;
    i = (fat_ts->tm_hour << 11) | (fat_ts->tm_min << 5) | (fat_ts->tm_sec/2);
    dir[0xE] = dir[0x16] = i & 0xFF; dir[0xF] = dir[0x17] = (i >> 8) & 0xFF;
    i = ((fat_ts->tm_year+1900-1980) << 9) | ((fat_ts->tm_mon+1) << 5) | (fat_ts->tm_mday);
    return dir + 32;
}

/**
 * Create a fat file system
 */
void fat_open(int start)
{
    int i;

    if(fat_numclu < 67584) { fprintf(stderr,"efidsk: not enough clusters\r\n"); exit(1); }
    /* "format" the partition to FAT32 */
    fs_len = fat_numclu * 512 * SECTOR_PER_CLUSTER;
    fs_base = realloc(fs_base, fs_len);
    if(!fs_base) { fprintf(stderr,"efidsk: unable to allocate memory\r\n"); exit(1); }
    memset(fs_base, 0, fs_len);
    memcpy(fs_base + 3, "MSWIN4.1", 8);
    fs_base[0xC] = 2; fs_base[0x10] = 2; fs_base[0x15] = 0xF8; fs_base[0x1FE] = 0x55; fs_base[0x1FF] = 0xAA;
    fs_base[0x18] = 0x20; fs_base[0x1A] = 0x40;
    memcpy(fs_base + 0x1C, &start, 4);
    memcpy(fs_base + 0x20, &fat_numclu, 4);
    fat_spf = (fat_numclu*4) / 512;
    fs_base[0xD] = SECTOR_PER_CLUSTER; fs_base[0xE] = 8;
    fs_base[0x24] = fat_spf & 0xFF; fs_base[0x25] = (fat_spf >> 8) & 0xFF;
    fs_base[0x26] = (fat_spf >> 16) & 0xFF; fs_base[0x27] = (fat_spf >> 24) & 0xFF;
    fs_base[0x2C] = 2; fs_base[0x30] = 1; fs_base[0x32] = 6; fs_base[0x40] = 0x80; fs_base[0x42] = 0x29;
    memcpy(fs_base + 0x43, &pguid, 4);
    memcpy(fs_base + 0x47, "EFI System FAT32   ", 19);
    memcpy(fs_base + 0x200, "RRaA", 4); memcpy(fs_base + 0x3E4, "rrAa", 4);
    for(i = 0; i < 8; i++) fs_base[0x3E8 + i] = 0xFF;
    fs_base[0x3FE] = 0x55; fs_base[0x3FF] = 0xAA;
    fat_bpc = fs_base[0xD] * 512;
    fat_rootdir = fs_base + (fat_spf*fs_base[0x10]+fs_base[0xE]) * 512;
    fat_data = fat_rootdir - 2*fat_bpc;
    fat_fat32_1 = (uint32_t*)(&fs_base[fs_base[0xE] * 512]);
    fat_fat32_2 = (uint32_t*)(&fs_base[(fs_base[0xE]+fat_spf) * 512]);
    fat_fat32_1[0] = fat_fat32_2[0] = fat_fat32_1[2] = fat_fat32_2[2] = 0x0FFFFFF8;
    fat_fat32_1[1] = fat_fat32_2[1] = 0x0FFFFFFF;
    fat_nextcluster = 3;
}

/**
 * Add a file to the file system
 */
void fat_add(struct stat *st, char *name, unsigned char *content, int size)
{
    int parent = 2, clu, i, j;
    unsigned char *dir = fat_rootdir;
    char *end, *fn = strrchr(name, '/');
    if(!fn) fn = name; else fn++;
    if(!strcmp(fn, ".") || !strcmp(fn, "..")) return;
    if(!S_ISREG(st->st_mode) && !S_ISDIR(st->st_mode)) return;
    fat_ts = gmtime(&st->st_mtime);
    fn = name;
    end = strchr(name, '/');
    if(!end) end = name + strlen(name);
    fat_lfncnt = 1;
    do {
        dir = fat_readlfn(dir, &clu, &j, parent);
        if(!dir) return;
        if(!memcmp(fat_lfn, fn, end - fn) && !fat_lfn[end - fn]) {
            fat_lfncnt = 1;
            parent = clu;
            dir = fat_data + parent * fat_bpc + 64;
            fn = end + 1;
            end = *end ? strchr(fn, '/') : NULL;
            if(!end) { end = fn + strlen(fn); break; }
        }
    } while(dir[0]);
    dir = fat_writelfn(dir, fn, S_ISDIR(st->st_mode), size, parent, 0);
    if(S_ISDIR(st->st_mode)) {
        dir = fat_newclu(fat_nextcluster);
        dir = fat_writelfn(dir, ".", 1, 0, 2, fat_nextcluster - 1);
        dir = fat_writelfn(dir, "..", 1, 0, 2, parent);
    } else if(content && size > 0) {
        if(fat_nextcluster * fat_bpc + size >= fs_len) {
            fprintf(stderr,"efidsk: not enough space on partition\r\n");
            exit(1);
        }
        memcpy(fat_data + fat_nextcluster * fat_bpc, content, size);
        for(i = 0; i < ((size + fat_bpc-1) & ~(fat_bpc-1)); i += fat_bpc, fat_nextcluster++) {
            fat_fat32_1[fat_nextcluster] = fat_fat32_2[fat_nextcluster] = fat_nextcluster+1;
        }
        fat_fat32_1[fat_nextcluster-1] = fat_fat32_2[fat_nextcluster-1] = 0xFFFFFFF;
    }
}

/**
 * Close the file system
 */
void fat_close()
{
    int i;
    if(!fs_base || fs_len < 512) return;
    fat_nextcluster -= 2;
    i = ((fs_len - (fat_spf*fs_base[0x10]+fs_base[0xE]) * 512)/fat_bpc) - fat_nextcluster;
    fs_base[0x3E8] = i & 0xFF; fs_base[0x3E9] = (i >> 8) & 0xFF;
    fs_base[0x3EA] = (i >> 16) & 0xFF; fs_base[0x3EB] = (i >> 24) & 0xFF;
    fs_base[0x3EC] = fat_nextcluster & 0xFF; fs_base[0x3ED] = (fat_nextcluster >> 8) & 0xFF;
    fs_base[0x3EE] = (fat_nextcluster >> 16) & 0xFF; fs_base[0x3EF] = (fat_nextcluster >> 24) & 0xFF;
    /* copy backup boot sectors */
    memcpy(fs_base + (fs_base[0x32]*512), fs_base, 1024);
}

/**
 * Read a file entirely into memory
 */
long int read_size;
unsigned char* readfileall(char *file)
{
    unsigned char *data=NULL;
    FILE *f;
    read_size = 0;
    if(!file || !*file) return NULL;
    f = fopen(file,"r");
    if(f) {
        fseek(f, 0L, SEEK_END);
        read_size = (long int)ftell(f);
        fseek(f, 0L, SEEK_SET);
        data = (unsigned char*)malloc(read_size + 1);
        if(!data) { fprintf(stderr, "efidsk: unable to allocate memory\r\n"); exit(1); }
        memset(data, 0, read_size + 1);
        fread(data, read_size, 1, f);
        data[read_size] = 0;
        fclose(f);
    }
    return data;
}

/**
 * Recursively parse a directory
 */
void parsedir(char *directory, int parent)
{
    DIR *dir;
    struct dirent *ent;
    char full[8192];
    unsigned char *tmp;
    struct stat st;

    if(!parent && !skipbytes) skipbytes = strlen(directory) + 1;

    if ((dir = opendir(directory)) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            if(!parent && (!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, ".."))) continue;
            full[0] = 0;
            strncat(full, directory, sizeof(full)-1);
            strncat(full, "/", sizeof(full)-1);
            strncat(full, ent->d_name, sizeof(full)-1);
            if(stat(full, &st)) continue;
            read_size = 0;
            if(S_ISDIR(st.st_mode)) {
                fat_add(&st, full + skipbytes, NULL, 0);
                if(strcmp(ent->d_name, ".") && strcmp(ent->d_name, ".."))
                    parsedir(full, parent+1);
            } else
            if(S_ISREG(st.st_mode)) {
                tmp = readfileall(full);
                fat_add(&st, full + skipbytes, tmp, read_size);
                if(tmp) free(tmp);
            }
        }
        closedir(dir);
    }
}

/**
 * CRC calculation with precalculated lookup table
 */
uint32_t crc32_lookup[256]={
    0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3, 0x0edb8832,
    0x79dcb8a4, 0xe0d5e91e, 0x97d2d988, 0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
    0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7, 0x136c9856, 0x646ba8c0, 0xfd62f97a,
    0x8a65c9ec, 0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
    0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940, 0x32d86ce3,
    0x45df5c75, 0xdcd60dcf, 0xabd13d59, 0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
    0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924, 0x2f6f7c87, 0x58684c11, 0xc1611dab,
    0xb6662d3d, 0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
    0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01, 0x6b6b51f4,
    0x1c6c6162, 0x856530d8, 0xf262004e, 0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
    0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65, 0x4db26158, 0x3ab551ce, 0xa3bc0074,
    0xd4bb30e2, 0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
    0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086, 0x5768b525,
    0x206f85b3, 0xb966d409, 0xce61e49f, 0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
    0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a, 0xead54739, 0x9dd277af, 0x04db2615,
    0x73dc1683, 0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
    0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7, 0xfed41b76,
    0x89d32be0, 0x10da7a5a, 0x67dd4acc, 0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
    0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b, 0xd80d2bda, 0xaf0a1b4c, 0x36034af6,
    0x41047a60, 0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
    0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7,
    0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d, 0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
    0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38, 0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7,
    0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
    0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45, 0xa00ae278,
    0xd70dd2ee, 0x4e048354, 0x3903b3c2, 0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
    0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9, 0xbdbdf21c, 0xcabac28a, 0x53b39330,
    0x24b4a3a6, 0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
    0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d};
uint32_t crc32_calc(unsigned char *start,int length)
{
    uint32_t crc32_val=0xffffffff;
    while(length--) crc32_val=(crc32_val>>8)^crc32_lookup[(crc32_val&0xff)^(unsigned char)*start++];
    crc32_val^=0xffffffff;
    return crc32_val;
}

/**
 * Write out GUID Partitioning Table
 */
void setint(int val, unsigned char *ptr) { memcpy(ptr,&val,4); }
void writegpt(FILE *f)
{
    guid_t efiguid = { 0xC12A7328, 0xF81F, 0x11D2, { 0xBA,0x4B,0x00,0xA0,0xC9,0x3E,0xC9,0x3B} };
    int i;
    unsigned char *gpt = malloc(FIRST_PARTITION * 512), gpt2[512], *p;
    char *name;
    if(!gpt) { fprintf(stderr, "efidsk: unable to allocate memory\r\n"); exit(1); }
    memset(gpt, 0, FIRST_PARTITION * 512);

    gpt[0x1FE]=0x55; gpt[0x1FF]=0xAA;
    memcpy(gpt+0x1B8, &dguid.Data1, 4);         /* WinNT disk id */
    /* MBR, EFI System Partition / boot partition. */
    gpt[0x1C0-2]=0x80;                          /* bootable flag */
    setint(FIRST_PARTITION+1,gpt+0x1C0);        /* start CHS */
    gpt[0x1C0+2]=0xC;                           /* type, LBA FAT32 (0xC) */
    setint(fs_len/512,gpt+0x1C0+4);             /* end CHS */
    setint(FIRST_PARTITION,gpt+0x1C0+6);        /* start LBA */
    setint(fs_len/512,gpt+0x1C0+10);            /* number of sectors */
    /* MBR, protective GPT entry */
    setint(1,gpt+0x1D0);                        /* start CHS */
    gpt[0x1D0+2]=0xEE;                          /* type */
    setint(FIRST_PARTITION+1,gpt+0x1D0+4);      /* end CHS */
    setint(1,gpt+0x1D0+6);                      /* start LBA */
    setint(FIRST_PARTITION,gpt+0x1D0+10);       /* number of sectors */

    /* GPT header */
    p = gpt + 512;
    memcpy(p,"EFI PART",8);                     /* magic */
    setint(1,p+10);                             /* revision */
    setint(92,p+12);                            /* size */
    setint(1,p+24);                             /* primary LBA */
    setint(2*FIRST_PARTITION+fat_numclu-2,p+32);/* secondary LBA */
    setint(FIRST_PARTITION,p+40);               /* first usable LBA */
    setint(FIRST_PARTITION+fat_numclu,p+48);    /* last usable LBA */
    memcpy(p+56,&dguid,sizeof(guid_t));         /* disk UUID */
    setint(2,p+72);                             /* partitioning table LBA */
    setint((FIRST_PARTITION-2)*512/128,p+80);   /* number of entries */
    setint(128,p+84);                           /* size of one entry */
    p += 512;

    /* GPT, EFI System Partition (ESP) */
    memcpy(p, &efiguid, sizeof(guid_t));        /* entry type */
    memcpy(p+16, &pguid, sizeof(guid_t));       /* partition UUID */
    setint(FIRST_PARTITION,p+32);               /* start LBA */
    setint(FIRST_PARTITION+fat_numclu-1,p+40);  /* end LBA */
    name = "EFI System Partition";              /* name */
    for(i = 0; name[i]; i++) p[56+i*2] = name[i];
    p += 128;

    /* calculate checksums */
    setint(crc32_calc(gpt+1024,((gpt[512+81]<<8)|gpt[512+80])*128),gpt+512+88);
    setint(crc32_calc(gpt+512,92),gpt+512+16);
    memcpy(gpt2, gpt+512, 512);
    setint(1,gpt2+32);                          /* secondary lba */
    setint(2*FIRST_PARTITION+fat_numclu-2,gpt2+24); /* primary lba */
    setint(FIRST_PARTITION+fat_numclu,gpt2+72); /* partition lba */
    setint(0,gpt2+16);                          /* calculate with zero */
    setint(crc32_calc(gpt2,92),gpt2+16);

    fwrite(gpt, 1, FIRST_PARTITION * 512, f);
    fwrite(fs_base, 1, fs_len, f);
    fwrite(gpt + 1024, 1, (FIRST_PARTITION - 2) * 512, f);
    fwrite(gpt2, 1, 512, f);
    free(gpt);
}

/**
 * Write out ISO9660 El Torito "no emulation" Boot Catalog
 */
void setinte(int val, unsigned char *ptr) { char *v=(char*)&val; memcpy(ptr,&val,4); ptr[4]=v[3]; ptr[5]=v[2]; ptr[6]=v[1]; ptr[7]=v[0]; }
void writeetbc(FILE *f)
{
    int i;
    time_t t;
    char isodate[128];
    unsigned char *gpt = malloc(FIRST_PARTITION * 512), *iso;
    if(!gpt) { fprintf(stderr, "efidsk: unable to allocate memory\r\n"); exit(1); }
    memset(gpt, 0, FIRST_PARTITION * 512);

    t = time(NULL);
    fat_ts = gmtime(&t);
    sprintf((char*)&isodate, "%04d%02d%02d%02d%02d%02d00",
        fat_ts->tm_year+1900,fat_ts->tm_mon+1,fat_ts->tm_mday,fat_ts->tm_hour,fat_ts->tm_min,fat_ts->tm_sec);
    iso = gpt + 16*2048;
    /* 16th sector: Primary Volume Descriptor */
    iso[0]=1;   /* Header ID */
    memcpy(&iso[1], "CD001", 5);
    iso[6]=1;   /* version */
    for(i=8;i<72;i++) iso[i]=' ';
    memcpy(&iso[40], "EFI CDROM", 9);   /* Volume Identifier */
    setinte((FIRST_PARTITION*512+fs_len+2047)/2048, &iso[80]);
    iso[120]=iso[123]=1;        /* Volume Set Size */
    iso[124]=iso[127]=1;        /* Volume Sequence Number */
    iso[129]=iso[130]=8;        /* logical blocksize (0x800) */
    iso[156]=0x22;              /* root directory recordsize */
    setinte(20, &iso[158]);     /* root directory LBA */
    setinte(2048, &iso[166]);   /* root directory size */
    iso[174]=fat_ts->tm_year;   /* root directory create date */
    iso[175]=fat_ts->tm_mon+1;
    iso[176]=fat_ts->tm_mday;
    iso[177]=fat_ts->tm_hour;
    iso[178]=fat_ts->tm_min;
    iso[179]=fat_ts->tm_sec;
    iso[180]=0;                 /* timezone UTC (GMT) */
    iso[181]=2;                 /* root directory flags (0=hidden,1=directory) */
    iso[184]=1;                 /* root directory number */
    iso[188]=1;                 /* root directory filename length */
    for(i=190;i<813;i++) iso[i]=' ';    /* Volume data */
    memcpy(&iso[318], "POSIX-UEFI <HTTPS://GITLAB.COM/BZTSRC/POSIX-UEFI>", 49);
    memcpy(&iso[446], "EFIDSK", 6);
    memcpy(&iso[574], "POSIX-UEFI", 11);
    for(i=702;i<813;i++) iso[i]=' ';    /* file descriptors */
    memcpy(&iso[813], &isodate, 16);    /* volume create date */
    memcpy(&iso[830], &isodate, 16);    /* volume modify date */
    for(i=847;i<863;i++) iso[i]='0';    /* volume expiration date */
    for(i=864;i<880;i++) iso[i]='0';    /* volume shown date */
    iso[881]=1;                         /* filestructure version */
    for(i=883;i<1395;i++) iso[i]=' ';   /* file descriptors */
    /* 17th sector: Boot Record Descriptor */
    iso[2048]=0;    /* Header ID */
    memcpy(&iso[2049], "CD001", 5);
    iso[2054]=1;    /* version */
    memcpy(&iso[2055], "EL TORITO SPECIFICATION", 23);
    setinte(19, &iso[2048+71]);         /* Boot Catalog LBA */
    /* 18th sector: Volume Descritor Terminator */
    iso[4096]=0xFF; /* Header ID */
    memcpy(&iso[4097], "CD001", 5);
    iso[4102]=1;    /* version */
    /* 19th sector: Boot Catalog */
    /* --- UEFI, Validation Entry + Initial/Default Entry + Final --- */
    iso[6144]=0x91; /* Header ID, Validation Entry, Final */
    iso[6145]=0xEF; /* Platform EFI */
    iso[6176]=0x88; /* Bootable, Initial/Default Entry */
    iso[6182]=1;    /* Sector Count */
    setint(FIRST_PARTITION/4, &iso[6184]);  /* ESP Start LBA */
    /* 20th sector: Root Directory */
    /* . */
    iso[8192]=0x21 + 1;          /* recordsize */
    setinte(20, &iso[8194]);     /* LBA */
    setinte(2048, &iso[8202]);   /* size */
    iso[8210]=fat_ts->tm_year;   /* date */
    iso[8211]=fat_ts->tm_mon+1;
    iso[8212]=fat_ts->tm_mday;
    iso[8213]=fat_ts->tm_hour;
    iso[8214]=fat_ts->tm_min;
    iso[8215]=fat_ts->tm_sec;
    iso[8216]=0;                 /* timezone UTC (GMT) */
    iso[8217]=2;                 /* flags (0=hidden,1=directory) */
    iso[8220]=1;                 /* serial */
    iso[8224]=1;                 /* filename length */
    iso[8225]=0;                 /* filename '.' */
    /* .. */
    iso[8226]=0x21 + 1;          /* recordsize */
    setinte(20, &iso[8228]);     /* LBA */
    setinte(2048, &iso[8236]);   /* size */
    iso[8244]=fat_ts->tm_year;   /* date */
    iso[8245]=fat_ts->tm_mon+1;
    iso[8246]=fat_ts->tm_mday;
    iso[8247]=fat_ts->tm_hour;
    iso[8248]=fat_ts->tm_min;
    iso[8249]=fat_ts->tm_sec;
    iso[8250]=0;                 /* timezone UTC (GMT) */
    iso[8251]=2;                 /* flags (0=hidden,1=directory) */
    iso[8254]=1;                 /* serial */
    iso[8258]=1;                 /* filename length */
    iso[8259]='\001';            /* filename '..' */

    fwrite(gpt, 1, FIRST_PARTITION * 512, f);
    fwrite(fs_base, 1, fs_len, f);
    free(gpt);
}

/**
 * Main function
 */
int main(int argc, char **argv)
{
    FILE *f;
    int i, part = 0, cdrom = 0;
    char *in = NULL, *out = NULL;

    /* get random GUIDs */
    srand(time(NULL));
    i = rand(); memcpy(&((uint8_t*)&dguid)[0], &i, 4);
    i = rand(); memcpy(&((uint8_t*)&dguid)[4], &i, 4);
    i = rand(); memcpy(&((uint8_t*)&dguid)[8], &i, 4);
    i = rand(); memcpy(&((uint8_t*)&dguid)[12], &i, 4);
    i = rand(); memcpy(&((uint8_t*)&pguid)[0], &i, 4);
    i = rand(); memcpy(&((uint8_t*)&pguid)[4], &i, 4);
    i = rand(); memcpy(&((uint8_t*)&pguid)[8], &i, 4);
    i = rand(); memcpy(&((uint8_t*)&pguid)[12], &i, 4);

    /* parse command line */
    for(i = 1; i < argc && argv[i]; i++)
        if(argv[i][0] == '-') {
            switch(argv[i][1]) {
                case 'p': part++; break;
                case 'c': cdrom++; break;
                case 's': fat_numclu = atoi(argv[++i]) * 2048; break;
                default: usage(argv[0]); break;
            }
        } else
        if(!in) in = argv[i]; else
        if(!out) out = argv[i]; else
            usage(argv[0]);
    if(!out) usage(argv[0]);

    /* generate file system image */
    remove(out);
    fat_open(part ? 0 : FIRST_PARTITION);
    parsedir(in, 0);
    fat_close();

    /* write out image */
    if(fs_base) {
        f = fopen(out, "wb");
        if(!f) {
            fprintf(stderr, "efidsk: unable to write '%s'\r\n", out);
            return 2;
        }
        if(!part) {
            if(!cdrom)
                writegpt(f);
            else
                writeetbc(f);
        } else
            fwrite(fs_base, 1, fs_len, f);
        fclose(f);
        free(fs_base);
    }

    return 0;
}
