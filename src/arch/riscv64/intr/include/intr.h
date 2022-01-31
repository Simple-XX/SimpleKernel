
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
    /**
     * @brief 中断处理函数指针
     * @param  _argc           参数个数
     * @param  _argv           参数列表
     * @return int32_t         返回值，0 成功
     */
    typedef int32_t (*interrupt_handler_t)(int _argc, char **_argv);

private:
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
    interrupt_handler_t interrupt_handlers[INTERRUPT_MAX]
        __attribute__((aligned(4)));
    /// 异常处理函数数组
    interrupt_handler_t excp_handlers[EXCP_MAX] __attribute__((aligned(4)));

public:
    /// 页读错误
    static constexpr const uint8_t EXCP_LOAD_PAGE_FAULT = 13;
    /// 页写错误
    static constexpr const uint8_t EXCP_STORE_PAGE_FAULT = 15;
    /// S 态时钟中断
    static constexpr const uint8_t INTR_S_TIMER = 5;
    /// S 态外部中断
    static constexpr const uint8_t INTR_S_EXTERNEL = 9;

    /**
     * @brief 获取单例
     * @return INTR&            静态对象
     */
    static INTR &get_instance(void);

    /**
     * @brief 中断初始化
     * @return int32_t         成功返回 0
     */
    int32_t init(void);

    /**
     * @brief 注册中断处理函数
     * @param  _no             中断号
     * @param  _interrupt_handler 中断处理函数
     */
    void
    register_interrupt_handler(uint8_t                   _no,
                               INTR::interrupt_handler_t _interrupt_handler);

    /**
     * @brief 注册异常处理函数
     * @param  _no             异常号
     * @param  _interrupt_handler 异常处理函数
     */
    void register_excp_handler(uint8_t                   _no,
                               INTR::interrupt_handler_t _interrupt_handler);

    /**
     * @brief 执行中断处理
     * @param  _no             中断号
     * @param  _argc           参数个数
     * @param  _argv           参数列表
     * @return int32_t         返回值，0 成功
     */
    int32_t do_interrupt(uint8_t _no, int32_t _argc, char **_argv);

    /**
     * @brief 执行异常处理
     * @param  _no             异常号
     * @param  _argc           参数个数
     * @param  _argv           参数列表
     * @return int32_t         返回值，0 成功
     */
    int32_t do_excp(uint8_t _no, int32_t _argc, char **_argv);

    /**
     * @brief 获取中断名
     * @param  _no              中断号
     * @return const char*      中断名
     */
    const char *get_intr_name(uint8_t _no) const;

    /**
     * @brief 获取异常名
     * @param  _no              异常号
     * @return const char*      异常名
     */
    const char *get_excp_name(uint8_t _no) const;
};

/**
 * @brief core-local interrupt controller
 * 本地核心中断控制器
 * 用于控制 excp 与 intr
 */
class CLINT {
public:
    /**
     * @brief 获取单例
     * @return CLINT&           静态对象
     */
    static CLINT &get_instance(void);

    /**
     * @brief 初始化
     * @return int32_t         成功返回 0
     */
    int32_t init(void);
};

/**
 * @brief platform-level interrupt controller
 * 平台级中断控制器
 * 用于控制外部中断
 */
class PLIC {
private:
    /// 外部中断处理函数指针
    typedef void (*externel_interrupt_handler_t)(uint8_t _no);
    /// 最大外部中断数量
    static constexpr const size_t EXTERNEL_INTERRUPR_MAX = 16;
    /// 外部中断处理函数数组
    externel_interrupt_handler_t
        externel_interrupt_handlers[EXTERNEL_INTERRUPR_MAX];
    /// 基地址，由 dtb 传递
    uintptr_t base_addr;
    /// @todo ？
    uint64_t PLIC_PRIORITY;
    /// @todo ？
    uint64_t PLIC_PENDING;
    /// @todo ？
    uint64_t PLIC_SENABLE(uint64_t hart);
    /// @todo ？
    uint64_t PLIC_SPRIORITY(uint64_t hart);
    /// @todo ？
    uint64_t PLIC_SCLAIM(uint64_t hart);

protected:
public:
    /**
     * @brief 获取单例
     * @return PLIC&            静态对象
     */
    static PLIC &get_instance(void);

    /**
     * @brief 初始化
     * @return int32_t         成功返回 0
     */
    int32_t init(void);

    /**
     * @brief 向 PLIC 询问中断
     * 返回发生的外部中断号
     * @return uint8_t         中断号
     */
    uint8_t get(void);

    /**
     * @brief 告知 PLIC 已经处理了当前 IRQ
     * @param  _no             中断号
     */
    void done(uint8_t _no);

    /**
     * @brief 设置中断状态
     * @param  _no             中断号
     * @param  _status         状态
     * @todo 不确定
     */
    void set(uint8_t _no, bool _status);

    /**
     * @brief 注册外部中断处理函数
     * @param  _no             外部中断号
     * @param  _interrupt_handler 外部中断处理函数
     */
    void
    register_externel_handler(uint8_t                      _no,
                              externel_interrupt_handler_t _interrupt_handler);

    /**
     * @brief 执行外部中断处理
     * @param  _no              外部中断号
     */
    void do_externel_interrupt(uint8_t _no);
};

/**
 * @brief 时钟抽象
 */
class TIMER {
public:
    /**
     * @brief 获取单例
     * @return TIMER&           静态对象
     */
    static TIMER &get_instance(void);

    /**
     * @brief 初始化
     */
    void init(void);
};

#endif /* _INTR_H_ */
