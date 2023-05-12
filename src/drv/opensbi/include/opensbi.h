
/**
 * @file opensbi.cpp
 * @brief opensbi 接口头文件
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

#ifndef SIMPLEKERNEL_OPENSBI_H
#define SIMPLEKERNEL_OPENSBI_H

#include "cstdint"

/**
 * @brief opensbi 接口抽象
 * @see
 * https://github.com/riscv-non-isa/riscv-sbi-doc/blob/master/riscv-sbi.adoc
 */
class OPENSBI {
public:
    /**
     * @brief sbi 返回值
     */
    struct sbiret_t {
        enum : long {
            SUCCESS               = 0,
            ERR_FAILED            = -1,
            ERR_NOT_SUPPORTED     = -2,
            ERR_INVALID_PARAM     = -3,
            ERR_DENIED            = -4,
            ERR_INVALID_ADDRESS   = -5,
            ERR_ALREADY_AVAILABLE = -6,
            ERR_ALREADY_STARTED   = -7,
            ERR_ALREADY_STOPPED   = -8,
        };

        /// 错误码
        long error;
        /// 返回值
        long value;
    };

private:
    /// 设置定时器
    static constexpr const uint32_t FID_SET_TIMER       = 0x0;
    static constexpr const uint32_t EID_SET_TIMER       = 0x54494D45;

    /// uart 输出字符
    static constexpr const uint32_t FID_CONSOLE_PUTCHAR = 0x0;
    static constexpr const uint32_t EID_CONSOLE_PUTCHAR = 0x1;

    /// uart 获取字符
    static constexpr const uint32_t FID_CONSOLE_GETCHAR = 0x0;
    static constexpr const uint32_t EID_CONSOLE_GETCHAR = 0x2;

    /// 清空 IPI
    static constexpr const uint32_t FID_CLEAR_IPI       = 0x0;
    static constexpr const uint32_t EID_CLEAR_IPI       = 0x3;

    /// 发送 IPI
    static constexpr const uint32_t FID_SEND_IPI        = 0x0;
    static constexpr const uint32_t EID_SEND_IPI        = 0x735049;

    /// 关机
    static constexpr const uint32_t FID_SHUTDOWN        = 0x0;
    static constexpr const uint32_t EID_SHUTDOWN        = 0x8;

    /// 启动 hart
    static constexpr const uint32_t FID_HART_START      = 0x0;
    static constexpr const uint32_t EID_HART_START      = 0x48534D;

    /**
     * @brief ecall 借口
     * @param  _arg0            使用 a0 传递
     * @param  _arg1            使用 a1 传递
     * @param  _arg2            使用 a2 传递
     * @param  _arg3            使用 a3 传递
     * @param  _arg4            使用 a4 传递
     * @param  _arg5            使用 a5 传递
     * @param  _fid             使用 a6 传递
     * @param  _eid             使用 a7 传递
     * @return sbiret_t
     */
    sbiret_t
    ecall(unsigned long _arg0, unsigned long _arg1, unsigned long _arg2,
          unsigned long _arg3, unsigned long _arg4, unsigned long _arg5,
          int _fid, int _eid);

protected:

public:
    /**
     * @brief 获取单例
     * @return OPENSBI&         静态对象
     */
    static OPENSBI& get_instance(void);

    /**
     * @brief 输出一个字符
     * @param  _c              要输出的字符
     */
    void            put_char(const char _c);

    /**
     * @brief 从 uart 读取一个字符
     * @return uint8_t         读取到的字符，没有读到数据返回 0xFF
     * @note 不会阻塞
     */
    uint8_t         get_char(void);

    /**
     * @brief 设置时钟
     * @param  _value          要设置的时间
     * @return sbiret_t        返回值
     */
    sbiret_t        set_timer(uint64_t _value);

    /**
     * @brief 发送 ipi(inter-processor interrupt) 给指定的 hart
     * @param  _hart_mask       要发送给的 hart
     * @note hart_mask is a virtual address that points to a bit-vector of
     * harts. The bit vector is represented as a sequence of unsigned longs
     * whose length equals the number of harts in the system divided by the
     * number of bits in an unsigned long, rounded up to the next integer.
     * @return sbiret_t        返回值
     */
    sbiret_t        send_ipi(const unsigned long* _hart_mask);

    /**
     * @brief 启动指定的 hart
     * @param  _hartid          要启动的 hart id
     * @param  _start_addr      启动后执行的程序地址
     * @param  _opaque          参数
     * The opaque parameter is a XLEN-bit value which will be set in the a1
     * register when the hart starts executing at start_addr.
     * @return sbiret_t        返回值
     * @note 启动后的寄存器状态
     * satp: 0
     * sstatus.SIE: 0
     * a0: hartid
     * a1: _opaque
     */
    sbiret_t        hart_start(unsigned long _hartid, unsigned long _start_addr,
                               unsigned long _opaque);
};

#endif /* SIMPLEKERNEL_OPENSBI_H */
