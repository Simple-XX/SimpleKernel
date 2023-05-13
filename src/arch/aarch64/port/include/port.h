
/**
 * @file port.h
 * @brief arm 端口驱动
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

#ifndef SIMPLEKERNEL_PORT_H
#define SIMPLEKERNEL_PORT_H

#include "cstdint"

namespace PORT {
// 端口读一个字节
uint8_t  inb(const uint32_t port);
// 端口读一个字
uint16_t inw(const uint32_t port);
// 端口读一个双字
uint32_t ind(const uint32_t port);
// 端口写一个字节
void     outb(const uint32_t port, const uint8_t data);
// 端口写一个字
void     outw(const uint32_t port, const uint16_t data);
// 端口写一个双字
void     outd(const uint32_t port, const uint32_t data);
};     // namespace PORT

#endif /* SIMPLEKERNEL_PORT_H */
