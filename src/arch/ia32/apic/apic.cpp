
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// apic.cpp for Simple-XX/SimpleKernel.

#include "stdio.h"
#include "assert.h"
#include "intr.h"
#include "cpu.hpp"
#include "apic.h"

// 64-ia-32-architectures-software-developer-vol-3a-manual#10

// TODO: 完善
// TODO: 加入内核
APIC::APIC(void) {
    return;
}

APIC::~APIC(void) {
    return;
}

// 64-ia-32-architectures-software-developer-vol-3a-manual#10.4.3

int32_t APIC::init(void) {
    LOCAL_APIC::init();
    IO_APIC::init();
    info("apic init.\n");
    return 0;
}
