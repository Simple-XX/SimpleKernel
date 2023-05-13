
/**
 * @file io_apic.cpp
 * @brief IOAPIC 抽象
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

#include "apic.h"
#include "cpu.hpp"
#include "intr.h"

/// @todo

IO_APIC::IO_APIC(void) {
    return;
}

IO_APIC::~IO_APIC(void) {
    return;
}

int32_t IO_APIC::init(void) {
    info("io apic init.\n");
    return 0;
}