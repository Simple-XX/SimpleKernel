# ns16550a

riscv64 使用的 uart

```
serial@10000000 {
			interrupts = <0x0a>;
			interrupt-parent = <0x03>;
			clock-frequency = "\08@";
			reg = <0x00 0x10000000 0x00 0x100>;
			compatible = "ns16550a";
		};
```


