
/**
 * @file bus_dev.cpp
 * @brief 总线设备基类
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2021-09-18
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2021-12-01<td>MRNIU<td>迁移到 doxygen
 * </table>
 */

#include "bus_dev.h"

bus_dev_t::bus_dev_t(void) {
    return;
}

bus_dev_t::bus_dev_t(const resource_t &_resource) : dev_t(_resource) {
// #define DEBUG
#ifdef DEBUG
    printf("bus_dev_t ctor.\n");
#undef DEBUG
#endif
    return;
}

bus_dev_t::~bus_dev_t(void) {
    return;
}

bool bus_dev_t::add_drv(const mystl::string &_compatible_name,
                        const mystl::string &_drv_name) {
    drvs_name.push_back(
        mystl::pair<mystl::string, mystl::string>(_compatible_name, _drv_name));
    return true;
}

bool bus_dev_t::add_dev(dev_t *_dev) {
    devs.push_back(_dev);
    return true;
}

bool bus_dev_t::match(dev_t                                     &_dev,
                      mystl::pair<mystl::string, mystl::string> &_name_pair) {
    if (_dev.compatible_name == _name_pair.first) {
        // 设置驱动
        /// @todo 这里应该新建一个驱动对象
        _dev.drv = drv_factory_t::get_instance().get_class(_name_pair.second,
                                                           _dev.resource);
        if (_dev.drv == nullptr) {
            warn("%s has not register\n", _dev.compatible_name.c_str());
            return false;
        }
        else {
            return true;
        }
    }
    else {
        return false;
    }
    return true;
}

void bus_dev_t::show(void) const {
    info("dev count: 0x%X\n", devs.size());
    for (auto i : devs) {
        std::cout << *i << std::endl;
    }
    info("drv count: 0x%X\n", drvs.size());
    for (auto i : drvs) {
        std::cout << *i << std::endl;
    }
    return;
}

dev_t *bus_dev_t::get_dev_via_intr_no(uint8_t _no) {
    // 遍历设备向量
    for (auto i : devs) {
        // 寻找对应的设备
        if (i->resource.intr_no == _no) {
            return i;
        }
    }
    return nullptr;
}
