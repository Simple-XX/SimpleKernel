
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// dtb.cpp for Simple-XX/SimpleKernel.

#include "stdint.h"
#include "stdio.h"
#include "endian.h"
#include "assert.h"
#include "common.h"
#include "boot_info.h"
#include "resource.h"
#include "dtb.h"

// 地址
uintptr_t boot_info_addr;
// 长度
size_t BOOT_INFO::boot_info_size;
// 启动核
size_t dtb_init_hart;

DTB::node_t        DTB::nodes[MAX_NODES];
size_t             DTB::node_t::count = 0;
DTB::phandle_map_t DTB::phandle_map[MAX_NODES];
size_t             DTB::phandle_map_t::count = 0;

DTB::node_t *DTB::get_phandle(uint32_t _phandle) {
    uint32_t i;
    for (i = 0; i < phandle_map[0].count; i++) {
        if (phandle_map[i].phandle == _phandle) {
            return phandle_map[i].node;
        }
    }
    return nullptr;
}

DTB::dt_fmt_t DTB::get_fmt(const char *_prop_name) {
    dt_fmt_t res = FMT_UNKNOWN;
    for (size_t i = 0; i < sizeof(props) / sizeof(dt_prop_fmt_t); i++) {
        if (strcmp(_prop_name, props[i].prop_name) == 0) {
            res = props[i].fmt;
        }
    }
    return res;
}

void DTB::print_attr_propenc(const iter_data_t *_iter,
                             dt_propenc_field_t _fields[], size_t _fcnt) {
    uint32_t  entry_size = 0, phandle, remain = _iter->prop_len;
    uint32_t *reg = _iter->prop_addr;
    node_t *  ref;

    printf("%s: ", _iter->prop_name);

    for (size_t i = 0; i < _fcnt; i++) {
        entry_size += 4 * _fields[i].cells;
        if (_fields[i].phandle && _fields[i].cells != 1) {
            printf("malformed prop-encoded array, has phandle with "
                   "len != 1\n");
        }
    }

    printf("(len=%u/%u) ", _iter->prop_len, entry_size);

    if (_iter->prop_len % entry_size != 0) {
        printf("malformed prop-encoded array, len=%u, entry len=%u\n",
               _iter->prop_len, entry_size);
        return;
    }

    while (remain > 0) {
        std::cout << "<";
        for (size_t i = 0; i < _fcnt; i++) {
            if (_fields[i].phandle) {
                phandle = be32toh(*reg);
                ref     = get_phandle(phandle);
                if (ref) {
                    printf("<phandle &%s>", ref->name);
                }
                else {
                    printf("<phandle 0x%x>", phandle);
                }
                reg++;
                remain -= 4;
            }
            else {
                std::cout << "0x";
                for (size_t j = 0; j < _fields[i].cells; j++) {
                    printf("%x ", be32toh(*reg));
                    reg++;
                    remain -= 4;
                }
            }
            std::cout << "| ";
        }
        std::cout << "> ";
    }
    std::cout << std::endl;
    return;
}

void DTB::dtb_mem_reserved(void) {
    fdt_reserve_entry_t *entry;

    printf("+ reserved memory regions at 0x%x\n", info.reserved);
    for (entry = info.reserved; entry->addr_le || entry->size_le; entry++) {
        printf("  addr=0x%x, size=0x%x\n", be32toh(entry->addr_le),
               be32toh(entry->size_le));
    }
    return;
}

/**
 * Iterate over the device tree! This is a configurable function which
 executes
 * your callback at whichever point in the iteration, and can terminate at
 * whatever point you want.
 *
 * Args:
 *   _cb_flags: Specify what tokens you'd like your callback to get called
 at.
 *     Uses the bit constants DTB_ITER_*.
 *   _cb: A function which is called at particular tokens during the
 iteration.
 *     If this returns true, the iteration returns early. The first argument
 to
 *     this callback is a iter_data_t *, which MUST NOT be modified. See
 the
 *     difinition of iter_data_t for all the juicy details on what it
 *     contains. The second argument to the callback is an arbitrary "_data"
 *     which the caller defines.
 *   _data: Some _data to give to the callback
 *
 * Returns: void!
 */
