
/**
 * @file test.cpp
 * @brief 测试
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2023-03-31
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2023-03-31<td>Zone.N<td>迁移到 doxygen
 * </table>
 */

#include "assert.h"
#include "common.h"
#include "ff.h"
#include "heap.h"
#include "kernel.h"
#include "pmm.h"
#include "stdio.h"
#include "stdlib.h"
#include "vfs.h"
#include "vmm.h"

int32_t test_pmm(void) {
    // 保存现有 pmm 空闲页数量
    size_t free_pages = PMM::get_instance().get_free_pages_count();
    // 计算内核实际占用页数
    auto   kernel_pages
      = (COMMON::ALIGN(COMMON::KERNEL_END_ADDR, COMMON::PAGE_SIZE)
         - COMMON::ALIGN(COMMON::KERNEL_START_ADDR, COMMON::PAGE_SIZE))
      / COMMON::PAGE_SIZE;
    // 再加上启动信息使用的页，一般为一页
    kernel_pages++;
    // TODO: 替换宏
    // riscv 的 dtb 占用空间比较大，两页
#ifdef __riscv
    kernel_pages++;
#endif
    // 空闲页数应该等于物理内存大小-内核使用
    assert(free_pages
           == (PMM::get_instance().get_pmm_length() / COMMON::PAGE_SIZE)
                - kernel_pages);
    // 获取已使用页数
    size_t used_pages = PMM::get_instance().get_used_pages_count();
    // 已使用页数应该等于内核使用页数
    assert(used_pages == kernel_pages);
    // 分配
    auto addr1 = PMM::get_instance().alloc_pages(2);
    // 已使用应该会更新
    assert(PMM::get_instance().get_used_pages_count() == 2 + kernel_pages);
    // 同上
    auto addr2 = PMM::get_instance().alloc_pages(3);
    assert(PMM::get_instance().get_used_pages_count() == 5 + kernel_pages);
    // 同上
    auto addr3 = PMM::get_instance().alloc_pages(100);
    assert(PMM::get_instance().get_used_pages_count() == 105 + kernel_pages);
    // 同上
    auto addr4 = PMM::get_instance().alloc_pages(100);
    assert(PMM::get_instance().get_used_pages_count() == 205 + kernel_pages);
    // 分配超过限度的内存，应该返回 nullptr
    auto addr5 = PMM::get_instance().alloc_pages(0xFFFFFFFF);
    assert(addr5 == 0);
    // 全部释放
    PMM::get_instance().free_pages(addr1, 2);
    PMM::get_instance().free_pages(addr2, 3);
    PMM::get_instance().free_pages(addr3, 100);
    PMM::get_instance().free_pages(addr4, 100);
    // 现在内存使用情况应该与此函数开始时相同
    assert(PMM::get_instance().get_free_pages_count() == free_pages);
    // 下面测试内核空间物理内存分配
    // 已使用页数应该等于内核使用页数
    assert(used_pages == kernel_pages);
    // 分配
    addr1 = PMM::get_instance().alloc_pages_kernel(2);
    // 已使用应该会更新
    assert(PMM::get_instance().get_used_pages_count() == 2 + kernel_pages);
    // 同上
    addr2 = PMM::get_instance().alloc_pages_kernel(3);
    assert(PMM::get_instance().get_used_pages_count() == 5 + kernel_pages);
    // 同上
    addr3 = PMM::get_instance().alloc_pages_kernel(100);
    assert(PMM::get_instance().get_used_pages_count() == 105 + kernel_pages);
    // 同上
    addr4 = PMM::get_instance().alloc_pages_kernel(100);
    assert(PMM::get_instance().get_used_pages_count() == 205 + kernel_pages);
    // 分配超过限度的内存，应该返回 nullptr
    addr5 = PMM::get_instance().alloc_pages_kernel(0xFFFFFFFF);
    assert(addr5 == 0);
    // 全部释放
    PMM::get_instance().free_pages(addr1, 2);
    PMM::get_instance().free_pages(addr2, 3);
    PMM::get_instance().free_pages(addr3, 100);
    PMM::get_instance().free_pages(addr4, 100);
    // 现在内存使用情况应该与此函数开始时相同
    assert(PMM::get_instance().get_free_pages_count() == free_pages);
    info("pmm test done.\n");
    return 0;
}

