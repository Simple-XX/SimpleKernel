
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// opensbi.h for Simple-XX/SimpleKernel.

#ifndef _OPENSBI_H_
#define _OPENSBI_H_

#include "stddef.h"
#include "stdint.h"

class OPENSBI {
private:
    static constexpr const uint32_t SET_TIMER              = 0x0;
    static constexpr const uint32_t CONSOLE_PUTCHAR        = 0x1;
    static constexpr const uint32_t CONSOLE_GETCHAR        = 0x2;
    static constexpr const uint32_t CLEAR_IPI              = 0x3;
    static constexpr const uint32_t SEND_IPI               = 0x4;
    static constexpr const uint32_t REMOTE_FENCE_I         = 0x5;
    static constexpr const uint32_t REMOTE_SFENCE_VMA      = 0x6;
    static constexpr const uint32_t REMOTE_SFENCE_VMA_ASID = 0x7;
    static constexpr const uint32_t SHUTDOWN               = 0x8;
    // _num: 系统调用号 使用 a7 传递
    // _a0: 使用 a0 传递
    // _a1: 使用 a1 传递
    // _a2: 使用 a2 传递
    static uint64_t ecall(uint64_t _num, uint64_t _a0, uint64_t _a1,
                          uint64_t _a2, uint64_t _a3);

protected:
public:
    OPENSBI(void);
    ~OPENSBI(void);
    static void put_char(const char _c);
    // 从 uart 读取一个字符
    // 不会阻塞
    // 没有读到数据返回 0xFF
    static uint8_t get_char(void);
    static void    set_timer(uint64_t _value);
};

#endif /* _OPENSBI_H_ */
