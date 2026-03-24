# GIC (Generic Interrupt Controller)

AArch64 架构通用中断控制器，实现 GICv3 规范

## 功能特性

- 支持 GICv3 规范
- 分布式架构：Distributor (GICD) + Redistributor (GICR)
- 支持三种中断类型：
  - SGI (Software Generated Interrupts): ID 0-15
  - PPI (Private Peripheral Interrupts): ID 16-31
  - SPI (Shared Peripheral Interrupts): ID 32-1019
- 支持多核处理器 (SMP)
- 支持中断优先级管理
- 支持中断亲和性配置

## 类接口

### 构造函数
```cpp
Gic(uint64_t gicd_base_addr, uint64_t gicr_base_addr)
```
- `gicd_base_addr`: GICD (Distributor) 基地址
- `gicr_base_addr`: GICR (Redistributor) 基地址

### 主要方法
```cpp
void SetUp() const
```
初始化当前 CPU 的 GIC 配置

```cpp
void SPI(uint32_t intid, uint32_t cpuid) const
```
配置共享外设中断 (SPI)

```cpp
void PPI(uint32_t intid, uint32_t cpuid) const
```
配置私有外设中断 (PPI)

### Distributor (GICD) 接口
```cpp
void Enable(uint32_t intid) const
void Disable(uint32_t intid) const
void Clear(uint32_t intid) const
void SetPrio(uint32_t intid, uint32_t prio) const
void SetTarget(uint32_t intid, uint32_t cpuid) const
```

### Redistributor (GICR) 接口
```cpp
void Enable(uint32_t intid, uint32_t cpuid) const
void Disable(uint32_t intid, uint32_t cpuid) const
void Clear(uint32_t intid, uint32_t cpuid) const
void SetPrio(uint32_t intid, uint32_t cpuid, uint32_t prio) const
```

## 使用示例

```cpp
// 创建 GIC 实例
Gic gic(0x8000000,   // GICD 基地址
        0x80A0000);  // GICR 基地址

// 多核环境初始化
gic.SetUp();

// 配置 SPI 中断 35，由 CPU 0 处理
gic.SPI(35, 0);

// 配置 PPI 中断 30，由当前 CPU 处理
gic.PPI(30, cpu_io::GetCurrentCoreId());
```

## 参考文档

https://developer.arm.com/documentation/100095/0003/Generic-Interrupt-Controller-CPU-Interface

https://www.kernel.org/doc/Documentation/devicetree/bindings/interrupt-controller/arm%2Cgic-v3.txt

https://github.com/qemu/qemu/blob/master/hw/arm/virt.c

## 设备树配置示例

```
intc@8000000 {
		phandle = <0x8005>;
		reg = <0x00 0x8000000 0x00 0x10000 0x00 0x80a0000 0x00 0xf60000>;
		#redistributor-regions = <0x01>;
		compatible = "arm,gic-v3";
		ranges;
		#size-cells = <0x02>;
		#address-cells = <0x02>;
		interrupt-controller;
		#interrupt-cells = <0x03>;

		its@8080000 {
			phandle = <0x8006>;
			reg = <0x00 0x8080000 0x00 0x20000>;
			#msi-cells = <0x01>;
			msi-controller;
			compatible = "arm,gic-v3-its";
		};
	};
```
