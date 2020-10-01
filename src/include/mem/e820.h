
// This file is a part of SimpleXX/SimpleKernel
// (https://github.com/SimpleXX/SimpleKernel).
//
// e820.h for SimpleXX/SimpleKernel.

#ifndef _E820_H_
#define _E820_H_

#define E820_MAX 8
#define E820_RAM 1
#define E820_RESERVED 2
#define E820_ACPI 3
#define E820_NVS 4
#define E820_UNUSABLE 5

typedef struct e820entry {
    uint64_t addr;
    uint64_t length;
    uint32_t type;
} __attribute__((packed)) e820entry_t;

typedef struct e820map {
    uint32_t    nr_map;
    e820entry_t map[E820_MAX];
} e820map_t;

#endif /* _E820_H_ */
