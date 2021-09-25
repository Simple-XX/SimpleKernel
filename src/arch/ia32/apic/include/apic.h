
/**
 * @file apic.h
 * @brief APIC 抽象头文件
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

#ifndef _APIC_H_
#define _APIC_H_

#include "stdint.h"

/**
 * @brief APIC 抽象
 */
class APIC {
private:
protected:
public:
    APIC(void);
    ~APIC(void);
    static int32_t init(void);
};

/**
 * @brief 本地 APIC
 */
class LOCAL_APIC {
private:
protected:
public:
    LOCAL_APIC(void);
    ~LOCAL_APIC(void);
    static int32_t init(void);
};

/**
 * @brief IO APIC
 */
class IO_APIC {
private:
protected:
public:
    IO_APIC(void);
    ~IO_APIC(void);
    static int32_t init(void);
};

static APIC apic;

#endif /* _APIC_H_ */
