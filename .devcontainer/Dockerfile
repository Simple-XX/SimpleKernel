# Copyright The SimpleKernel Contributors

FROM ubuntu:latest

ENV DEBIAN_FRONTEND=noninteractive

# 基础工具 + QEMU（用于系统测试）
RUN apt-get update && apt-get upgrade -y && \
    apt-get install --no-install-recommends --fix-missing -y \
    ca-certificates \
    curl \
    git \
    build-essential \
    pkg-config \
    device-tree-compiler \
    qemu-system-arm \
    qemu-system-misc \
    gdb-multiarch && \
    apt-get autoremove -y && apt-get clean -y && \
    rm -rf /var/lib/apt/lists/*

# Rust toolchain（由 rust-toolchain.toml 控制具体版本）
ENV RUSTUP_HOME=/usr/local/rustup \
    CARGO_HOME=/usr/local/cargo \
    PATH=/usr/local/cargo/bin:$PATH

RUN curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | \
    sh -s -- -y --default-toolchain none && \
    rustup show