void DTB::dtb_iter(uint8_t _cb_flags, bool (*_cb)(const iter_data_t *, void *),
                   void *  _data) {
    char *str;

    iter_data_t iter;
    iter.path.len  = 0;
    iter.addr      = (uint32_t *)info.data;
    iter.nodes_idx = 0;
    bool begin     = true;

    while (1) {
        iter.type = be32toh(iter.addr[0]);
        switch (iter.type) {
            case FDT_NOP: {
                iter.addr++;
                break;
            }
            case FDT_BEGIN_NODE: {
                str                           = (char *)(iter.addr + 1);
                iter.path.path[iter.path.len] = str;
                iter.path.len++;
                iter.nodes_idx = begin ? 0 : (iter.nodes_idx + 1);
                begin          = false;
                if (_cb_flags & DT_ITER_BEGIN_NODE) {
                    if (_cb(&iter, _data)) {
                        return;
                    }
                }
                iter.addr++;
                iter.addr +=
                    COMMON::ALIGN(strlen((char *)iter.addr) + 1, 4) / 4;
                break;
            }
            case FDT_END_NODE: {
                if (_cb_flags & DT_ITER_END_NODE) {
                    if (_cb(&iter, _data)) {
                        return;
                    }
                }
                iter.path.len--;
                iter.addr++;
                break;
            }
            case FDT_PROP: {
                iter.prop_name = (char *)(info.str + be32toh(iter.addr[2]));
                iter.prop_len  = be32toh(iter.addr[1]);
                iter.prop_addr = iter.addr + 3;
                if (_cb_flags & DT_ITER_PROP) {
                    if (_cb(&iter, _data)) {
                        return;
                    }
                }
                iter.prop_name = nullptr;
                iter.prop_addr = nullptr;
                iter.addr++;
                iter.addr++;
                iter.addr++;
                iter.addr += COMMON::ALIGN(iter.prop_len, 4) / 4;
                iter.prop_len = 0;
                break;
            }
            case FDT_END: {
                return;
            }
            default: {
                printf("unrecognized token 0x%x\n", iter.type);
                return;
            }
        }
    }
    return;
}

DTB::dtb_info_t DTB::info;

/*
 * This callback constructs tracking information about each node.
 */
bool DTB::dtb_init_cb(const iter_data_t *_iter, void *) {
    size_t nodeidx = _iter->nodes_idx;
    switch (_iter->type) {
        case FDT_BEGIN_NODE: {
            nodes[nodeidx].name  = _iter->path.path[_iter->path.len - 1];
            nodes[nodeidx].tok   = _iter->addr;
            nodes[nodeidx].depth = _iter->path.len;
            nodes[nodeidx].address_cells   = 2;
            nodes[nodeidx].size_cells      = 1;
            nodes[nodeidx].interrupt_cells = 0;
            nodes[nodeidx].phandle         = 0;
            if (nodeidx != 0) {
                size_t i = nodeidx - 1;
                while (nodes[i].depth != nodes[nodeidx].depth - 1) {
                    i--;
                }
                nodes[nodeidx].parent = &nodes[i];
            }
            else {
                nodes[nodeidx].parent = &nodes[nodeidx];
            }
            nodes[nodeidx].interrupt_parent = nodes[nodeidx].parent;
            break;
        }
        case FDT_PROP: {
            if (strcmp(_iter->prop_name, "#address-cells") == 0) {
                nodes[nodeidx].address_cells = be32toh(_iter->addr[3]);
            }
            else if (strcmp(_iter->prop_name, "#size-cells") == 0) {
                nodes[nodeidx].size_cells = be32toh(_iter->addr[3]);
            }
            else if (strcmp(_iter->prop_name, "#interrupt-cells") == 0) {
                nodes[nodeidx].interrupt_cells = be32toh(_iter->addr[3]);
            }
            else if (strcmp(_iter->prop_name, "phandle") == 0) {
                nodes[nodeidx].phandle = be32toh(_iter->addr[3]);
                phandle_map[phandle_map[0].count].phandle =
                    nodes[nodeidx].phandle;
                phandle_map[phandle_map[0].count].node = &nodes[nodeidx];
                phandle_map[0].count++;
            }
// #define DEBUG
#ifdef DEBUG
            std::cout << _iter->path << std::endl;
            printf("iter.prop_len: 0x%X\n", _iter->prop_len);
            for (size_t i = 0; i < _iter->prop_len / 4; i++) {
                uint32_t *tmp = _iter->prop_addr + i;
                printf("0x%X ", be32toh(*tmp));
            }
            printf("\n");
#undef DEBUG
#endif
            break;
        }
        case FDT_END_NODE: {
            nodes[0].count = nodeidx + 1;
            break;
        }
    }
    return false;
}

bool DTB::dtb_init_interrupt_cb(const iter_data_t *_iter, void *) {
    unsigned short nodeidx = _iter->nodes_idx;
    uint32_t       phandle;
    node_t *       parent;
    if (strcmp(_iter->prop_name, "interrupt-parent") == 0) {
        phandle = be32toh(_iter->addr[3]);
        parent  = get_phandle(phandle);
        if (parent) {
            nodes[nodeidx].interrupt_parent = parent;
        }
        else {
            std::cout << "dtb: error: for node " << _iter->path
                      << ", interrupt-parent specifies unknown phandle: ";
            printf("0x%x\n", phandle);
        }
    }
    return false;
}

bool DTB::ttt(const iter_data_t *_iter, void *) {
    std::cout << *_iter << std::endl;
    return false;
}

