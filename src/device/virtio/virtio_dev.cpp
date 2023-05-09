
/**
 * @file virtio_dev.cpp
 * @brief virtio 设备实现
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2021-12-21
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2021-12-01<td>MRNIU<td>迁移到 doxygen
 * </table>
 */

#include "virtio_dev.h"

virtio_dev_t::virtio_dev_t(void) {
    return;
}

virtio_dev_t::virtio_dev_t(const resource_t &_resource) : device_base_t(_resource) {
// #define DEBUG
#ifdef DEBUG
    std::cout<<"virtio_dev_t ctor."<<std::endl;
#undef DEBUG
#endif
    return;
}

virtio_dev_t::~virtio_dev_t(void) {
    return;
}

int virtio_dev_t::read(void* _where, void* _buf) {
    return device_base_t::read(_where, _buf);
}

int virtio_dev_t::write(void* _where, void* _buf) {
    return device_base_t::write(_where, _buf);
}

int virtio_dev_t::ioctl(uint8_t _cmd, void* _buf) {
    return device_base_t::ioctl(_cmd, _buf);
}

int virtio_dev_t::status(uint8_t _cmd) {
    return device_base_t::status(_cmd);
}
