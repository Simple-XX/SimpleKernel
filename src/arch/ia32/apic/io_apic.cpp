
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// ioapic.cpp for Simple-XX/SimpleKernel.

#include "intr.h"
#include "cpu.hpp"
#include "io.h"
#include "apic.h"

// TODO

IO_APIC::IO_APIC(void) {

    return;
}

IO_APIC::~IO_APIC(void) {
    return;
}

int32_t IO_APIC::init(void) {
    printf("io apic init.\n");
    return 0;
}