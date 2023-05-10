
/**
 * @file bus_device.cpp
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

#include "bus_device.h"

bus_device_t::bus_device_t(void) {
    is_ready = true;
    return;
}

bus_device_t::bus_device_t(const mystl::string& _bus_name)
    : bus_name(_bus_name) {
    is_ready = true;
    return;
}

bus_device_t::~bus_device_t(void) {
    return;
}

bool bus_device_t::add_driver(const mystl::string& _compatible_name,
                              const mystl::string& _drv_name) {
    drvs_name.push_back(
      mystl::pair<mystl::string, mystl::string>(_compatible_name, _drv_name));
    return true;
}

bool bus_device_t::add_device(device_base_t* _dev) {
    devices.push_back(_dev);
    return true;
}

bool bus_device_t::match(
  device_base_t& _dev, mystl::pair<mystl::string, mystl::string>& _name_pair) {
    if (_dev.dev_name == _name_pair.first) {
        // 设置驱动
        /// @todo 这里应该新建一个驱动对象
        _dev.drv = drv_factory_t::get_instance().get_class(_name_pair.first,
                                                           _dev.resource);
        if (_dev.drv == nullptr) {
            warn("%s has not register\n", _dev.dev_name.c_str());
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

void bus_device_t::show(void) const {
    info("dev count: 0x%X\n", devices.size());
    for (auto i : devices) {
        std::cout << *i << std::endl;
    }
    info("drv count: 0x%X\n", drvs_name.size());
    for (auto i : drvs_name) {
        std::cout << i.first << std::endl;
    }
    return;
}

device_base_t* bus_device_t::get_dev_via_intr_no(uint8_t _no) {
    // 遍历设备向量
    for (auto i : devices) {
        // 寻找对应的设备
        if (i->resource.intr_no == _no) {
            return i;
        }
    }
    return nullptr;
}
