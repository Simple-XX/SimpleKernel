
/**
 * @file opensbi.cpp
 * @brief opensbi 接口实现
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

#include "opensbi.h"

OPENSBI::sbiret_t OPENSBI::ecall(unsigned long _arg0, unsigned long _arg1,
                                 unsigned long _arg2, unsigned long _arg3,
                                 unsigned long _arg4, unsigned long _arg5,
                                 int _fid, int _eid) {
    OPENSBI::sbiret_t  ret;
    register uintptr_t a0 asm("a0") = (uintptr_t)(_arg0);
    register uintptr_t a1 asm("a1") = (uintptr_t)(_arg1);
    register uintptr_t a2 asm("a2") = (uintptr_t)(_arg2);
    register uintptr_t a3 asm("a3") = (uintptr_t)(_arg3);
    register uintptr_t a4 asm("a4") = (uintptr_t)(_arg4);
    register uintptr_t a5 asm("a5") = (uintptr_t)(_arg5);
    register uintptr_t a6 asm("a6") = (uintptr_t)(_fid);
    register uintptr_t a7 asm("a7") = (uintptr_t)(_eid);
    asm volatile("ecall"
                 : "+r"(a0), "+r"(a1)
                 : "r"(a2), "r"(a3), "r"(a4), "r"(a5), "r"(a6), "r"(a7)
                 : "memory");
    ret.error = a0;
    ret.value = a1;
    return ret;
}

OPENSBI &OPENSBI::get_instance(void) {
    /// 定义全局 OPENSBI 对象
    static OPENSBI opensbi;
    return opensbi;
}

void OPENSBI::put_char(const char _c) {
    ecall(_c, 0, 0, 0, 0, 0, FID_CONSOLE_PUTCHAR, EID_CONSOLE_PUTCHAR);
    return;
}

uint8_t OPENSBI::get_char(void) {
    return (uint8_t)ecall(0, 0, 0, 0, 0, 0, FID_CONSOLE_GETCHAR,
                          EID_CONSOLE_GETCHAR)
        .value;
}

OPENSBI::sbiret_t OPENSBI::set_timer(uint64_t _value) {
    return ecall(_value, 0, 0, 0, 0, 0, FID_SET_TIMER, EID_SET_TIMER);
}

OPENSBI::sbiret_t OPENSBI::send_ipi(const unsigned long *_hart_mask) {
    return ecall((uintptr_t)_hart_mask, 0, 0, 0, 0, 0, FID_SEND_IPI,
                 EID_SEND_IPI);
}

OPENSBI::sbiret_t OPENSBI::hart_start(unsigned long _hartid,
                                      unsigned long _start_addr,
                                      unsigned long _opaque) {
    return ecall(_hartid, _start_addr, _opaque, 0, 0, 0, FID_HART_START,
                 EID_HART_START);
}
