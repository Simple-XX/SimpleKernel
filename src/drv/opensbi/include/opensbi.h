
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

#ifndef _OPENSBI_H_
#define _OPENSBI_H_

#include "stdint.h"

/**
 * @brief opensbi 接口抽象
 */
class OPENSBI {
private:
    /// 设置定时器
    static constexpr const uint32_t SET_TIMER = 0x0;
    /// uart 输出字符
    static constexpr const uint32_t CONSOLE_PUTCHAR = 0x1;
    /// uart 获取字符
    static constexpr const uint32_t CONSOLE_GETCHAR = 0x2;
    /// 清空 IPI
    static constexpr const uint32_t CLEAR_IPI = 0x3;
    /// 发送 IPI
    static constexpr const uint32_t SEND_IPI = 0x4;
    /// @todo 待补全
    static constexpr const uint32_t REMOTE_FENCE_I = 0x5;
    /// @todo 待补全
    static constexpr const uint32_t REMOTE_SFENCE_VMA = 0x6;
    /// @todo 待补全
    static constexpr const uint32_t REMOTE_SFENCE_VMA_ASID = 0x7;
    /// 关机
    static constexpr const uint32_t SHUTDOWN = 0x8;

    static constexpr const uint32_t EID_HSM        = 0x48534D;
    static constexpr const uint32_t FID_HSM_START  = 0;
    static constexpr const uint32_t FID_HSM_STOP   = 1;
    static constexpr const uint32_t FID_HSM_STATUS = 2;
    static constexpr const uint32_t EID_TIME       = 0x54494D45;
    static constexpr const uint32_t FID_TIME_SET   = 0;

    // TODO
    struct sbi_ret_t {
        long error;
        long value;
    };

    /**
     * @brief ecall 接口
     * @param  _num            系统调用号 使用 a7 传递
     * @param  _a0             使用 a0 传递
     * @param  _a1             使用 a1 传递
     * @param  _a2             使用 a2 传递
     * @param  _a3             使用 a3 传递
     * @return uint64_t        返回 _num 指定的调用的返回值
     */
    uint64_t ecall(uint64_t _num, uint64_t _a0, uint64_t _a1, uint64_t _a2,
                   uint64_t _a3);

protected:
public:
    /**
     * @brief 获取单例
     * @return OPENSBI&         静态对象
     */
    static OPENSBI &get_instance(void);

    /**
     * @brief 输出一个字符
     * @param  _c              要输出的字符
     */
    void put_char(const char _c);

    /**
     * @brief 从 uart 读取一个字符
     * @return uint8_t         读取到的字符，没有读到数据返回 0xFF
     * @note 不会阻塞
     */
    uint8_t get_char(void);

    /**
     * @brief 设置时钟
     * @param  _value          要设置的时间
     */
    void set_timer(uint64_t _value);

    void send_ipi(const unsigned long _hart_mask);
    // privilege mode to set to (1 = supervisor, 0 = user)
    void hart_start(unsigned long _hartid, unsigned long _start_addr,
                           unsigned long _priv);
};

#endif /* _OPENSBI_H_ */
