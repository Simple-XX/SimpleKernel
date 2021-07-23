
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// apic.h for Simple-XX/SimpleKernel.

#ifndef _APIC_H_
#define _APIC_H_

#include "stdint.h"

class APIC {
private:
protected:
public:
    APIC(void);
    ~APIC(void);
    static int32_t init(void);
};

class IOAPIC {
private:
protected:
public:
    IOAPIC(void);
    ~IOAPIC(void);
};

static APIC apic;

#endif /* _APIC_H_ */
