
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// apic.cpp for Simple-XX/SimpleKernel.

#include "intr.h"
#include "cpu.hpp"
#include "apic.h"

// TODO: 完善
// TODO: 加入内核

IO APIC::io;

APIC::APIC(void) {
    pbase = CPU::get_apic_base();
    // CPU::set_apic_base(pbase);
    write(LAPIC_SIVR, read(LAPIC_SIVR) | LAPIC_ISR);
    io.printf("apic init\n");
    return;
}

APIC::~APIC(void) {
    return;
}

uint32_t APIC::read(const uint32_t _idx) const {
    return *((uint32_t *)(reinterpret_cast<uint32_t>(pbase) + _idx));
}

void APIC::write(const uint32_t _idx, const uint32_t _data) const {
    *((uint32_t *)(reinterpret_cast<uint32_t>(pbase) + _idx)) = _data;
    return;
}
