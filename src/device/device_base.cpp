
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
    buf.sector = _buf.sector;
    auto res   = drv->read(buf);

    // 等待中断完成
    while (buf.valid == false) {
        ;
    }

    memcpy(_buf.data, buf.data, COMMON::BUFFFER_SIZE);

    buf.valid = false;

    return res;
}

int device_base_t::write(buf_t& _buf) {
    auto res = drv->write(buf);

    // 等待中断完成
    while (buf.valid == false) {
        ;
    }

    memcpy(buf.data, _buf.data, COMMON::BUFFFER_SIZE);

    buf.valid = false;

    return res;
}

int device_base_t::ioctl(uint8_t _cmd, void* _buf) {
    return drv->ioctl(_cmd, _buf);
}

int device_base_t::status(uint8_t _cmd) {
    return drv->status(_cmd);
}
