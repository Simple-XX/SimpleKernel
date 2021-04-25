
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// fat32.cpp for Simple-XX/SimpleKernel.

#include "fat32.h"
#include "stdlib.h"

FAT32::FAT32(void) {
    name = "fat32";
    return;
}

FAT32::~FAT32(void) {
    return;
}

int FAT32::get_sb(void) {
    return 0;
}

int FAT32::kill_sb(void) {
    return 0;
}

int FAT32::read_super(void) {
    return 0;
}

int FAT32::open(void) {
    return 0;
}

int FAT32::close(void) {
    return 0;
}

int FAT32::seek(void) {
    return 0;
}