void DTB::dtb_init(void) {
    info.header = (fdt_header_t *)boot_info_addr;
    assert(be32toh(info.header->magic) == FDT_MAGIC);
    assert(be32toh(info.header->version) == FDT_VERSION);

    info.reserved =
        (fdt_reserve_entry_t *)(boot_info_addr +
                                be32toh(info.header->off_mem_rsvmap));
    info.data = boot_info_addr + be32toh(info.header->off_dt_struct);
    info.str  = boot_info_addr + be32toh(info.header->off_dt_strings);
    bzero(nodes, sizeof(nodes));
    bzero(phandle_map, sizeof(phandle_map));
    dtb_iter(DT_ITER_BEGIN_NODE | DT_ITER_END_NODE | DT_ITER_PROP, dtb_init_cb,
             nullptr);
    dtb_iter(DT_ITER_PROP, dtb_init_interrupt_cb, nullptr);
    dtb_iter(DT_ITER_PROP, ttt, nullptr);
    return;
}

std::ostream &operator<<(std::ostream &_os, const DTB::iter_data_t &_iter) {
    switch (DTB::get_fmt(_iter.prop_name)) {
        case DTB::FMT_UNKNOWN: {
            printf("%s: (unknown format, len=0x%X)", _iter.prop_name,
                   _iter.prop_len);
            break;
        }
        case DTB::FMT_EMPTY: {
            _os << _iter.prop_name << ": (empty)";
            break;
        }
        case DTB::FMT_U32: {
            printf("%s: 0x%X", _iter.prop_name,
                   be32toh(*(uint32_t *)_iter.prop_addr));
            break;
        }
        case DTB::FMT_U64: {
            printf("%s: 0x%p", _iter.prop_name,
                   be32toh(*(uint64_t *)_iter.prop_addr));
            break;
        }
        case DTB::FMT_STRING: {
            _os << _iter.prop_name << ": " << (char *)_iter.prop_addr;
            break;
        }
        case DTB::FMT_PHANDLE: {
            uint32_t     phandle = be32toh(_iter.addr[3]);
            DTB::node_t *ref     = DTB::get_phandle(phandle);
            if (ref != nullptr) {
                _os << _iter.prop_name << ": "
                    << "<phandle &" << ref->name << ">";
            }
            else {
                printf("%s: <phandle 0x%x>\n", _iter.prop_name, phandle);
            }
            break;
        }
        case DTB::FMT_STRINGLIST: {
            size_t len = 0;
            char * str = (char *)_iter.prop_addr;
            _os << _iter.prop_name << ": [";
            while (len < _iter.prop_len) {
                _os << "\"" << str << "\"";
                len += strlen(str) + 1;
                str = (char *)((uint8_t *)_iter.prop_addr + len);
            }
            _os << "]";
            break;
        }
        case DTB::FMT_REG: {
            uint8_t                 nodeidx  = _iter.nodes_idx;
            DTB::dt_propenc_field_t fields[] = {
                {.cells   = DTB::nodes[nodeidx].parent->address_cells,
                 .phandle = false},
                {.cells   = DTB::nodes[nodeidx].parent->size_cells,
                 .phandle = false},
            };
            DTB::print_attr_propenc(&_iter, fields,
                                    sizeof(fields) /
                                        sizeof(DTB::dt_propenc_field_t));
            break;
        }
        case DTB::FMT_RANGES: {
            uint8_t                 nodeidx  = _iter.nodes_idx;
            DTB::dt_propenc_field_t fields[] = {
                {.cells = DTB::nodes[nodeidx].address_cells, .phandle = false},
                {.cells   = DTB::nodes[nodeidx].parent->address_cells,
                 .phandle = false},
                {.cells = DTB::nodes[nodeidx].size_cells, .phandle = false},
            };
            DTB::print_attr_propenc(&_iter, fields,
                                    sizeof(fields) /
                                        sizeof(DTB::dt_propenc_field_t));
            break;
        }
        case DTB::FMT_INTERRUPTS: {
            uint8_t      nodeidx         = _iter.nodes_idx;
            DTB::node_t *node            = DTB::nodes[nodeidx].parent;
            uint32_t     interrupt_cells = 0;
            _os << "name: " << node->name << std::endl;
            while ((node != nullptr) && node->interrupt_cells == 0) {
                _os << "name: " << node->name << std::endl;
                node = node->interrupt_parent;
                _os << "name: " << node->name << std::endl;
            }
            if (node != nullptr) {
                interrupt_cells = node->interrupt_cells;
            }

            DTB::dt_propenc_field_t fields[] = {
                {.cells = interrupt_cells, .phandle = false},
            };
            DTB::print_attr_propenc(&_iter, fields,
                                    sizeof(fields) /
                                        sizeof(DTB::dt_propenc_field_t));
            break;
        }
        default: {
            assert(0);
            break;
        }
    }
    return _os;
}
