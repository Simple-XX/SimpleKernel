
/**
 * @file port.cpp
 * @brief 串口驱动
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2023-03-31
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2023-03-31<td>Zone.N<td>迁移到 doxygen
 * </table>
 */

#include "port.h"

uint8_t PORT::inb(const uint32_t port __attribute((unused))) {
    return 0;
}

uint16_t PORT::inw(const uint32_t port __attribute((unused))) {
    return 0;
}

uint32_t PORT::ind(const uint32_t port) {
    return *(uint32_t volatile*)port;
}

void PORT::outw(const uint32_t port __attribute((unused)),
                const uint16_t data __attribute((unused))) {
    return;
}

void PORT::outb(const uint32_t port __attribute((unused)),
                const uint8_t  data __attribute((unused))) {
    return;
}

void PORT::outd(const uint32_t port, const uint32_t data) {
    *(uint32_t volatile*)port = data;
    return;
}
