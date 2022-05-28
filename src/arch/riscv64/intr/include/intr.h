
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
#include "spinlock.h"

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
        [CPU::EXCP_INSTRUCTION_ADDRESS_MISALIGNED] =
            "Instruction Address Misaligned",
        [CPU::EXCP_INSTRUCTION_ACCESS_FAULT] = "Instruction Access Fault",
        [CPU::EXCP_ILLEGAL_INSTRUCTION]      = "Illegal Instruction",
        [CPU::EXCP_BREAKPOINT]               = "Breakpoint",
        [CPU::EXCP_LOAD_ADDRESS_MISALIGNED]  = "Load Address Misaligned",
        [CPU::EXCP_LOAD_ACCESS_FAULT]        = "Load Access Fault",
        [CPU::EXCP_STORE_AMO_ADDRESS_MISALIGNED] =
            "Store/AMO Address Misaligned",
        [CPU::EXCP_STORE_AMO_ACCESS_FAULT] = "Store/AMO Access Fault",
        [CPU::EXCP_ECALL_U]                = "Environment Call from U-mode",
        [CPU::EXCP_ECALL_S]                = "Environment Call from S-mode",
        "Reserved",
        [CPU::EXCP_ECALL_M]                = "Environment Call from M-mode",
        [CPU::EXCP_INSTRUCTION_PAGE_FAULT] = "Instruction Page Fault",
        [CPU::EXCP_LOAD_PAGE_FAULT]        = "Load Page Fault",
        "Reserved",
        [CPU::EXCP_STORE_AMO_PAGE_FAULT] = "Store/AMO Page Fault",
        "Reserved",
    };

    /// 中断名
    static constexpr const char *const intr_names[] = {
        [CPU::INTR_SOFT_U] = "User Software Interrupt",
        [CPU::INTR_SOFT_S] = "Supervisor Software Interrupt",
        "Reserved",
        [CPU::INTR_SOFT_M]  = "Machine Software Interrupt",
        [CPU::INTR_TIMER_U] = "User Timer Interrupt",
        [CPU::INTR_TIMER_S] = "Supervisor Timer Interrupt",
        "Reserved",
        [CPU::INTR_TIMER_M]  = "Machine Timer Interrupt",
        [CPU::INTR_EXTERN_U] = "User External Interrupt",
        [CPU::INTR_EXTERN_S] = "Supervisor External Interrupt",
        "Reserved",
        [CPU::INTR_EXTERN_M] = "Machine External Interrupt",
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

    /// 自旋锁
    spinlock_t spinlock;

public:
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
    int32_t init_other_core(void);

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
private:
    /// 自旋锁
    spinlock_t spinlock;

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
    int32_t init_other_core(void);
};

/**
 * @brief platform-level interrupt controller
 * 平台级中断控制器
 * 用于控制外部中断
 */
class PLIC {
private:
    /// 自旋锁
    spinlock_t spinlock;
    /// 基地址，由 dtb 传递
    static uintptr_t base_addr;
    /// @todo ？
    static uint64_t PLIC_PRIORITY;
    /// @todo ？
    static uint64_t PLIC_PENDING;
    /// @todo ？
    static inline uint64_t PLIC_SENABLE(uint64_t _hart);

    static inline uint64_t PLIC_MENABLE(uint64_t _hart);
    /// @todo ？
    static inline uint64_t PLIC_SPRIORITY(uint64_t _hart);
    static inline uint64_t PLIC_MPRIORITY(uint64_t _hart);
    /// @todo ？
    static inline uint64_t PLIC_SCLAIM(uint64_t _hart);
    static inline uint64_t PLIC_MCLAIM(uint64_t _hart);

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
    int32_t init_other_core(void);

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
    void init_other_core(void);
};

/**
 * @brief 缺页读处理
 */
int32_t pg_load_excp(int, char **);

/**
 * @brief 缺页写处理
 * @todo 需要读权限吗？测试发现没有读权限不行，原因未知
 */
int32_t pg_store_excp(int, char **);

#endif /* _INTR_H_ */
