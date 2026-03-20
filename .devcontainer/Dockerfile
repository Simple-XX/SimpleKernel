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
    uuid-dev \
    libgnutls28-dev \
    libssl-dev \
    swig \
    libgtest-dev \
    doxygen \
    graphviz \
    lcov \
    qemu-system-arm \
    qemu-system-misc \
    gdb-multiarch \
    gcc-14 g++-14 \
    gcc-14-riscv64-linux-gnu g++-14-riscv64-linux-gnu \
    gcc-14-aarch64-linux-gnu g++-14-aarch64-linux-gnu \
    gcc-14-arm-linux-gnueabihf g++-14-arm-linux-gnueabihf && \
    update-alternatives \
      --install /usr/bin/gcc gcc /usr/bin/gcc-14 100 \
      --slave   /usr/bin/g++ g++ /usr/bin/g++-14 \
      --slave   /usr/bin/gcov gcov /usr/bin/gcov-14 && \
    update-alternatives \
      --install /usr/bin/riscv64-linux-gnu-gcc riscv64-linux-gnu-gcc \
                /usr/bin/riscv64-linux-gnu-gcc-14 100 \
      --slave   /usr/bin/riscv64-linux-gnu-g++ riscv64-linux-gnu-g++ \
                /usr/bin/riscv64-linux-gnu-g++-14 && \
    update-alternatives \
      --install /usr/bin/aarch64-linux-gnu-gcc aarch64-linux-gnu-gcc \
                /usr/bin/aarch64-linux-gnu-gcc-14 100 \
      --slave   /usr/bin/aarch64-linux-gnu-g++ aarch64-linux-gnu-g++ \
                /usr/bin/aarch64-linux-gnu-g++-14 \
      --slave   /usr/bin/aarch64-linux-gnu-cpp aarch64-linux-gnu-cpp \
                /usr/bin/aarch64-linux-gnu-cpp-14 && \
    update-alternatives \
      --install /usr/bin/arm-linux-gnueabihf-gcc arm-linux-gnueabihf-gcc \
                /usr/bin/arm-linux-gnueabihf-gcc-14 100 \
      --slave   /usr/bin/arm-linux-gnueabihf-g++ arm-linux-gnueabihf-g++ \
                /usr/bin/arm-linux-gnueabihf-g++-14 && \
    apt-get autoremove -y && apt-get clean -y && \
    rm -rf /var/lib/apt/lists/* && \
    mkdir -p /srv/tftp
