
/**
 * @file drv.h
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


#ifndef _DRV_H_
#define _DRV_H_

#include "stdint.h"
#include "vector"
#include "string"

// 设备驱动抽象，是所有驱动的基类
class drv_t {
private:
protected:
public:
    // 驱动名
    mystl::string name;

    drv_t(void);
    virtual ~drv_t(void) = 0;
    // 驱动操作
    // 初始化
    virtual bool         init(void) = 0;
    friend std::ostream &operator<<(std::ostream &_out, drv_t &_drv) {
        info("drv name: %s", _drv.name.c_str());
        return _out;
    }
};

#endif /* _DRV_H_ */
