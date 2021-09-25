
/**
 * @file resource.h
 * @brief 资源信息抽象
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2021-09-18
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2021-09-18<td>digmouse233<td>迁移到 doxygen
 * </table>
 */

#ifndef _RESOURCE_H_
#define _RESOURCE_H_

#include "stdint.h"
#include "iostream"
#include "assert.h"

/**
 * @brief 用于表示一种资源
 */
struct resource_t {
    /// 资源类型
    typedef enum : uint8_t {
        /// 内存
        MEM,
    } type_t;
    type_t type;
    /// 资源名称
    char *name;
    /// 当资源类型为内存时，uinon 保存内存地址
    /// 当资源类型为中断号，uinon 保存中断号
    union {
        struct {
            uintptr_t addr;
            size_t    len;
        } mem;
    };
    /**
     * @brief resource_t 输出
     * @param  _os             输出流
     * @param  _res            要输出的 resource_t
     * @return std::ostream&   输出流
     */
    friend std::ostream &operator<<(std::ostream &_os, const resource_t &_res) {
        switch (_res.type) {
            case MEM: {
                printf("%s(MEM), 0x%p, 0x%p", _res.name, _res.mem.addr,
                       _res.mem.len);
                break;
            }
            default: {
                assert(0);
                break;
            }
        }
        return _os;
    }
};

#endif /* _RESOURCE_H_ */
