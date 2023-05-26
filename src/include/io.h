
/**
 * @file io.h
 * @brief IO 接口
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

#ifndef SIMPLEKERNEL_IO_H
#define SIMPLEKERNEL_IO_H

#include "color.h"
#include "cstdint"

// 根据不同架构选择不同的输出方式
#if defined(__x86_64__)
#    include "tui.h"
#elif defined(__aarch64__)
#    include "uart.h"
#elif defined(__riscv)
#    include "sbi_console.h"
#endif

/**
 * @brief IO 接口抽象
 */
class IO {
private:
    // io 对象
#if defined(__i386__) || defined(__x86_64__)
    // x86 平台使用 TUI 接口
    TUI io;
#elif defined(__arm__) || defined(__aarch64__)
    // arn 平台使用 UART 接口
    UART io;
#elif defined(__riscv)
    // riscv 平台使用 opensbi 提供的接口
    SBI_CONSOLE io;
#endif

protected:

public:
    /// 缓冲区大小
    static constexpr const size_t BUF_SIZE = 512;

    /**
     * @brief 获取单例
     * @return IO&              静态对象
     */
    static IO&                    get_instance(void);

    /**
     * @brief 端口读字节
     * @param  _port           要读的端口
     * @return uint8_t         读到的数据
     */
    uint8_t                       inb(const uint32_t _port);

    /**
     * @brief 端口读字
     * @param  _port           要读的端口
     * @return uint16_t        读到的数据
     */
    uint16_t                      inw(const uint32_t _port);

    /**
     * @brief 端口读双字
     * @param  _port           要读的端口
     * @return uint32_t        读到的数据
     */
    uint32_t                      ind(const uint32_t _port);

    /**
     * @brief 端口写字节
     * @param  _port           要写的端口
     * @param  _data           要写的数据
     */
    void           outb(const uint32_t _port, const uint8_t _data);

    /**
     * @brief 端口写字
     * @param  _port           要写的端口
     * @param  _data           要写的数据
     */
    void           outw(const uint32_t _port, const uint16_t _data);

    /**
     * @brief 端口写双字
     * @param  _port           要写的端口
     * @param  _data           要写的数据
     */
    void           outd(const uint32_t _port, const uint32_t _data);

    /**
     * @brief MMIO 读字节
     * @param  _addr           要读的地址
     * @return uint8_t         读到的数据
     */
    uint8_t        read8(void* _addr);

    /**
     * @brief MMIO 写字节
     * @param  _addr           要写的地址
     * @param  _val            要写的值
     */
    void           write8(void* _addr, uint8_t _val);

    /**
     * @brief MMIO 读字
     * @param  _addr           要读的地址
     * @return uint16_t        读到的数据
     */
    uint16_t       read16(void* _addr);

    /**
     * @brief MMIO 写字
     * @param  _addr           要写的地址
     * @param  _val            要写的值
     */
    void           write16(void* _addr, uint16_t _val);

    /**
     * @brief MMIO 读双字
     * @param  _addr           要读的地址
     * @return uint32_t        读到的数据
     */
    uint32_t       read32(void* _addr);

    /**
     * @brief MMIO 写双字
     * @param  _addr           要写的地址
     * @param  _val            要写的数据
     */
    void           write32(void* _addr, uint32_t _val);

    /**
     * @brief MMIO 读四字
     * @param  _addr           要读的地址
     * @return uint64_t        读到的数据
     */
    uint64_t       read64(void* _addr);

    /**
     * @brief MMIO 写四字
     * @param  _addr           要写的地址
     * @param  _val            要写的数据
     */
    void           write64(void* _addr, uint64_t _val);

    /**
     * @brief 获取当前颜色
     * @return COLOR::color_t  当前使用的颜色
     */
    COLOR::color_t get_color(void);

    /**
     * @brief 设置当前颜色
     * @param  _color          要设置的颜色
     */
    void           set_color(const COLOR::color_t _color);

    /**
     * @brief 输出字符
     * @param  _c              要输出的字符
     */
    void           put_char(const char _c);

    /**
     * @brief 输入字符
     * @return char            读到的字符
     * @todo
     */
    char           get_char(void);

    /**
     * @brief 输出字符串
     * @param  _s              要输出的字符串
     * @return int32_t         输出的长度
     * @todo 返回值
     */
    int32_t        write_string(const char* _s);
};

#endif /* SIMPLEKERNEL_IO_H */
