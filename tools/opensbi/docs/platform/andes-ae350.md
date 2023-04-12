Andes AE350 SoC Platform
========================
The AE350 AXI/AHB-based platform N25(F)/NX25(F)/D25F/A25/AX25 CPU with level-one
memories, interrupt controller, debug module, AXI and AHB Bus Matrix Controller,
AXI-to-AHB Bridge and a collection of fundamental AHB/APB bus IP components
pre-integrated together as a system design. The high-quality and configurable
AHB/APB IPs suites a majority embedded systems, and the verified platform serves
as a starting point to jump start SoC designs.

To build platform specific library and firmwares, provide the
*PLATFORM=generic* parameter to the top level `make` command.

Platform Options
----------------

The Andes AE350 platform does not have any platform-specific options.

Building Andes AE350 Platform
-----------------------------

AE350's dts is included in https://github.com/andestech/linux/tree/RISCV-Linux-5.4-ast-v5_1_0-branch

**Linux Kernel Payload**

```
make PLATFORM=generic FW_PAYLOAD_PATH=<linux_build_directory>/arch/riscv/boot/Image FW_FDT_PATH=<ae350.dtb path>
```

DTS Example: (Quad-core AX45MP)
-------------------------------

```
	compatible = "andestech,ae350";
	cpus {
		#address-cells = <1>;
		#size-cells = <0>;
		timebase-frequency = <60000000>;

		CPU0: cpu@0 {
			device_type = "cpu";
			reg = <0>;
			status = "okay";
			compatible = "riscv";
			riscv,isa = "rv64imafdc";
			riscv,priv-major = <1>;
			riscv,priv-minor = <10>;
			mmu-type = "riscv,sv48";
			clock-frequency = <60000000>;
			i-cache-size = <0x8000>;
			i-cache-sets = <256>;
			i-cache-line-size = <64>;
			i-cache-block-size = <64>;
			d-cache-size = <0x8000>;
			d-cache-sets = <128>;
			d-cache-line-size = <64>;
			d-cache-block-size = <64>;
			next-level-cache = <&L2>;
			CPU0_intc: interrupt-controller {
				#interrupt-cells = <1>;
				interrupt-controller;
				compatible = "riscv,cpu-intc";
			};
		};
		CPU1: cpu@1 {
			device_type = "cpu";
			reg = <1>;
			status = "okay";
			compatible = "riscv";
			riscv,isa = "rv64imafdc";
			riscv,priv-major = <1>;
			riscv,priv-minor = <10>;
			mmu-type = "riscv,sv48";
			clock-frequency = <60000000>;
			i-cache-size = <0x8000>;
			i-cache-sets = <256>;
			i-cache-line-size = <64>;
			i-cache-block-size = <64>;
			d-cache-size = <0x8000>;
			d-cache-sets = <128>;
			d-cache-line-size = <64>;
			d-cache-block-size = <64>;
			next-level-cache = <&L2>;
			CPU1_intc: interrupt-controller {
				#interrupt-cells = <1>;
				interrupt-controller;
				compatible = "riscv,cpu-intc";
			};
		};
		CPU2: cpu@2 {
			device_type = "cpu";
			reg = <2>;
			status = "okay";
			compatible = "riscv";
			riscv,isa = "rv64imafdc";
			riscv,priv-major = <1>;
			riscv,priv-minor = <10>;
			mmu-type = "riscv,sv48";
			clock-frequency = <60000000>;
			i-cache-size = <0x8000>;
			i-cache-sets = <256>;
			i-cache-line-size = <64>;
			i-cache-block-size = <64>;
			d-cache-size = <0x8000>;
			d-cache-sets = <128>;
			d-cache-line-size = <64>;
			d-cache-block-size = <64>;
			next-level-cache = <&L2>;
			CPU2_intc: interrupt-controller {
				#interrupt-cells = <1>;
				interrupt-controller;
				compatible = "riscv,cpu-intc";
			};
		};
		CPU3: cpu@3 {
			device_type = "cpu";
			reg = <3>;
			status = "okay";
			compatible = "riscv";
			riscv,isa = "rv64imafdc";
			riscv,priv-major = <1>;
			riscv,priv-minor = <10>;
			mmu-type = "riscv,sv48";
			clock-frequency = <60000000>;
			i-cache-size = <0x8000>;
			i-cache-sets = <256>;
			i-cache-line-size = <64>;
			i-cache-block-size = <64>;
			d-cache-size = <0x8000>;
			d-cache-sets = <128>;
			d-cache-line-size = <64>;
			d-cache-block-size = <64>;
			next-level-cache = <&L2>;
			CPU3_intc: interrupt-controller {
				#interrupt-cells = <1>;
				interrupt-controller;
				compatible = "riscv,cpu-intc";
			};
		};
	};

	soc {
		#address-cells = <2>;
		#size-cells = <2>;
		compatible = "andestech,riscv-ae350-soc", "simple-bus";
		ranges;

		plic0: interrupt-controller@e4000000 {
			compatible = "riscv,plic0";
			reg = <0x00000000 0xe4000000 0x00000000 0x02000000>;
			interrupts-extended = < &CPU0_intc 11 &CPU0_intc 9
			                        &CPU1_intc 11 &CPU1_intc 9
			                        &CPU2_intc 11 &CPU2_intc 9
			                        &CPU3_intc 11 &CPU3_intc 9 >;
			interrupt-controller;
			#address-cells = <2>;
			#interrupt-cells = <2>;
			riscv,ndev = <71>;
		};

		plicsw: interrupt-controller@e6400000 {
			compatible = "andestech,plicsw";
			reg = <0x00000000 0xe6400000 0x00000000 0x00400000>;
			interrupts-extended = < &CPU0_intc 3
			                        &CPU1_intc 3
			                        &CPU2_intc 3
			                        &CPU3_intc 3 >;
			interrupt-controller;
			#address-cells = <2>;
			#interrupt-cells = <2>;
		};

		plmt0: plmt0@e6000000 {
			compatible = "andestech,plmt0";
			reg = <0x00000000 0xe6000000 0x00000000 0x00100000>;
			interrupts-extended = < &CPU0_intc 7
			                        &CPU1_intc 7
			                        &CPU2_intc 7
			                        &CPU3_intc 7 >;
		};

		wdt: watchdog@f0500000 {
			compatible = "andestech,atcwdt200";
			reg = <0x00000000 0xf0500000 0x00000000 0x00001000>;
			interrupts = <3 4>;
			interrupt-parent = <&plic0>;
			clock-frequency = <15000000>;
		};

		serial0: serial@f0300000 {
			compatible = "andestech,uart16550", "ns16550a";
			reg = <0x00000000 0xf0300000 0x00000000 0x00001000>;
			interrupts = <9 4>;
			interrupt-parent = <&plic0>;
			clock-frequency = <19660800>;
			current-speed = <38400>;
			reg-shift = <2>;
			reg-offset = <32>;
			reg-io-width = <4>;
			no-loopback-test = <1>;
		};

		smu: smu@f0100000 {
			compatible = "andestech,atcsmu";
			reg = <0x00000000 0xf0100000 0x00000000 0x00001000>;
		};
	};
```