/// @note riscv 内核模式下无法测试 VMM_PAGE_USER，默认状态下 S/U
/// 模式的页无法互相访问
/// @see
/// https://five-embeddev.com/riscv-isa-manual/latest/supervisor.html#sec:translation
int32_t test_vmm(void) {
    uintptr_t addr = 0;
    // 首先确认内核空间被映射了
    assert(VMM::get_instance().get_pgd() != nullptr);
    assert(VMM::get_instance().get_mmap(VMM::get_instance().get_pgd(),
                                        (COMMON::KERNEL_START_ADDR + 0x1000),
                                        &addr)
           == 1);
    assert(addr == COMMON::KERNEL_START_ADDR + 0x1000);
    addr = 0;
    assert(VMM::get_instance().get_mmap(VMM::get_instance().get_pgd(),
                                        COMMON::KERNEL_START_ADDR
                                          + VMM_KERNEL_SPACE_SIZE - 1,
                                        &addr)
           == 1);
    assert(addr
           == ((COMMON::KERNEL_START_ADDR + VMM_KERNEL_SPACE_SIZE - 1)
               & COMMON::PAGE_MASK));
    addr = 0;
    assert(VMM::get_instance().get_mmap(
             VMM::get_instance().get_pgd(),
             (COMMON::ALIGN(COMMON::KERNEL_START_ADDR, 4 * COMMON::KB)
              + VMM_KERNEL_SPACE_SIZE),
             &addr)
           == 0);
    assert(addr == 0);
    addr = 0;
    assert(VMM::get_instance().get_mmap(
             VMM::get_instance().get_pgd(),
             (COMMON::ALIGN(COMMON::KERNEL_START_ADDR, 4 * COMMON::KB)
              + VMM_KERNEL_SPACE_SIZE + 0x1024),
             0)
           == 0);
    // 测试映射与取消映射
    addr         = 0;
    // 准备映射的虚拟地址 3GB 处
    uintptr_t va = 0xC0000000;
    // 分配要映射的物理地址
    uintptr_t pa = PMM::get_instance().alloc_page_kernel();
    // 确定一块未映射的内存
    assert(VMM::get_instance().get_mmap(VMM::get_instance().get_pgd(), va,
                                        nullptr)
           == 0);
    // 映射
    VMM::get_instance().mmap(VMM::get_instance().get_pgd(), va, pa,
                             VMM_PAGE_READABLE | VMM_PAGE_WRITABLE);
    assert(VMM::get_instance().get_mmap(VMM::get_instance().get_pgd(), va,
                                        &addr)
           == 1);
    assert(addr == pa);
    // 写测试
    *(uintptr_t*)va = 0xCD;
    // 取消映射
    VMM::get_instance().unmmap(VMM::get_instance().get_pgd(), va);
    assert(VMM::get_instance().get_mmap(VMM::get_instance().get_pgd(), va,
                                        &addr)
           == 0);
    assert(addr == 0);
    // 回收物理地址
    PMM::get_instance().free_page(pa);
    info("vmm test done.\n");
    return 0;
}

