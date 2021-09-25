
/**
 * @file intr.h
 * @brief 中断抽象头文件
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

#ifndef _INTR_H_
#define _INTR_H_

#include "stdint.h"

void handler_default(void);

class INTR {
public:
    typedef void (*interrupt_handler_t)(void);
    /**
     * @brief 中断初始化
     * @return int32_t         成功返回 0
     */
    static int32_t init(void);
};

/**
 * @brief core-local interrupt controller
 * 本地核心中断控制器
 * 用于控制 excp 与 intr
 */
class CLINT {
public:
    /// 页读错误
    static constexpr const uint8_t EXCP_LOAD_PAGE_FAULT = 13;
    /// 页写错误
    static constexpr const uint8_t EXCP_STORE_PAGE_FAULT = 15;
    /// 异常名
    static constexpr const char *const excp_names[] = {
        "Instruction Address Misaligned",
        "Instruction Access Fault",
        "Illegal Instruction",
        "Breakpoint",
        "Load Address Misaligned",
        "Load Access Fault",
        "Store/AMO Address Misaligned",
        "Store/AMO Access Fault",
        "Environment Call from U-mode",
        "Environment Call from S-mode",
        "Reserved",
        "Environment Call from M-mode",
        "Instruction Page Fault",
        "Load Page Fault",
        "Reserved",
        "Store/AMO Page Fault",
        "Reserved",
    };

    /// S 态时钟中断
    static constexpr const uint8_t INTR_S_TIMER = 5;
    /// S 态外部中断
    static constexpr const uint8_t INTR_S_EXTERNEL = 9;
    /// 中断名
    static constexpr const char *const intr_names[] = {
        "User Software Interrupt",
        "Supervisor Software Interrupt",
        "Reserved",
        "Machine Software Interrupt",
        "User Timer Interrupt",
        "Supervisor Timer Interrupt",
        "Reserved",
        "Machine Timer Interrupt",
        "User External Interrupt",
        "Supervisor External Interrupt",
        "Reserved",
        "Machine External Interrupt",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Local Interrupt X",
    };

    /// 最大中断数
    static constexpr const uint32_t INTERRUPT_MAX = 16;
    /// 最大异常数
    static constexpr const uint32_t EXCP_MAX = 16;
    /// 中断处理函数数组
    static INTR::interrupt_handler_t interrupt_handlers[INTERRUPT_MAX]
        __attribute__((aligned(4)));
    /// 异常处理函数数组
    static INTR::interrupt_handler_t excp_handlers[EXCP_MAX]
        __attribute__((aligned(4)));

    /**
     * @brief 初始化
     * @return int32_t         成功返回 0
     */
    static int32_t init(void);
    /**
     * @brief 注册中断处理函数
     * @param  _no             中断号
     * @param  _interrupt_handler 中断处理函数
     */
    static void
    register_interrupt_handler(uint8_t                   _no,
                               INTR::interrupt_handler_t _interrupt_handler);
    /**
     * @brief 注册异常处理函数
     * @param  _no             异常号
     * @param  _interrupt_handler 异常处理函数
     */
    static void
    register_excp_handler(uint8_t                   _no,
                          INTR::interrupt_handler_t _interrupt_handler);
    /**
     * @brief 执行中断处理
     * @param  _no             中断号
     */
    static void do_interrupt(uint8_t _no);
    /**
     * @brief 执行异常处理
     * @param  _no             异常号
     */
    static void do_excp(uint8_t _no);
};

/**
 * @brief platform-level interrupt controller
 * 平台级中断控制器
 * 用于控制外部中断
 */
class PLIC {
private:
    /// 基地址，由 dtb 传递
    static uintptr_t base_addr;
    /// @todo ？
    static const uint64_t PLIC_PRIORITY;
    /// @todo ？
    static const uint64_t PLIC_PENDING;
    /// @todo ？
    static uint64_t PLIC_SENABLE(uint64_t hart);
    /// @todo ？
    static uint64_t PLIC_SPRIORITY(uint64_t hart);
    /// @todo ？
    static uint64_t PLIC_SCLAIM(uint64_t hart);

protected:
public:
    /**
     * @brief 初始化
     * @return int32_t         成功返回 0
     */
    static int32_t init(void);
    /**
     * @brief 向 PLIC 询问中断
     * 返回发生的外部中断号
     * @return uint8_t         中断号
     */
    static uint8_t get(void);
    /**
     * @brief 告知 PLIC 已经处理了当前 IRQ
     * @param  _no             中断号
     */
    static void done(uint8_t _no);
    /**
     * @brief 设置中断状态
     * @param  _no             中断号
     * @param  _status         状态
     * @todo 不确定
     */
    static void set(uint8_t _no, bool _status);
};

/**
 * @brief 时钟抽象
 */
class TIMER {
public:
    /**
     * @brief 初始化
     */
    static void init(void);
};

#endif /* _INTR_H_ */
