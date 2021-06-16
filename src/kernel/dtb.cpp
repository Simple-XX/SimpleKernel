
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// dtb.cpp for Simple-XX/SimpleKernel.

#ifdef __cplusplus
extern "C" {
#endif

#include "string.h"
#include "stdio.h"
#include "assert.h"
#include "endian.h"
#include "dtb.h"

namespace DTB {
    void dtb_init(uint32_t, uint64_t _addr) {
        assert(_addr == 0x82200000);
        fdt_header header = *((fdt_header *)_addr);
        // dtb 为大端，可能需要转换
        assert(header.magic == be32toh(FDT_MAGIC));
        printf("size: 0x%X, ", be32toh(header.totalsize));
        printf("ver: 0x%X, ", be32toh(header.version));
        printf("last: 0x%X\n", be32toh(header.last_comp_version));

        return;
    }
};

#ifdef __cplusplus
}
#endif
