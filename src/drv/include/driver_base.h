
/**
 * @file driver_base.h
 * @brief 设备驱动基类头文件
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

#ifndef SIMPLEKERNEL_DRIVER_BASE_H
#define SIMPLEKERNEL_DRIVER_BASE_H

#include "cstdint"
#include "map"
#include "resource.h"
#include "string"
#include "vector"

// 设备驱动抽象，是所有驱动的基类
class driver_base_t {
private:

protected:

public:
    // 驱动名
    const mystl::string name;

    driver_base_t(void);
    driver_base_t(const mystl::string& _name);
    virtual ~driver_base_t(void)                         = 0;

    /**
     * @brief 从设备读
     * @param  _resource        设备使用的资源
     * @param  _drv             设备使用的驱动
     */
    virtual int          read(void* _where, void* _buf)  = 0;

    /**
     * @brief 向设备写
     * @param  _resource        设备使用的资源
     * @param  _drv             设备使用的驱动
     */
    virtual int          write(void* _where, void* _buf) = 0;

    /**
     * @brief ioctl 控制
     * @param  _resource        设备使用的资源
     * @param  _drv             设备使用的驱动
     */
    virtual int          ioctl(uint8_t _cmd, void* _buf) = 0;

    /**
     * @brief 获取设备状态
     * @param  _resource        设备使用的资源
     * @param  _drv             设备使用的驱动
     */
    virtual int          status(uint8_t _cmd)            = 0;

    friend std::ostream& operator<<(std::ostream& _out, driver_base_t& _drv) {
        info("drv name: %s", _drv.name.c_str());
        return _out;
    }
};

/**
 * @brief 创建驱动实例工厂类
 */
class driver_factory_t {
public:
    /**
     * @brief 用于创建驱动实例的回调函数函数指针
     */
    typedef driver_base_t* (*constructor_fun_t)(const resource_t& _resource);

private:
    /**
     * @brief 类型名(非 compatible)-用于创建驱动实例的回调函数映射表
     */
    mystl::map<mystl::string, constructor_fun_t> type_name_ctor_map;

    /**
     * @brief 使用默认构造
     */
    driver_factory_t(void)  = default;

    /**
     * @brief 使用默认析构
     */
    ~driver_factory_t(void) = default;

public:
    /**
     * @brief 获取单例
     * @return drv_factory_t&       静态对象
     */
    static driver_factory_t& get_instance(void);

    /**
     * @brief 注册回调函数
     * @param  _compatible_name 驱动类型名
     * @param  _ctor_fun        回调函数指针
     */
    void           register_class(const mystl::string&     _compatible_name,
                                  const constructor_fun_t& _ctor_fun);

    /**
     * @brief 创建驱动实例
     * @param  _compatible_name 驱动类型名
     * @param  _resource        需要的硬件信息
     * @return drv_t*           创建好的驱动实例指针
     */
    driver_base_t* get_class(const mystl::string& _compatible_name,
                             const resource_t&    _resource) const;
};

// 这里的逻辑是，在填充设备的 drv 字段时，new 一个驱动对象
// 以下三个宏都是为了这一目的
// 使用时，首先在 xx_drv.h 的最后创建声明
// define_call_back(xx_drv_t)
// 在 xx_drv.cpp 的最后进行定义
// define_call_back(xx_drv_t)
// 使用 register_call_back(xx_drv_t) 将驱动注册到 drv_factory_t 中
// 最后，在 match 函数中通过 drv_factory_t 创建新对象并赋值给设备的 drv

/**
 * @brief 声明用于创建驱动实例的回调函数
 * @param  _class_name      驱动类型名
 */
#define declare_call_back(_class_name) \
    _class_name* call_back_##_class_name(const resource_t& _resource)

/**
 * @brief 定义用于创建驱动实例的回调函数
 * @param  _class_name      驱动类型名
 */
#define define_call_back(_class_name)                                   \
    _class_name* call_back_##_class_name(const resource_t& _resource) { \
        return new _class_name(_resource);                              \
    }

/**
 * @brief 注册用于创建驱动实例的回调函数
 * @param  _bus             要注册的总线
 * @param  _compatible_name 要注册的驱动名
 * @param  _class_name      驱动类型名
 */
#define register_call_back(_bus, _compatible_name, _class_name)           \
    do {                                                                  \
        driver_factory_t::get_instance().register_class(                  \
          _compatible_name,                                               \
          (driver_factory_t::constructor_fun_t)&call_back_##_class_name); \
        _bus->add_driver(_compatible_name);                               \
    } while (0)

#endif /* SIMPLEKERNEL_DRIVER_BASE_H */
