
/**
 * @file apic.cpp
 * @brief APIC 抽象
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

#include "stdio.h"
#include "assert.h"
#include "intr.h"
#include "cpu.hpp"
#include "apic.h"

// 64-ia-32-architectures-software-developer-vol-3a-manual#10

/// @todo 完善，加入内核
APIC::APIC(void) {
    return;
}

APIC::~APIC(void) {
    return;
}

/// @see 64-ia-32-architectures-software-developer-vol-3a-manual#10.4.3

int32_t APIC::init(void) {
    LOCAL_APIC::init();
    IO_APIC::init();
    info("apic init.\n");
    return 0;
}
