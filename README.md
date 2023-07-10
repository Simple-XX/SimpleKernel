
# boot

## 本分支新增特性

基本的构建体系与辅助脚本，包括

- 对不同体系结构的支持

    目前支持 x86_64，aarch64，riscv64 三种架构

    支持在 qemu 平台上运行

    支持在 x86_64 架构的 osx、win(wsl)、linux(ubuntu) 上编译运行

- 对 uefi 与 opensbi 的支持

    x86_64 与 aarch64 架构通过 uefi 进行引导

    riscv64 通过 opensbi 进行引导

- 对 qemu 的支持

    通过 qemu 进行模拟

- 对使用 qemu gdb 进行调试的支持

    通过生成内核调试符号、向 qemu 传递正确的参数，达到使用 gdb 进行调试，观察内核的目标

- 对 clion 的支持

    clion 的配置文件与用法

- 对第三方代码的支持

    用到的第三方库的自动下载、编译

- 对项目的约束

    代码规范、commit 规范

编译参数的确定

- 汇总到 config.cmake

从系统启动到进入内核

- x86_64，aarch64 通过 uefi 进行引导
- riscvf64 通过 opensbi 进行引导

## 已支持的全部特性

- poxix-uefi
- gnu-efi
- x86_64
- aarch64
- uefi
