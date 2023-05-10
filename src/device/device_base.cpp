
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

device_base_t::device_base_t(const resource_t& _resource)
    : dev_name(_resource.name), drv(nullptr), resource(_resource) {
    return;
}

device_base_t::device_base_t(const resource_t& _resource, driver_base_t* _drv)
    : dev_name(_resource.name), drv(_drv), resource(_resource) {
    return;
}

device_base_t::~device_base_t(void) {
    return;
}

int device_base_t::read(void* _where, void* _buf) {
    return drv->read(_where, _buf);
}

int device_base_t::write(void* _where, void* _buf) {
    return drv->write(_where, _buf);
}

int device_base_t::ioctl(uint8_t _cmd, void* _buf) {
    return drv->ioctl(_cmd, _buf);
}

int device_base_t::status(uint8_t _cmd) {
    return drv->status(_cmd);
}
