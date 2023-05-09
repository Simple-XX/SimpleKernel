
/**
 * @file kernel_main.cpp
 * @brief 内核主要逻辑
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2021-09-18
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2021-09-18<td>digmouse233<td>迁移到 doxygen
 * </table>
 */

#include "boot_info.h"
#include "cassert"
#include "common.h"
#include "cpu.hpp"
#include "cstdio"
#include "dev_drv_manager.h"
#include "heap.h"
#include "intr.h"
#include "iostream"
#include "kernel.h"
#include "pmm.h"
#include "vfs.h"
#include "vmm.h"

#include "ff.h"

#include "diskio.h"

#include "virtio_mmio_drv.h"


#define BSIZE 1024  // block size
struct buf {
    int valid;   // has data been read from disk?
    int disk;    // does disk "own" buf?
    uint32_t dev;
    uint32_t blockno;
    //    struct sleeplock lock;
    uint32_t refcnt;
    struct buf *prev; // LRU cache list
    struct buf *next;
    uint8_t data[BSIZE];
};
extern void virtio_disk_rw(struct buf *b, int write);


DRESULT disk_read(BYTE _pdrv, BYTE* _buff, LBA_t _sector, UINT _count) {
    DRESULT    res;
    int        result;
info("read _sector: 0x%X, _count: 0x%X\n",_sector,_count);
//    bus_dev_t* dev
//      = (bus_dev_t*)DEV_DRV_MANAGER::get_instance().get_dev_via_intr_no(1);
//    virtio_mmio_drv_t* drv = (virtio_mmio_drv_t*)dev->drv;
//    virtio_mmio_drv_t::virtio_blk_req_t* req
//      = new virtio_mmio_drv_t::virtio_blk_req_t;
//    req->type   = virtio_mmio_drv_t::virtio_blk_req_t::IN;
//    req->sector = _sector;
//    drv->rw(*req, _buff);
for(int i = 0; i < _count; i++){
    struct buf a;
//    a.data = _buff;
    a.blockno = _sector /2;
    virtio_disk_rw(&a, 0);
}


    return RES_OK;
}

DRESULT disk_write(BYTE _pdrv, const BYTE* _buff, LBA_t _sector, UINT _count) {
    DRESULT res;
    int     result;
    info("write _sector: 0x%X, _count: 0x%X\n",_sector,_count);

    bus_dev_t* dev
      = (bus_dev_t*)DEV_DRV_MANAGER::get_instance().get_dev_via_intr_no(1);
    virtio_mmio_drv_t* drv = (virtio_mmio_drv_t*)dev->drv;

    virtio_mmio_drv_t::virtio_blk_req_t* req
      = new virtio_mmio_drv_t::virtio_blk_req_t;
    req->type   = virtio_mmio_drv_t::virtio_blk_req_t::OUT;
    req->sector = _sector;
    void* aaa=const_cast<uint8_t*>( _buff);


    auto size = drv->rw(*req, aaa);

    warn("write done %d\n",size);

    return RES_OK;
}

/**
 * @brief 内核主要逻辑
 * @note 这个函数不会返回
 */
void kernel_main(void) {
    // 初始化
    BOOT_INFO::init();
    // 物理内存初始化
    PMM::get_instance().init();
    // 测试物理内存
    test_pmm();
    // 虚拟内存初始化
    /// @todo 将vmm的初始化放在构造函数里，这里只做开启分页
    VMM::get_instance().init();
    // 测试虚拟内存
    test_vmm();
    // 堆初始化
    HEAP::get_instance().init();
    // 测试堆
    test_heap();
    // 中断初始化
    INTR::get_instance().init();
    // 测试中断
    test_intr();
    // 时钟中断初始化
    TIMER::get_instance().init();
    // 初始化设备
    DEV_DRV_MANAGER::get_instance().init();
    // 测试设备
    // test_dev();
    // 初始化文件系统
    VFS::get_instance().init();
    // 测试文件系统
    // test_vfs();
    // 允许中断
    CPU::ENABLE_INTR();
    // 显示基本信息
    show_info();

    // FatFs work area needed for each volume
    FATFS   FatFs;
    // File object needed for each open file
    FIL     Fil;

    UINT    bw;
    FRESULT fr;

    info("1sssssss\n");

    // Give a work area to the default drive
    auto aaa = f_mount(&FatFs, "", 0);
    info("2sssssss %d\n",aaa);

    // Create a file
    fr = f_open(&Fil, "newfile.txt",
                FA_WRITE | FA_CREATE_ALWAYS);

    info("fr： %d\n",fr);

    if (fr == FR_OK) {
        // Write data to the file
        f_write(&Fil, "It works!\r\n", 11, &bw);
        // Close the file
        fr = f_close(&Fil);
                                                 info("sssssss\n");
    }

    // 进入死循环
    while (1) {
        ;
    }
    // 不应该执行到这里
    assert(0);
    return;
}

/**
 * @brief 输出系统信息
 */
void show_info(void) {
    // 内核实际大小
    auto kernel_size = COMMON::KERNEL_END_ADDR - COMMON::KERNEL_START_ADDR;
    // 内核实际占用页数
    auto kernel_pages
      = (COMMON::ALIGN(COMMON::KERNEL_END_ADDR, COMMON::PAGE_SIZE)
         - COMMON::ALIGN(COMMON::KERNEL_START_ADDR, COMMON::PAGE_SIZE))
      / COMMON::PAGE_SIZE;
    info("Kernel start: 0x%p, end 0x%p, size: 0x%X bytes, 0x%X pages.\n",
         COMMON::KERNEL_START_ADDR, COMMON::KERNEL_END_ADDR, kernel_size,
         kernel_pages);
    info("Kernel start4k: 0x%p, end4k: 0x%p.\n",
         COMMON::ALIGN(COMMON::KERNEL_START_ADDR, 4 * COMMON::KB),
         COMMON::ALIGN(COMMON::KERNEL_END_ADDR, 4 * COMMON::KB));
    std::cout << "Simple Kernel." << std::endl;
    return;
}
