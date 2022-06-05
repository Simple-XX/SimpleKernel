# SimpleKernel 物理内存管理

物理内存是内核的基础模块，我们会使用一个分配器对它进行统一管理。

分配器的基类是 `ALLOCATOR` 类，这是一个抽象类，必须被继承后才能使用。

由抽象类派生出的具体类是一个分配算法，使用了首次匹配算法(First Fit)。

分配器以 4KB 为最小管理单位，每次分配或回收都是它的整数倍。

此外，为了区分内核权限的内存和用户权限的内存，在 PMM 类的实现中有两个指向 FIRSTFIT 类的 ALLOCATOR 类指针，这允许我们指定要操作的内存位置。

```c++
/**
 * @brief 内存分配器抽象类
 */
class ALLOCATOR {
private:
protected:
    /// 分配器名称
    const char *name;
    /// 当前管理的内存区域地址
    uintptr_t allocator_start_addr;
    /// 当前管理的内存区域长度
    size_t allocator_length;
    /// 当前管理的内存区域空闲长度
    size_t allocator_free_count;
    /// 当前管理的内存区域已使用长度
    size_t allocator_used_count;

public:
    /**
     * @brief 构造内存分配器
     * @param  _name           分配器名
     * @param  _addr           要管理的内存开始地址
     * @param  _len            要管理的内存长度，单位以具体实现为准
     */
    ALLOCATOR(const char *_name, uintptr_t _addr, size_t _len);

    virtual ~ALLOCATOR(void) = 0;

    /**
     * @brief 分配 _len 页
     * @param  _len            页数
     * @return uintptr_t       分配到的地址
     */
    virtual uintptr_t alloc(size_t _len) = 0;

    /**
     * @brief 在指定地址分配 _len 长度
     * @param  _addr           指定的地址
     * @param  _len            长度
     * @return true            成功
     * @return false           失败
     */
    virtual bool alloc(uintptr_t _addr, size_t _len) = 0;

    /**
     * @brief 释放 _len 长度
     * @param  _addr           地址
     * @param  _len            长度
     */
    virtual void free(uintptr_t _addr, size_t _len) = 0;

    /**
     * @brief 已使用数量
     * @return size_t          数量
     */
    virtual size_t get_used_count(void) const = 0;

    /**
     * @brief 空闲数量
     * @return size_t          数量
     */
    virtual size_t get_free_count(void) const = 0;
};
```

具体到不同架构，物理内存管理会从 `BOOT_INFO` 获取信息进行初始化

```c++
bool PMM::init(void) {
    // 获取物理内存信息
    resource_t mem_info = BOOT_INFO::get_memory();
    // 设置物理地址的起点与长度
    start  = mem_info.mem.addr;
    length = mem_info.mem.len;
    // 计算页数
    total_pages = length / COMMON::PAGE_SIZE;
    // 内核空间地址开始
    kernel_space_start = COMMON::KERNEL_START_ADDR;
    // 长度手动指定
    kernel_space_length = COMMON::KERNEL_SPACE_SIZE;
    // 非内核空间在内核空间结束后
    non_kernel_space_start =
        COMMON::KERNEL_START_ADDR + COMMON::KERNEL_SPACE_SIZE;
    // 长度为总长度减去内核长度
    non_kernel_space_length = length - kernel_space_length;

    // 创建分配器
    // 内核空间
    static FIRSTFIT first_fit_allocator_kernel(
        "First Fit Allocator(kernel space)", kernel_space_start,
        kernel_space_length / COMMON::PAGE_SIZE);
    kernel_space_allocator = (ALLOCATOR *)&first_fit_allocator_kernel;
    // 非内核空间
    static FIRSTFIT first_fit_allocator(
        "First Fit Allocator", non_kernel_space_start,
        non_kernel_space_length / COMMON::PAGE_SIZE);
    allocator = (ALLOCATOR *)&first_fit_allocator;

    // 内核实际占用页数 这里也算了 0～1M 的 reserved 内存
    size_t kernel_pages =
        (COMMON::ALIGN(COMMON::KERNEL_END_ADDR, COMMON::PAGE_SIZE) -
         COMMON::ALIGN(COMMON::KERNEL_START_ADDR, COMMON::PAGE_SIZE)) /
        COMMON::PAGE_SIZE;
    // 将内核已使用部分划分出来
    if (alloc_pages_kernel(COMMON::KERNEL_START_ADDR, kernel_pages) == true) {
        // 将 multiboot2/dtb 信息移动到内核空间
        move_boot_info();
        info("pmm init.\n");
        return true;
    }
    else {
        assert(0);
        return false;
    }
}
```

- 相关代码

    ./src/include/mem/allocator.h

    ./src/include/mem/firstfit.h
    
    ./src/include/mem/pmm.h
    
    ./src/kernel/allocator.cpp
    
    ./src/kernel/firstfit.cpp
    
    ./src/kernel/pmm.cpp


更多细节请查看注释。
