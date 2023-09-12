# 构建系统

## 目录及文件介绍

```shell
.
├── 3rd                                 // 第三方源文件
├── CMakeLists.txt                      // 最上层 cmake
├── CMakePresets.json                   // 预定义的 cmake 配置
├── Dockerfile                          // docker 配置
├── LICENSE                             // MIT 与 ANTI996 
├── README.md                           // README
├── build_riscv64                       // riscv64 架构的构建目录
├── build_x86_64                        // x86_64 架构的构建目录
├── cmake                               // cmake 模块目录
│   ├── 3rd.cmake                       // 管理第三方资源
│   ├── add_header.cmake                // 为 target 添加头文件
│   ├── clang.cmake                     // clang 工具链配置
│   ├── compile_config.cmake            // 编译选项
│   ├── functions.cmake                 // 辅助函数
│   ├── project_config.cmake            // 项目配置
│   ├── x86_64-riscv64-gcc.cmake        // host 为 x86_64，target 为 riscv64 的工具链
│   └── x86_64-x86_64-gcc.cmake         // host 为 x86_64，target 为 x86_64 的工具链
├── doc                                 // 文档目录
│   ├── CMakeLists.txt                  // 生成 doxygen 文档
│   ├── UEFI_Spec_2_10_Aug29.pdf        // UEFI 协议文档
│   ├── build.md                        // 本文件
│   └── docker.md                       // docker 使用方法
├── run.sh                              // 一键运行脚本
├── src                                 // 代码目录
│   ├── CMakeLists.txt                  // 
│   ├── boot                            // 引导程序目录
│   │   ├── CMakeLists.txt              // 
│   │   ├── boot.cpp                    // 
│   │   ├── graphics.cpp                // 
│   │   ├── include                     // 
│   │   │   ├── boot.h                  // 
│   │   │   ├── load_elf.h              // 
│   │   │   └── ostream.hpp             // 
│   │   ├── load_elf.cpp                // 
│   │   ├── memory.cpp                  // 
│   │   └── ostream.cpp                 // 
│   └── kernel                          // 内核目录
│       ├── CMakeLists.txt              // 
│       ├── arch                        // 架构相关代码
│       │   ├── CMakeLists.txt          // 
│       │   ├── aarch64                 // 
│       │   │   └── arch.cpp            // 
│       │   ├── arch.cpp                // 
│       │   ├── include                 // 
│       │   │   └── arch.h              // 
│       │   ├── riscv64                 // 
│       │   │   ├── arch.cpp            // 
│       │   │   ├── boot.S              // 
│       │   │   └── link.ld             // 链接脚本
│       │   └── x86_64                  // 
│       │       └── arch.cpp            // 
│       ├── driver                      // 驱动目录
│       │   ├── CMakeLists.txt          // 
│       │   ├── driver.cpp              // 
│       │   └── include                 // 
│       │       └── driver.h            // 
│       ├── include                     // 内核头文件目录
│       │   └── kernel.h                // 
│       ├── libc                        // libc 目录
│       │   ├── CMakeLists.txt          // 
│       │   ├── include                 // 
│       │   │   └── libc.h              // 
│       │   └── libc.c                  // 
│       ├── libcxx                      // libcxx 目录
│       │   ├── CMakeLists.txt          // 
│       │   ├── include                 // 
│       │   │   └── libcxx.h            // 
│       │   └── libcxx.cpp              // 
│       └── main.cpp                    // 内核入口
├── test                                // 测试目录
│   ├── CMakeLists.txt                  // 
│   ├── integration_test                // 集成测试
│   │   ├── CMakeLists.txt              // 
│   │   └── example.cpp                 // 
│   ├── system_test                     // 系统测试
│   │   ├── CMakeLists.txt              // 
│   │   ├── gnu_efi_test                // 测试 gnu-efi
│   │   │   ├── CMakeLists.txt          // 
│   │   │   ├── README.md               // 
│   │   │   ├── boot.cpp                // 
│   │   │   └── main.cpp                // 
│   │   └── opensbi_test                // 测试 opensbi
│   │       ├── CMakeLists.txt          // 
│   │       └── boot.cpp                // 
│   └── unit_test                       // 单元测试
│       ├── CMakeLists.txt              // 
│       └── example.cpp                 // 
└── tools                               // 其它工具
    ├── cppcheck-suppressions.xml       // cppcheck 配置文件
    ├── ovmf                            // ovmf 文件目录
    │   └── OVMF_x86_64.fd              // 
    └── startup.nsh.in                  // uefi 初始化配置输入文件
```

每个子模块都有自己的 cmake 文件，上级 cmake 通过 add_subdirectory 将其引入，同时传递由 config.cmake 设置的变量。

所有内核相关代码都在 src 目录下，其中 

1. boot 是引导程序，在生成时为一个独立的 elf 文件。
2. arch 目录下存放了体系结构相关的代码，在有涉及到体系结构内容时应当将实现放在这里，向外提供统接口。
3. driver 目录下是设备驱动。在内核开发初期，可以将驱动代码与内核一起编译，在后期应当提供独立文件。
4. kernel 目录是内核的核心逻辑，内存管理、任务管理等。
5. libc 与 libcxx 是 c/c++ 库的位置，提供内核可用的相关接口。

## 命名规范

- 系统测试

    以测试内容为名称的文件夹。

    cmake project 名为 xxx-test，如 gnu-efi-test。
