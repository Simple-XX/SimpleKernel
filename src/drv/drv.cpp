
/**
 * @file drv.cpp
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

#include "drv.h"

drv_t::drv_t(void) : name("drv_t"), type_name("drv_t typename") {
    return;
}

drv_t::drv_t(const resource_t &) {
    return;
}

drv_t::drv_t(const mystl::string &_name, const mystl::string &_type_name)
    : name(_name), type_name(_type_name) {
    return;
}

drv_t::~drv_t(void) {
    return;
}

drv_factory_t::drv_factory_t(void) {
    return;
}

drv_factory_t::~drv_factory_t(void) {
    return;
}

drv_factory_t &drv_factory_t::get_instance(void) {
    static drv_factory_t factory;
    return factory;
}

void drv_factory_t::register_class(const mystl::string     &_class_name,
                                   const constructor_fun_t &_ctor_fun) {
    // 插入到 map
    type_name_ctor_map.insert(
        mystl::pair<mystl::string, constructor_fun_t>(_class_name, _ctor_fun));
    return;
}

drv_t *drv_factory_t::get_class(const mystl::string &_class_name,
                                const resource_t    &_resource) const {
    auto iter = type_name_ctor_map.find(_class_name);
    if (iter == type_name_ctor_map.end()) {
        return (drv_t *)nullptr;
    }
    else {
        return (drv_t *)((iter->second)(_resource));
    }
    return (drv_t *)nullptr;
}
