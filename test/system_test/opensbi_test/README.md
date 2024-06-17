# opensbi_test

使用 opensbi 加载内核。

步骤：

1. 在 qemu 中运行
2. 进入 opensbi
3. opensbi 跳转到 0x80210000 执行内核
4. 内核使用 opensbi 提供的接口进行输出
