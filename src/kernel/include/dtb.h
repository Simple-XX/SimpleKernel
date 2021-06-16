
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// dtb.h for Simple-XX/SimpleKernel.

#ifndef _DTB_H_
#define _DTB_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"

// See devicetree-specification-v0.3.pdf for more info
namespace DTB {
    // devicetree-specification-v0.3.pdf#5.1
    static constexpr const uint32_t FDT_MAGIC = 0xD00DFEED;
    struct fdt_header {
        uint32_t magic;
        uint32_t totalsize;
        uint32_t off_dt_struct;
        uint32_t off_dt_strings;
        uint32_t off_mem_rsvmap;
        uint32_t version;
        uint32_t last_comp_version;
        uint32_t boot_cpuid_phys;
        uint32_t size_dt_strings;
        uint32_t size_dt_struct;
    };
    // devicetree-specification-v0.3.pdf#5.3.2
    struct fdt_reserve_entry {
        uint64_t address;
        uint64_t size;
    };

    static constexpr const uint64_t FDT_BEGIN_NODE = 0x1;
    static constexpr const uint64_t FDT_END_NODE   = 0x2;
    static constexpr const uint64_t FDT_PROP       = 0x3;
    static constexpr const uint64_t FDT_NOP        = 0x4;
    static constexpr const uint64_t FDT_END        = 0x9;

    struct fdt_node_header {
        uint32_t tag;
        /* node名称，作为额外数据以'\0'结尾的字符串形式存储在structure block
         * 32-bits对齐，不够的位用0x0补齐
         */
        char name[0];
    };

    struct fdt_property {
        uint32_t tag;
        /* 表示property value的长度 */
        uint32_t len;
        /* property的名称存放在string block区域，nameoff表示其在string
         * block的偏移 */
        uint32_t nameoff;
        /* property value值，作为额外数据以'\0'结尾的字符串形式存储structure
         * block 32-bits对齐，不够的位用0x0补齐
         */
        char data[0];
    };

    // TODO: 仅传递 dtb 地址参数
    void dtb_init(uint32_t, uint64_t _addr);
};

#ifdef __cplusplus
}
#endif

#endif /* _DTB_H_ */
