
# This file is a part of Simple-XX/SimpleKernel
# (https://github.com/Simple-XX/SimpleKernel).
#
# Dockerfile for Simple-XX/SimpleKernel.

FROM ubuntu:latest

# 替换为你想要的用户名
ARG user=zone
ARG password=zone

# 元数据
LABEL maintainer="Zone.N" email="zone.niuzh@hotmail.com"

# 安装所有依赖
RUN DEBIAN_FRONTEND=noninteractive \
    && apt update \
    && apt install --no-install-recommends --fix-missing -y \
        curl \
        wget \
        sudo \
        zsh \
        zip \
        openssh-server \
        rsync \
        tar \
        git \
        vim \
        doxygen \
        graphviz \
        make \
        cmake \
        clang-format \
        qemu-system \
        build-essential \
        binutils \
        valgrind \
        gdb-multiarch \
        gcc \
        g++ \
        gcc-riscv64-linux-gnu \
        g++-riscv64-linux-gnu \
    && apt clean \
    && git config --global --add safe.directory '*' \
    && sh -c "$(curl -fsSL https://raw.githubusercontent.com/ohmyzsh/ohmyzsh/master/tools/install.sh)"

# 设置 ssh
RUN mkdir -p /var/run/sshd \
    && echo 'PasswordAuthentication yes' >> /etc/ssh/sshd_config \
    && echo 'PubkeyAuthentication yes' >> /etc/ssh/sshd_config \
    && echo 'PermitRootLogin yes' >> /etc/ssh/sshd_config \
    && ssh-keygen -A

# 添加用户
RUN useradd --create-home --no-log-init --shell /bin/zsh ${user} \
    && adduser ${user} sudo \
    && echo "${user}:${password}" | chpasswd

# 指定容器登录用户
# USER ${user}

# 指定容器起来的工作目录
WORKDIR /home/${user}

# 开放 22 端口
EXPOSE 22

# 启动 ssh 服务
ENTRYPOINT service ssh restart && bash