// TODO: 更多测试
int test_heap(void) {
    // 根据字长不同 CHUNK_SIZE 是不一样的
    size_t chunk_size = 0;
    if (sizeof(void*) == 4) {
        chunk_size = 0x10;
    }
    else if (sizeof(void*) == 8) {
        chunk_size = 0x20;
    }
    void* addr1 = nullptr;
    void* addr2 = nullptr;
    void* addr3 = nullptr;
    void* addr4 = nullptr;
    // 申请超过最大允许的内存 65536B
    addr1       = kmalloc(0x10001);
    // 应该返回 nullptr
    assert(addr1 == nullptr);
    // 申请小块内存
    addr2 = kmalloc(0x1);
    assert(addr2 != nullptr);
    // 第一块被申请的内存，减去 chunk 大小后应该是 4k 对齐的
    assert(((uintptr_t)((uint8_t*)addr2 - chunk_size) & 0xFFF) == 0x0);
    // 在 LEN512 申请新的内存
    addr3 = kmalloc(0x200);
    assert(addr3 != nullptr);
    // 第一块被申请的内存，减去 chunk 大小后应该是 4k 对齐的
    assert(((uintptr_t)((uint8_t*)addr3 - chunk_size) & 0xFFF) == 0x0);
    // 加上 chunk 大小长度刚好是 LEN256
    addr4 = kmalloc(0x80);
    assert(addr4 != nullptr);
    // LEN256 区域第二块被申请的内存，地址可以计算出来
    // 前一个块的地址+chunk 长度+数据长度+对齐长度
    assert(addr4 == (uint8_t*)addr2 + chunk_size + 0x1 + 0x7);
    /// @bug 这里释放会同时 unmmap，导致后面的分支出现 pg
    // 全部释放
    //    kfree(addr1);
    //    kfree(addr2);
    //    kfree(addr3);
    //    kfree(addr4);
    info("heap test done.\n");
    return 0;
}

// TODO: 更多测试
int test_intr(void) {
    // 触发 pg 中断
    uintptr_t* addr = (uintptr_t*)0xC0000000;
    int        tmp  = 0x666;
    tmp             = *addr;
    assert(tmp == 0);
    *addr = 0x233;
    tmp   = *addr;
    assert(tmp == 0x233);
    *addr = 0x0;
    info("intr test done.\n");
    return 0;
}

int test_fs(void) {
    // // FatFs work area needed for each volume
    // FATFS    FatFs;
    // // File object needed for each open file
    // FIL      Fil;
    //
    // uint32_t bw;
    // FRESULT  fr;
    //
    // // Give a work area to the default drive
    // fr = f_mount(&FatFs, "/", 1);
    // info("f_mount：%d\n", fr);
    //
    // // Create a file
    // fr = f_open(&Fil, "newfile.txt", FA_WRITE | FA_CREATE_ALWAYS);
    // info("f_open：%d\n", fr);
    //
    // if (fr == FR_OK) {
    //     // Write data to the file
    //     f_write(&Fil, "It works!\r\n", 11, &bw);
    //     // Close the file
    //     fr = f_close(&Fil);
    //     info("f_close %d\n", fr);
    // }

    FATFS   sdcard_fs;
    FRESULT status;
    DIR     dj;
    FILINFO fno;

    printf("/********************fs test*******************/\n");
    status = f_mount(&sdcard_fs, "", 0);
    printf("mount sdcard: %d\n", status);
    if (status != FR_OK) {
        return status;
    }

    /// @bug 找不到文件
    status = f_findfirst(&dj, &fno, "", "*");

    printf("printf filename %d [%s]\n", status, "fno.fname[0]");
    while (status == FR_OK && fno.fname[0]) {
        if (fno.fattrib & AM_DIR) {
            printf("dir: %s\n", fno.fname);
        }
        else {
            printf("file: %s\n", fno.fname);
        }
        status = f_findnext(&dj, &fno);
        printf("f_findnext %d [%s]\n", status, "fno.fname[0]");
    }
    f_closedir(&dj);
    return 0;
}

int test_vfs(void) {
    VFS::get_instance().mkdir("/233", 0);
    VFS::get_instance().mkdir("/233/555", 0);
    VFS::get_instance().rmdir("/233/555");
    VFS::get_instance().mkdir("/233/555", 0);
    fd_t fd  = VFS::get_instance().open("/233/555/test.c", O_CREAT);
    char a[] = "test233.c";
    VFS::get_instance().write(fd, a, 10);
    char b[10];
    VFS::get_instance().read(fd, b, 10);
    assert(strcmp(a, b) == 0);
    printf("vfs test done.\n");
    return 0;
}
