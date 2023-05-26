
/**
 * @file port.cpp
 * @brief IA32 端口读写实现
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

#include "port.h"

uint8_t PORT::inb(const uint32_t _port) {
    uint8_t ret;
    __asm__ volatile("inb %1, %0" : "=a"(ret) : "dN"(_port));
    return ret;
}

uint16_t PORT::inw(const uint32_t _port) {
    uint16_t ret;
    __asm__ volatile("inw %1, %0" : "=a"(ret) : "dN"(_port));
    return ret;
}

uint32_t PORT::ind(const uint32_t _port) {
    uint32_t ret;
    __asm__ volatile("inl %1, %0" : "=a"(ret) : "dN"(_port));
    return ret;
}

void PORT::outw(const uint32_t _port, const uint16_t _data) {
    __asm__ volatile("outw %1, %0" : : "dN"(_port), "a"(_data));
    return;
}

void PORT::outb(const uint32_t _port, const uint8_t _data) {
    __asm__ volatile("outb %1, %0" : : "dN"(_port), "a"(_data));
    return;
}

void PORT::outd(const uint32_t _port, const uint32_t _data) {
    __asm__ volatile("outl %1, %0" : : "dN"(_port), "a"(_data));
    return;
}
