
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
    const mystl::string type_name;

    driver_base_t(void);
    driver_base_t(const resource_t&);
    driver_base_t(const mystl::string& _name, const mystl::string& _type_name);
    virtual ~driver_base_t(void)                         = 0;
    // 驱动操作
    // 初始化
    virtual bool         init(void)                      = 0;

    // 设备基本操作
    // 从设备读
    virtual int          read(void* _where, void* _buf)  = 0;
    // 向设备写
    virtual int          write(void* _where, void* _buf) = 0;
    // ioctl 控制
    virtual int          ioctl(uint8_t _cmd, void* _buf) = 0;
    // 获取设备状态
    virtual int          status(uint8_t _cmd)            = 0;

    friend std::ostream& operator<<(std::ostream& _out, driver_base_t& _drv) {
        info("drv name: %s, drv type_name: %s", _drv.name.c_str(),
             _drv.type_name.c_str());
        return _out;
    }
};

template <class _T>
struct drv {
    char* compatible_name;
    _T    drv;
};

/**
 * @brief 创建驱动实例工厂类
 */
class drv_factory_t {
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
    drv_factory_t(void);
    ~drv_factory_t(void);

public:
    /**
     * @brief 获取单例
     * @return drv_factory_t&       静态对象
     */
    static drv_factory_t& get_instance(void);

    /**
     * @brief 注册回调函数
     * @param  _class_name      驱动类型名
     * @param  _ctor_fun        回调函数指针
     */
    void                  register_class(const mystl::string&     _class_name,
                                         const constructor_fun_t& _ctor_fun);

    /**
     * @brief 创建驱动实例
     * @param  _class_name      驱动类型名
     * @param  _resource        需要的硬件信息
     * @return drv_t*           创建好的驱动实例指针
     */
    driver_base_t*        get_class(const mystl::string& _class_name,
                                    const resource_t&    _resource) const;
};

/**
 * @brief 声明用于创建驱动实例的回调函数
 */
#define declare_call_back(_class_name) \
    _class_name* call_back_##_class_name(const resource_t& _resource);

/**
 * @brief 定义用于创建驱动实例的回调函数
 */
#define define_call_back(_class_name)                                   \
    _class_name* call_back_##_class_name(const resource_t& _resource) { \
        return new _class_name(_resource);                              \
    }

/**
 * @brief 注册用于创建驱动实例的回调函数
 */
#define register_call_back(_class_name)                                \
    do {                                                               \
        drv_factory_t::get_instance().register_class(                  \
          #_class_name,                                                \
          (drv_factory_t::constructor_fun_t)&call_back_##_class_name); \
    } while (0);

#endif /* SIMPLEKERNEL_DRIVER_BASE_H */
