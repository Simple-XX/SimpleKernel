
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// ioapic.cpp for Simple-XX/SimpleKernel.

#include "intr.h"
#include "cpu.hpp"
#include "io.h"
#include "apic.h"

// TODO

IOAPIC::IOAPIC(void) {

    return;
}

IOAPIC::~IOAPIC(void) {
    return;
}

uint32_t IOAPIC::read(const uint32_t _idx __attribute__((unused))) const {
    return 0;
}

void IOAPIC::write(const uint32_t _idx __attribute__((unused)),
                   const uint32_t _data __attribute__((unused))) const {
    return;
}
