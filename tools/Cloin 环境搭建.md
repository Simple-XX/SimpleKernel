# Clion 环境搭建

NOTE: 未完成

以 riscv64 为例

## 工具链

![截屏2021-06-29 下午2.36.16](https://tva1.sinaimg.cn/large/008i3skNly1grz4mhjvazj31ka0u0alh.jpg)

1. C Compiler: 修改为实际工具路径
2. C++ Compiler: 修改为实际工具路径
3. Debugger: 修改为实际工具路径

## CMake

使用 Clion 打开该工程，如下配置 Clion 中的 cmake，配置参数：

![截屏2021-06-29 下午2.53.54](/Users/nzh/Library/Application%20Support/typora-user-images/%E6%88%AA%E5%B1%8F2021-06-29%20%E4%B8%8B%E5%8D%882.53.54.png)

1. `DCMAKE_TOOLCHAIN_FILE` 修改为实际需要的工具链，
2. `DARCH` 修改为要运行的架构

## 运行

需要借助 external tool 运行虚拟机

![截屏2021-06-29 下午3.07.00](https://tva1.sinaimg.cn/large/008i3skNly1grz5ig6ltuj31op0u0qp4.jpg)

Working directory: 修改为实际 SimpleKernel 路径

## 调试

在 external tool 设置页面 Arguments 追加 `-gdb tcp::2333`

![截屏2021-06-29 下午3.12.08](https://tva1.sinaimg.cn/large/008i3skNly1grz5nr2mnfj31s50u0wyg.jpg)



## 使用

先运行 kernel.elf，再开启 debug
