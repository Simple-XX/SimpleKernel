
/**
 * @file driver_base.cpp
 * @brief 设备驱动基类实现
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

#include "driver_base.h"

driver_base_t::driver_base_t(void) : name("drv_t") {
    return;
}

driver_base_t::driver_base_t(const mystl::string& _name,
                             const resource_t&    _resource)
    : name(_name) {
    return;
}

driver_base_t::~driver_base_t(void) {
    return;
}

driver_factory_t& driver_factory_t::get_instance(void) {
    static driver_factory_t factory;
    return factory;
}

void driver_factory_t::register_class(const mystl::string&     _compatible_name,
                                      const constructor_fun_t& _ctor_fun) {
    // 插入到 map
    type_name_ctor_map.insert(mystl::pair<mystl::string, constructor_fun_t>(
      _compatible_name, _ctor_fun));
    return;
}

driver_base_t*
driver_factory_t::get_class(const mystl::string& _compatible_name,
                            const resource_t&    _resource) const {
    auto iter = type_name_ctor_map.find(_compatible_name);
    if (iter == type_name_ctor_map.end()) {
        return (driver_base_t*)nullptr;
    }
    else {
        return (driver_base_t*)((iter->second)(_resource));
    }
    return (driver_base_t*)nullptr;
}
