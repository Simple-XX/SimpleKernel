
/**
 * @file loacl_apic.cpp
 * @brief loacl APIC 抽象
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

/// @see 64-ia-32-architectures-software-developer-vol-3a-manual#10

/// @todo 完善，加入内核
LOCAL_APIC::LOCAL_APIC(void) {
    return;
}

LOCAL_APIC::~LOCAL_APIC(void) {
    return;
}

/// @see  64-ia-32-architectures-software-developer-vol-3a-manual#10.4.3

int32_t LOCAL_APIC::init(void) {
    // 先判断 cpu 是否支持
    CPU::CPUID cpuid;
    if (cpuid.xapic() == false) {
        warn("Not support LOCAL_APIC&xAPIC.\n");
    }
    if (cpuid.x2apic() == false) {
        warn("Not support x2APIC.\n");
    }
    uint64_t msr = CPU::READ_MSR(CPU::IA32_APIC_BASE);
    // 开启 xAPIC 与 x2APIC
    msr |= (CPU::IA32_APIC_BASE_GLOBAL_ENABLE_BIT |
            CPU::IA32_APIC_BASE_X2APIC_ENABLE_BIT);
    CPU::WRITE_MSR(CPU::IA32_APIC_BASE, msr);
    // 设置 SIVR
    msr = CPU::READ_MSR(CPU::IA32_X2APIC_SIVR);
    if (cpuid.eoi() == true) {
        msr |= CPU::IA32_X2APIC_SIVR_EOI_ENABLE_BIT;
    }
    msr |= CPU::IA32_X2APIC_SIVR_APIC_ENABLE_BIT;
    CPU::WRITE_MSR(CPU::IA32_X2APIC_SIVR, msr);

    // 屏蔽所有 LVT
    msr = 0;
    msr |= CPU::IA32_X2APIC_LVT_MASK_BIT;
    msr = 0x10000;
    CPU::WRITE_MSR(CPU::IA32_X2APIC_CMCI, msr);
    CPU::WRITE_MSR(CPU::IA32_X2APIC_LVT_TIMER, msr);
    CPU::WRITE_MSR(CPU::IA32_X2APIC_LVT_THERMAL, msr);
    CPU::WRITE_MSR(CPU::IA32_X2APIC_LVT_PMI, msr);
    CPU::WRITE_MSR(CPU::IA32_X2APIC_LVT_LINT0, msr);
    CPU::WRITE_MSR(CPU::IA32_X2APIC_LVT_LINT1, msr);
    CPU::WRITE_MSR(CPU::IA32_X2APIC_LVT_ERROR, msr);

    info("local apic init.\n");
    return 0;
}
