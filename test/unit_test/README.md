# unit_test

- kernel_elf_test.cpp

  读取 kernel.elf 的信息

  kernel.elf.h: kernel.elf 的二进制数据

- kernel_fdt_test.cpp

  测试 fdt 解析

  riscv64_virt.dtb:
  由 `qemu-system-riscv64 -machine virt -machine dumpdtb=riscv64_virt.dtb` 生成的
  dtb 数据

  riscv64_virt.dts: `riscv64_virt.dtb` 的可读格式

  riscv64_virt.dtb.h: `riscv64_virt.dtb` 的二进制数据

- riscv64_cpu_test.cpp

  测试 riscv64/cpu.hpp 相关内容

- x86_64_cpu_test.cpp

  测试 x86_64/cpu.hpp 相关内容

- aarch64_cpu_test.cpp

  测试 aarch64/cpu.hpp 相关内容
