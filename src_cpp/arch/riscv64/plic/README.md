# Platform-Level Interrupt Controller (PLIC)

RISC-V 64 外部中断控制器

## 特性

- 支持最多 1024 个中断源 (ID 1-1023，ID 0 保留)
- 多 context 支持 (每个 hart 支持 M-mode 和 S-mode)
- 中断优先级管理 (优先级 0 表示禁用)
- 中断使能/禁用控制
- 中断挂起状态管理
- 中断声明与完成机制
- 优先级阈值配置

## 类接口

```cpp
class Plic {
public:
    Plic(uint64_t dev_addr, size_t ndev, size_t context_count);

    // 中断处理
    auto Which() const -> uint32_t;
    void Done(uint32_t source_id) const;
    void RegisterInterruptFunc(uint8_t cause, InterruptFunc func);
    void Do(uint64_t cause, uint8_t* context);

    // 中断配置
    void Set(uint32_t hart_id, uint32_t source_id, uint32_t priority, bool enable) const;
    auto Get(uint32_t hart_id, uint32_t source_id) const -> std::tuple<uint32_t, bool, bool>;

    // 私有辅助方法
    auto GetEnableBit(uint32_t context_id, uint32_t source_id) const -> bool;
    void SetEnableBit(uint32_t context_id, uint32_t source_id, bool value) const;
    auto SourcePriority(uint32_t source_id) const -> uint32_t&;
    auto GetPendingBit(uint32_t source_id) const -> bool;
    void SetPendingBit(uint32_t source_id, bool value) const;
    auto PriorityThreshold(uint32_t context_id) const -> uint32_t&;
    auto ClaimComplete(uint32_t context_id) const -> uint32_t&;
};
```

## 使用示例

```cpp
// 初始化 PLIC
Plic plic(0xc000000, 95, 4);  // base_addr, ndev, context_count

// 配置中断
plic.Set(0, 1, 7, true);  // hart_id=0, source_id=1, priority=7, enable=true

// 注册中断处理函数
plic.RegisterInterruptFunc(1, [](uint64_t cause, uint8_t* context) {
    // 处理中断
});

// 中断处理流程
auto source_id = plic.Which();  // 获取中断源 ID
if (source_id != 0) {
    plic.Do(source_id, context);  // 执行中断处理
    plic.Done(source_id);         // 完成中断处理
}
```

## 设备树配置

```dts
plic@c000000 {
    phandle = <0x05>;
    riscv,ndev = <0x5f>;
    reg = <0x00 0xc000000 0x00 0x600000>;
    interrupts-extended = <0x04 0x0b 0x04 0x09 0x02 0x0b 0x02 0x09>;
    interrupt-controller;
    compatible = "sifive,plic-1.0.0", "riscv,plic0";
    #address-cells = <0x00>;
    #interrupt-cells = <0x01>;
};
```

## 参考文档

https://github.com/riscv/riscv-plic-spec/blob/master/riscv-plic-1.0.0.pdf
