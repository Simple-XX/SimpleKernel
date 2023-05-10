
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

bool bus_device_t::match(device_base_t& _dev, const mystl::string& _drv_name) {
    // 名称不匹配直接返回
    if (_dev.dev_name != _drv_name) {
        return false;
    }
    // 已有驱动直接返回
    if (_dev.drv != nullptr) {
        warn("%s already has driver\n", _dev.dev_name.c_str());
        return false;
    }
    // 设置驱动 通过 driver_factory_t new 了一个驱动对象
    _dev.drv
      = driver_factory_t::get_instance().get_class(_drv_name, _dev.resource);
    // 设置失败
    if (_dev.drv == nullptr) {
        warn("%s has not register\n", _dev.dev_name.c_str());
        return false;
    }
    return true;
}

bool bus_device_t::match(void) {
    auto ret = false;
    // 遍历找到没有驱动的设备
    for (auto i : devices) {
        if (i->drv == nullptr) {
            // 遍历所有驱动，尝试进行匹配
            for (auto j : drivers) {
                ret = match(*i, j);
                if (ret == true) {
                    info("%s device init successful.\n", i->dev_name.c_str());
                }
            }
        }
    }
    return ret;
}

bus_device_t::bus_device_t(const mystl::string& _bus_name)
    : bus_name(_bus_name) {
    return;
}

bool bus_device_t::add_driver(const mystl::string& _compatible_name) {
    drivers.push_back(_compatible_name);
    match();
    return true;
}

bool bus_device_t::add_device(device_base_t* _dev) {
    devices.push_back(_dev);
    match();
    return true;
}

void bus_device_t::show(void) const {
    info("dev count: 0x%X\n", devices.size());
    for (auto i : devices) {
        std::cout << *i << std::endl;
    }
    info("drv count: 0x%X\n", drivers.size());
    for (auto i : drivers) {
        std::cout << i << std::endl;
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
