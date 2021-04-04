
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// apic.h for Simple-XX/SimpleKernel.

#ifndef _APIC_H_
#define _APIC_H_

#include "stdint.h"

class APIC {
private:
    // TODO: 完善寄存器信息
    // LAPIC ID Register	Read/Write
    static constexpr const uint32_t LAPIC_ID = 0x020;
    // LAPIC Version Register	Read only
    static constexpr const uint32_t LAPIC_VER = 0x030;
    // Task Priority Register (TPR)	Read/Write
    static constexpr const uint32_t LAPIC_TPR = 0x080;
    // Arbitration Priority Register (APR)	Read only
    static constexpr const uint32_t LAPIC_APR = 0x090;
    // Processor Priority Register (PPR)	Read only
    static constexpr const uint32_t LAPIC_PPR = 0x0A0;
    // EOI register	Write only
    static constexpr const uint32_t LAPIC_EOI = 0x0B0;
    // Remote Read Register (RRD)	Read only
    static constexpr const uint32_t LAPIC_RRD = 0x0C0;
    // Logical Destination Register	Read/Write
    static constexpr const uint32_t LAPIC_LDR = 0x0D0;
    // Destination Format Register	Read/Write
    static constexpr const uint32_t LAPIC_DFR = 0x0E0;
    // Spurious Interrupt Vector Register	Read/Write
    static constexpr const uint32_t LAPIC_SIVR = 0x0F0;
    // 100h - 170h	In-Service Register (ISR)
    static constexpr const uint32_t LAPIC_ISR = 0x100;
    // 180h - 1F0h	Trigger Mode Register (TMR)
    static constexpr const uint32_t LAPIC_TMR = 0x180;
    // 200h - 270h	Interrupt Request Register (IRR)
    static constexpr const uint32_t LAPIC_IRR = 0x200;
    // Error Status Register	Read only
    static constexpr const uint32_t LAPIC_ERR = 0x280;
    // LVT Corrected Machine Check Interrupt (CMCI) Register	Read/Write
    static constexpr const uint32_t LAPIC_CMCI = 0x2F0;
    // 300h - 310h	Interrupt Command Register (ICR)
    static constexpr const uint32_t LAPIC_ICR = 0x300;
    // LVT Timer Register	Read/Write
    static constexpr const uint32_t LAPIC_LVT_TIMER = 0x320;
    // LVT Thermal Sensor Register	Read/Write
    static constexpr const uint32_t LAPIC_LVT_THERMAL = 0x330;
    // LVT Performance Monitoring Counters Register	Read/Write
    static constexpr const uint32_t LAPIC_LVT_PREF_MONITORING = 0x340;
    // LVT LINT0 Register	Read/Write
    static constexpr const uint32_t LAPIC_LVT_LINT0 = 0x350;
    // LVT LINT1 Register	Read/Write
    static constexpr const uint32_t LAPIC_LVT_LINT1 = 0x360;
    // LVT Error Register	Read/Write
    static constexpr const uint32_t LAPIC_LVT_ERR = 0x370;
    // Initial Count Register (for Timer)	Read/Write
    static constexpr const uint32_t LAPIC_TIMER_INIT_COUNT = 0x380;
    // Current Count Register (for Timer)	Read only
    static constexpr const uint32_t LAPIC_TIMER_CURR_COUNT = 0x390;
    // Divide Configuration Register (for Timer)	Read/Write
    static constexpr const uint32_t LAPIC_TIMER_DIVIDE = 0x3E0;

    void *pbase;
    void *vbase;

protected:
public:
    APIC(void);
    ~APIC(void);
    uint32_t read(const uint32_t _idx) const;
    void     write(const uint32_t _idx, const uint32_t _data) const;
};

class IOAPIC {
private:
protected:
public:
    IOAPIC(void);
    ~IOAPIC(void);
    uint32_t read(const uint32_t _idx) const;
    void     write(const uint32_t _idx, const uint32_t _data) const;
};

#endif /* _APIC_H_ */
