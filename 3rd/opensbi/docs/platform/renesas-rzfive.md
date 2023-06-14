Renesas RZ/Five SoC (R9A07G043F) Platform
=========================================
The RZ/Five microprocessor includes a single RISC-V CPU Core (Andes AX45MP)
1.0 GHz, 16-bit DDR3L/DDR4 interface. Supported interfaces include:
- Memory controller for DDR4-1600 / DDR3L-1333 with 16 bits
- System RAM (RAM of 128 Kbytes (ECC))
- SPI Multi I/O Bus Controller 1ch
- SD Card Host Interface/Multimedia Card Interface (SD/MMC) 2ch
- Serial Sound Interface (SSI) 4ch
- Sampling Rate Converter (SRC) 1ch
- USB2.0 host/function interface 2ch (ch0: Host-Function ch1: Host only)
- Gigabit Ethernet Interface (GbE) 2ch
- Controller Area Network Interface (CAN) 2ch (CAN-FD ISO 11898-1 (CD2014) compliant)
- Multi-function Timer Pulse Unit 3 (MTU3a) 9 ch (16 bits × 8 channels, 32 bits × 1 channel)
- Port Output Enable 3 (POE3)
- Watchdog Timer (WDT) 1ch
- General Timer (GTM) 3ch (32bits)
- I2C Bus Interface (I2C) 4ch
- Serial Communication Interface with FIFO (SCIFA) 5ch
- Serial Communication Interface (SCI) 2ch
- Renesas Serial Peripheral Interface (RSPI) 3ch
- A/D Converter (ADC) 2ch
making it ideal for applications such as entry-class social infrastructure
gateway control and industrial gateway control. More details can be found at
below link [0].

[0] https://www.renesas.com/us/en/products/microcontrollers-microprocessors/rz-mpus/rzfive-general-purpose-microprocessors-risc-v-cpu-core-andes-ax45mp-single-10-ghz-2ch-gigabit-ethernet

To build platform specific library and firmwares, provide the
*PLATFORM=generic* parameter to the top level make command.

Platform Options
----------------

The Renesas RZ/Five platform does not have any platform-specific options.

Building Renesas RZ/Five Platform
---------------------------------

```
make PLATFORM=generic
```

DTS Example: (RZ/Five AX45MP)
-----------------------------

```
	compatible = "renesas,r9a07g043f01", "renesas,r9a07g043";

	cpus {
		#address-cells = <1>;
		#size-cells = <0>;
		timebase-frequency = <12000000>;

		cpu0: cpu@0 {
			compatible = "andestech,ax45mp", "riscv";
			device_type = "cpu";
			reg = <0x0>;
			status = "okay";
			riscv,isa = "rv64imafdc";
			mmu-type = "riscv,sv39";
			i-cache-size = <0x8000>;
			i-cache-line-size = <0x40>;
			d-cache-size = <0x8000>;
			d-cache-line-size = <0x40>;
			clocks = <&cpg CPG_CORE R9A07G043_CLK_I>;

			cpu0_intc: interrupt-controller {
				#interrupt-cells = <1>;
				compatible = "riscv,cpu-intc";
				interrupt-controller;
			};
		};
	};

	soc {
		compatible = "simple-bus";
		#address-cells = <1>;
		#size-cells = <0>;
		ranges;

		scif0: serial@1004b800 {
			compatible = "renesas,scif-r9a07g043",
				     "renesas,scif-r9a07g044";
			reg = <0 0x1004b800 0 0x400>;
			interrupts = <412 IRQ_TYPE_LEVEL_HIGH>,
				     <414 IRQ_TYPE_LEVEL_HIGH>,
				     <415 IRQ_TYPE_LEVEL_HIGH>,
				     <413 IRQ_TYPE_LEVEL_HIGH>,
				     <416 IRQ_TYPE_LEVEL_HIGH>,
				     <416 IRQ_TYPE_LEVEL_HIGH>;
			interrupt-names = "eri", "rxi", "txi",
					  "bri", "dri", "tei";
			clocks = <&cpg CPG_MOD R9A07G043_SCIF0_CLK_PCK>;
			clock-names = "fck";
			power-domains = <&cpg>;
			resets = <&cpg R9A07G043_SCIF0_RST_SYSTEM_N>;
			status = "disabled";
		};

		cpg: clock-controller@11010000 {
			compatible = "renesas,r9a07g043-cpg";
			reg = <0 0x11010000 0 0x10000>;
			clocks = <&extal_clk>;
			clock-names = "extal";
			#clock-cells = <2>;
			#reset-cells = <1>;
			#power-domain-cells = <0>;
		};

		sysc: system-controller@11020000 {
			compatible = "renesas,r9a07g043-sysc";
			reg = <0 0x11020000 0 0x10000>;
			status = "disabled";
		};

		pinctrl: pinctrl@11030000 {
			compatible = "renesas,r9a07g043-pinctrl";
			reg = <0 0x11030000 0 0x10000>;
			gpio-controller;
			#gpio-cells = <2>;
			#interrupt-cells = <2>;
			interrupt-controller;
			gpio-ranges = <&pinctrl 0 0 152>;
			clocks = <&cpg CPG_MOD R9A07G043_GPIO_HCLK>;
			power-domains = <&cpg>;
			resets = <&cpg R9A07G043_GPIO_RSTN>,
				 <&cpg R9A07G043_GPIO_PORT_RESETN>,
				 <&cpg R9A07G043_GPIO_SPARE_RESETN>;
		};

		plmt0: plmt0@110c0000 {
			compatible = "andestech,plmt0", "riscv,plmt0";
			reg = <0x0 0x110c0000 0x0 0x10000>;
			interrupts-extended = <&cpu0_intc 7>;
		};

		plic: interrupt-controller@12c00000 {
			compatible = "renesas,r9a07g043-plic", "andestech,nceplic100";
			#interrupt-cells = <2>;
			#address-cells = <0>;
			riscv,ndev = <511>;
			interrupt-controller;
			reg = <0x0 0x12c00000 0x0 0x400000>;
			clocks = <&cpg CPG_MOD R9A07G043_NCEPLIC_ACLK>;
			power-domains = <&cpg>;
			resets = <&cpg R9A07G043_NCEPLIC_ARESETN>;
			interrupts-extended = <&cpu0_intc 11 &cpu0_intc 9>;
		};

		plicsw: interrupt-controller@13000000 {
			compatible = "andestech,plicsw";
			reg = <0x0 0x13000000 0x0 0x400000>;
			interrupts-extended = <&cpu0_intc 3>;
			interrupt-controller;
			#address-cells = <2>;
			#interrupt-cells = <2>;
		};
	};
```
