# SimpleKernel 解析启动信息

为了支持接下来的工作，我们需要获取电脑的硬件信息，这些信息可以通过引导程序传递，我们只要做解析即可。

同样，有一个 `BOOT_INFO` 名称空间来统一管理这些接口。

- 相关代码

    ./src/include/boot_info.h

    ./src/include/resource.h

## IA32

在启动时我们使用了 grub2+multiboot2 的方式进行引导，在同时还会为我们提供所需要的硬件信息。

grub2 会通过 `eax` 和 `ebx` 两个寄存器将信息传递给我们，只需要在启动阶段保存它们，就能在后面进行解析，获取硬件信息。

multiboot2 使用迭代方法寻找所需信息，函数 `multiboot2_iter(bool (*_fun)(const iter_data_t *, void *), void *_data)` 提供了这一接口。


- 相关代码

    ./src/arch/ia32/i386/boot/boot.S

    ./src/drv/multiboot2/include/multiboot2.h

    ./src/drv/multiboot2/multiboot2.cpp

更多细节请查看注释。

## RISCV

设备树(device tree)是嵌入式设备常用的硬件配置信息传递方式，在启动时，opensbi 会将设备树信息的地址传递给内核，

```assembly
.section .init
.globl _start
.type _start, @function
.extern kernel_main
.extern cpp_init
.extern boot_info_addr
.extern dtb_init_hart
_start:
    // 保存 sbi 传递的参数
    // 将 a0 的值传递给 dtb_init_hart
    sw a0, dtb_init_hart, t0
    // 将 a1 的值传递给 boot_info_addr
    sw a1, boot_info_addr, t0
    // 设置栈地址
    la sp, stack_top
    // 初始化 C++
    call cpp_init
    // 跳转到 C 代码执行
    call kernel_main
```

内核会在 `BOOT_INFO` 中进行初始化，解析其内容。

```c++
namespace BOOT_INFO {
bool init(void) {
    auto res = DTB::get_instance().dtb_init();
    if (inited == false) {
        inited = true;
        info("BOOT_INFO init.\n");
    }
    else {
        info("BOOT_INFO reinit.\n");
    }
    return res;
}
}; // namespace BOOT_INFO

bool DTB::dtb_init(void) {
    // 头信息
    dtb_info.header = (fdt_header_t *)BOOT_INFO::boot_info_addr;
    // 魔数
    assert(be32toh(dtb_info.header->magic) == FDT_MAGIC);
    // 版本
    assert(be32toh(dtb_info.header->version) == FDT_VERSION);
    // 设置大小
    BOOT_INFO::boot_info_size = be32toh(dtb_info.header->totalsize);
    // 内存保留区
    dtb_info.reserved =
        (fdt_reserve_entry_t *)(BOOT_INFO::boot_info_addr +
                                be32toh(dtb_info.header->off_mem_rsvmap));
    // 数据区
    dtb_info.data =
        BOOT_INFO::boot_info_addr + be32toh(dtb_info.header->off_dt_struct);
    // 字符区
    dtb_info.str =
        BOOT_INFO::boot_info_addr + be32toh(dtb_info.header->off_dt_strings);
    // 检查保留内存
    dtb_mem_reserved();
    // 初始化 map
    bzero(nodes, sizeof(nodes));
    bzero(phandle_map, sizeof(phandle_map));
    // 初始化节点的基本信息
    dtb_iter(DT_ITER_BEGIN_NODE | DT_ITER_END_NODE | DT_ITER_PROP, dtb_init_cb,
             nullptr);
    // 中断信息初始化，因为需要查找 phandle，所以在基本信息初始化完成后进行
    dtb_iter(DT_ITER_PROP, dtb_init_interrupt_cb, nullptr);
// #define DEBUG
#ifdef DEBUG
    // 输出所有信息
    for (size_t i = 0; i < nodes[0].count; i++) {
        std::cout << nodes[i].path << ": " << std::endl;
        for (size_t j = 0; j < nodes[i].prop_count; j++) {
            printf("%s: ", nodes[i].props[j].name);
            for (size_t k = 0; k < nodes[i].props[j].len / 4; k++) {
                printf("0x%X ",
                       be32toh(((uint32_t *)nodes[i].props[j].addr)[k]));
            }
            printf("\n");
        }
    }
#undef DEBUG
#endif
    return true;
}
```

`void DTB::dtb_iter(uint8_t _cb_flags, bool (*_cb)(const iter_data_t *, void *), void *  _data)` 是解析的核心函数

```c++
void DTB::dtb_iter(uint8_t _cb_flags, bool (*_cb)(const iter_data_t *, void *),
                   void *_data, uintptr_t _addr) {
    // 迭代变量
    iter_data_t iter;
    // 路径深度
    iter.path.len = 0;
    // 数据地址
    iter.addr = (uint32_t *)_addr;
    // 节点索引
    iter.nodes_idx = 0;
    // 开始 flag
    bool begin = true;

    while (1) {
        //
        iter.type = be32toh(iter.addr[0]);
        switch (iter.type) {
            case FDT_NOP: {
                // 跳过 type
                iter.addr++;
                break;
            }
            case FDT_BEGIN_NODE: {
                // 第 len 深底的名称
                iter.path.path[iter.path.len] = (char *)(iter.addr + 1);
                // 深度+1
                iter.path.len++;
                iter.nodes_idx = begin ? 0 : (iter.nodes_idx + 1);
                begin          = false;
                if (_cb_flags & DT_ITER_BEGIN_NODE) {
                    if (_cb(&iter, _data)) {
                        return;
                    }
                }
                // 跳过 type
                iter.addr++;
                // 跳过 name
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
                // 这一级结束了，所以 -1
                iter.path.len--;
                // 跳过 type
                iter.addr++;
                break;
            }
            case FDT_PROP: {
                iter.prop_len  = be32toh(iter.addr[1]);
                iter.prop_name = (char *)(dtb_info.str + be32toh(iter.addr[2]));
                iter.prop_addr = iter.addr + 3;
                if (_cb_flags & DT_ITER_PROP) {
                    if (_cb(&iter, _data)) {
                        return;
                    }
                }
                iter.prop_name = nullptr;
                iter.prop_addr = nullptr;
                // 跳过 type
                iter.addr++;
                // 跳过 len
                iter.addr++;
                // 跳过 nameoff
                iter.addr++;
                // 跳过 data，并进行对齐
                iter.addr += COMMON::ALIGN(iter.prop_len, 4) / 4;
                iter.prop_len = 0;
                break;
            }
            case FDT_END: {
                return;
            }
            default: {
                printf("unrecognized token 0x%X\n", iter.type);
                return;
            }
        }
    }
    return;
}
```

- 相关代码

    ./src/arch/riscv64/boot/boot.S

    ./src/drv/dtb/include/dtb.h

    ./src/drv/dtb/dtb.cpp

更多细节请查看注释。

## 相关文档

multiboot2 规范：https://www.gnu.org/software/grub/manual/multiboot2/multiboot.html

opensbi：https://github.com/riscv/opensbi

device-tree：https://github.com/devicetree-org/devicetree-specification

dtb解析0： https://e-mailky.github.io/2016-12-06-dts-introduce

dtb解析1： https://e-mailky.github.io/2019-01-14-dts-1

dtb解析2： https://e-mailky.github.io/2019-01-14-dts-2

dtb解析3： https://e-mailky.github.io/2019-01-14-dts-3

