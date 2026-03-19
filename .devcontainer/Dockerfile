# Copyright The SimpleKernel Contributors

FROM ubuntu:latest

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get upgrade -y && \
    apt-get install --no-install-recommends --fix-missing -y \
    ca-certificates \
    git \
    build-essential \
    binutils \
    cmake \
    pkg-config \
    flex \
    bison \
    gawk \
    bc \
    cpio \
    device-tree-compiler \
    u-boot-tools \
    grub-common \
    xorriso \
    mtools \
    dosfstools \
    python3-dev \
    python3-cryptography \
    python3-pyelftools \
    python3-setuptools \
    libgnutls28-dev \
    swig \
    libgtest-dev \
    doxygen \
    graphviz \
    lcov \
    qemu-system \
    gdb-multiarch \
    gcc-15 g++-15 \
    gcc-15-riscv64-linux-gnu g++-15-riscv64-linux-gnu \
    gcc-15-aarch64-linux-gnu g++-15-aarch64-linux-gnu \
    gcc-15-arm-linux-gnueabihf g++-15-arm-linux-gnueabihf \
    gcc-15-x86-64-linux-gnu g++-15-x86-64-linux-gnu && \
    update-alternatives \
      --install /usr/bin/gcc gcc /usr/bin/gcc-15 100 \
      --slave   /usr/bin/g++ g++ /usr/bin/g++-15 && \
    update-alternatives \
      --install /usr/bin/riscv64-linux-gnu-gcc riscv64-linux-gnu-gcc \
                /usr/bin/riscv64-linux-gnu-gcc-15 100 \
      --slave   /usr/bin/riscv64-linux-gnu-g++ riscv64-linux-gnu-g++ \
                /usr/bin/riscv64-linux-gnu-g++-15 && \
    update-alternatives \
      --install /usr/bin/aarch64-linux-gnu-gcc aarch64-linux-gnu-gcc \
                /usr/bin/aarch64-linux-gnu-gcc-15 100 \
      --slave   /usr/bin/aarch64-linux-gnu-g++ aarch64-linux-gnu-g++ \
                /usr/bin/aarch64-linux-gnu-g++-15 && \
    update-alternatives \
      --install /usr/bin/arm-linux-gnueabihf-gcc arm-linux-gnueabihf-gcc \
                /usr/bin/arm-linux-gnueabihf-gcc-15 100 \
      --slave   /usr/bin/arm-linux-gnueabihf-g++ arm-linux-gnueabihf-g++ \
                /usr/bin/arm-linux-gnueabihf-g++-15 && \
    update-alternatives \
      --install /usr/bin/x86_64-linux-gnu-gcc x86_64-linux-gnu-gcc \
                /usr/bin/x86_64-linux-gnu-gcc-15 100 \
      --slave   /usr/bin/x86_64-linux-gnu-g++ x86_64-linux-gnu-g++ \
                /usr/bin/x86_64-linux-gnu-g++-15 && \
    apt-get autoremove -y && apt-get clean -y && \
    rm -rf /var/lib/apt/lists/*
