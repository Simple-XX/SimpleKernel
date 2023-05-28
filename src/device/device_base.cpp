
/**
 * @file device_base.cpp
 * @brief 设备基类
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2021-09-18
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2021-12-01<td>Zone.N<td>迁移到 doxygen
 * </table>
 */

#include "device_base.h"

device_base_t::device_base_t(void) : dev_name("device_base_t"), drv(nullptr) {
    return;
}

device_base_t::device_base_t(const resource_t& _resource, driver_base_t* _drv)
    : dev_name(_resource.name), drv(_drv), resource(_resource) {
    return;
}

device_base_t::~device_base_t(void) {
    // drv 在 register_call_back 中创建
    if (drv != nullptr) {
        delete drv;
        drv = nullptr;
    }
    return;
}

int device_base_t::read(buf_t& _buf) {
    // 设置要读的位置
    buf.sector = _buf.sector;
    // 读
    auto res   = drv->read(buf);

    // 等待中断完成
    while (buf.valid == false) {
        ;
    }

    // 将设备缓冲区中的数据复制到 _buf
    memcpy(_buf.data, buf.data, COMMON::BUFFFER_SIZE);

    // 将设备缓冲区设置为无效
    buf.valid = false;

    return res;
}

int device_base_t::write(buf_t& _buf) {
    // 将设备缓冲区设置为无效
    buf.valid = false;
    // 将要写的数据复制到设备缓冲区
    memcpy(buf.data, _buf.data, COMMON::BUFFFER_SIZE);
    // 设置要写的位置
    buf.sector = _buf.sector;

    // 写
    auto res   = drv->write(buf);

    // 等待中断完成
    while (buf.valid == false) {
        ;
    }

    return res;
}

int device_base_t::ioctl(uint8_t _cmd, void* _buf) {
    return drv->ioctl(_cmd, _buf);
}

int device_base_t::status(uint8_t _cmd) {
    return drv->status(_cmd);
}
