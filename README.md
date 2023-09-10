
[![codecov](https://codecov.io/gh/Simple-XX/SimpleKernel/graph/badge.svg?token=J7NKK3SBNJ)](https://codecov.io/gh/Simple-XX/SimpleKernel)
![workflow](https://github.com/Simple-XX/SimpleKernel/actions/workflows/workflow.yml/badge.svg)

# boot

## 本分支新增特性

参考 [MRNIU/cmake-kernel](https://github.com/MRNIU/cmake-kernel) 的构建系统，包括以下特性

- [x] 对第三方依赖的支持/构建

  自动下载并编译第三方依赖

  自动生成相关 licence

  目前支持的第三方资源

  |                          第三方内容                          |                     描述                      |     类型     | 正在使用 |
      | :----------------------------------------------------------: | :-------------------------------------------: | :----------: | :------: |
  |        [CPM](https://github.com/cpm-cmake/CPM.cmake)         |                 cmake 包管理                  | cmake module |    ✔     |
  | [CPMLicences.cmake](https://github.com/TheLartians/CPMLicenses.cmake) |            为第三方包生成 licence             | cmake module |    ✔     |
  |  [google/googletest](https://github.com/google/googletest)   |                     测试                      |      库      |    ✔     |
  |   [easylogingpp](https://github.com/amrayn/easyloggingpp)    |                     日志                      |      库      |          |
  |           [rttr](https://github.com/rttrorg/rttr)            |         c++ 反射库，在设备驱动部分用          |      库      |          |
  | [Format.cmake](https://github.com/TheLartians/Format.cmake)  | 代码格式化，支持 clang-format 与 cmake-format | cmake module |          |
  |        [FreeImage](http://freeimage.sourceforge.net/)        |                   图片渲染                    |      库      |          |
  |            [Freetype](https://www.freetype.org/)             |                   字体渲染                    |      库      |          |
  |   [opensbi](https://github.com/riscv-software-src/opensbi)   |                  riscv 引导                   |      库      |    ✔     |
  |     [gnu-efi](https://sourceforge.net/projects/gnu-efi/)     |                 gnu uefi 引导                 |      库      |    ✔     |
  |                [ovmf](SimpleKernel/3rd/ovmf)                 |             qemu 使用的 uefi 固件             |     bin      |    ✔     |
  |          [edk2](https://github.com/tianocore/edk2)           |        构建 qemu 使用的 uefi 固件 ovmf        |      库      |          |
  |       [libcxxrt](https://github.com/libcxxrt/libcxxrt)       |                c++ 运行时支持                 |      库      |    ✔     |

- [x] 文档生成

  使用 doxygen 从生成文档

- [x] 构建内核

  生成内核 elf 文件

- [x] 运行内核

  在 qemu 上运行内核

- [x] 代码格式化

  格式化全部代码

- [x] 测试

  单元测试 集成测试 系统测试

- [x] CI

  github action、codecov

- [x] 调试

  使用 make debug 等进入调试模式

从系统启动到进入内核

- x86_64，aarch64 通过 uefi 进行引导
- riscv64 通过 opensbi 进行引导

## 已支持的全部特性

- [x] build

  内核构建与调试

- [x] doxygen

  基于 doxygen 的文档生成

- [x] CI
  
  github action，github pages，code coverage，ut it st

- [x] x86_64

  基于 gnu-efi 的 x86_64 uefi 引导

- [x] riscv64

  基于 opensbi 的 riscv64 架构支持

- [x] 3rd

  对依赖的第三方库的自动下载、编译
