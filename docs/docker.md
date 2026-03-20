
# Dev Container 开发环境

本项目使用 [Dev Container](https://containers.dev/) 提供一致的开发环境，包含 GCC 14 交叉编译工具链、QEMU 和全部构建依赖。

## 快速开始

### VS Code

1. 安装 [Dev Containers](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers) 扩展
2. 打开项目目录
3. 点击左下角 `><` 图标，选择 **Reopen in Container**
4. 等待容器构建完成（首次约 5-10 分钟）

### GitHub Codespaces

点击仓库页面的 **Code → Codespaces → Create codespace on main**，环境自动就绪。

### CLI

```shell
# 安装 devcontainer CLI
npm install -g @devcontainers/cli

# 构建并启动
devcontainer up --workspace-folder .

# 在容器内执行命令
devcontainer exec --workspace-folder . cmake --preset=build_riscv64
```

## 验证环境

```shell
gcc --version                      # GCC 14
aarch64-linux-gnu-gcc --version    # aarch64 交叉编译器
riscv64-linux-gnu-gcc --version    # riscv64 交叉编译器
cmake --version
qemu-system-riscv64 --version
```

## 构建与运行

```shell
# 配置 + 编译（二选一）
cmake --preset=build_riscv64 && cmake --build build_riscv64 --target SimpleKernel
cmake --preset=build_aarch64 && cmake --build build_aarch64 --target SimpleKernel

# 运行（在对应 build 目录下）
make run

# 调试
make debug    # GDB 连接 localhost:1234

# 单元测试 + 覆盖率
cmake --build build_riscv64 --target unit-test coverage
```
