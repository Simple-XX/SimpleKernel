
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
    enum : uint8_t {
        /// 内存
        MEM = 1 << 0,
        /// 中断号
        INTR_NO = 1 << 1,
    };
    uint8_t type;
    /// 资源名称
    char *name;
    /// 内存信息
    struct {
        uintptr_t addr;
        size_t    len;
    } mem;
    /// 中断号
    uint8_t intr_no;

    resource_t(void) : type(0), name(nullptr) {
        mem.addr = 0;
        mem.len  = 0;
        intr_no  = 0;
        return;
    }

    /**
     * @brief resource_t 输出
     * @param  _os             输出流
     * @param  _res            要输出的 resource_t
     * @return std::ostream&   输出流
     */
    friend std::ostream &operator<<(std::ostream &_os, const resource_t &_res) {
        printf("%s: ", _res.name);
        if (_res.type & MEM) {
            printf("MEM(0x%p, 0x%p)", _res.mem.addr, _res.mem.len);
        }
        if (_res.type & INTR_NO) {
            printf(", INTR_NO(0x%p)", _res.intr_no);
        }
        return _os;
    }
};

#endif /* _RESOURCE_H_ */
