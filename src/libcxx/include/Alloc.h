
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
// Based on https://github.com/zouxiaohang/TinySTL
// Alloc.h for Simple-XX/SimpleKernel.

#ifndef _ALLOC_H_
#define _ALLOC_H_

#include "stdlib.h"

class alloc {
private:
    enum EAlign { ALIGN = 8 };
    enum EMaxBytes { MAXBYTES = 128 };
    enum ENFreeLists { NFREELISTS = (EMaxBytes::MAXBYTES / EAlign::ALIGN) };
    enum ENObjs { NOBJS = 20 };

    union obj {
        union obj *next;
        char       client[1];
    };

    static obj *free_list[ENFreeLists::NFREELISTS];

    static char * start_free;
    static char * end_free;
    static size_t heap_size;

    static size_t ROUND_UP(size_t bytes);

    static size_t FREELIST_INDEX(size_t bytes);

    static void *refill(size_t n);

    static char *chunk_alloc(size_t size, size_t &nobjs);

public:
    static void *allocate(size_t bytes);
    static void  deallocate(void *ptr, size_t bytes);
    static void *reallocate(void *ptr, size_t old_sz, size_t new_sz);
};

#endif /* _ALLOC_H_ */