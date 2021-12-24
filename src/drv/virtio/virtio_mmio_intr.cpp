
/**
 * @file virtio_mmio_intr.cpp
 * @brief virtio mmio 中断处理
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2021-12-23
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2021-12-23<td>MRNIU<td>新增文件
 * </table>
 */

#include "stdint.h"
#include "vector"
#include "string"
#include "virtio_queue.h"
#include "virtio_mmio_drv.h"
#include "drv.h"
#include "io.h"
#include "intr.h"
#include "dev_drv_manager.h"

/**
 * @brief mmio 中断处理
 */
void virtio_mmio_intr(uint8_t _no) {
    // 遍历设备列表，寻找驱动号对应的设备
    dev_t *dev = DEV_DRV_MANAGER::get_instance().get_dev_via_intr_no(_no);
    printf("------------\n");
    std::cout<<*dev<<std::endl;
    printf("++++++++++++\n");
    dev->drv->init();
    return;
}
