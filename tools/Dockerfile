
# This file is a part of Simple-XX/SimpleKernel
# (https://github.com/Simple-XX/SimpleKernel).
#
# Dockerfile for Simple-XX/SimpleKernel.

FROM ubuntu:latest AS basic_os
RUN export DEBIAN_FRONTEND=noninteractive && apt update && apt upgrade -y
ENV LANG=C.UTF-8
ENV SSL_CERT_FILE=/etc/ssl/certs/ca-certificates.crt

FROM basic_os AS basic_tools
RUN apt install --no-install-recommends --fix-missing -y \
    locales \
    ca-certificates \
    git \
    curl \
    wget \
    openssh-server \
    rsync \
    sudo \
    zsh \
    zip \
    unzip \
    telnet \
    tar
RUN apt install --no-install-recommends --fix-missing -y \
    file \
    vim \
    doxygen \
    valgrind \
    graphviz \
    clang-format \
    clang-tidy \
    cppcheck \
    lcov
RUN locale-gen en_US.UTF-8 && locale-gen zh_CN.UTF-8 \
    && mkdir -p /var/run/sshd \
    && echo 'PasswordAuthentication yes' >> /etc/ssh/sshd_config \
    && echo 'PubkeyAuthentication yes' >> /etc/ssh/sshd_config \
    && echo 'PermitRootLogin yes' >> /etc/ssh/sshd_config \
    && ssh-keygen -A \
    && service ssh restart \
    && git clone https://github.com/robbyrussell/oh-my-zsh.git ~/.oh-my-zsh \
    && cp ~/.oh-my-zsh/templates/zshrc.zsh-template ~/.zshrc \
    && cp -r ~/.oh-my-zsh /home/ubuntu/ \
    && cp ~/.oh-my-zsh/templates/zshrc.zsh-template /home/ubuntu/.zshrc \
    && chsh -s /bin/zsh root \
    && usermod -s /bin/zsh root \
    && chsh -s /bin/zsh ubuntu \
    && usermod -s /bin/zsh ubuntu

FROM basic_tools AS dev_tools
RUN apt install --no-install-recommends --fix-missing -y \
    build-essential \
    binutils \
    make \
    cmake \
    git \
    qemu-system \
    gdb-multiarch \
    libgtest-dev \
    pkg-config \
    flex \
    bison
RUN apt install --no-install-recommends --fix-missing -y \
    gcc \
    g++ \
    gcc-riscv64-linux-gnu \
    g++-riscv64-linux-gnu \
    gcc-aarch64-linux-gnu \
    g++-aarch64-linux-gnu \
    gcc-riscv64-linux-gnu \
    g++-riscv64-linux-gnu \
    gcc-x86-64-linux-gnu \
    g++-x86-64-linux-gnu

FROM dev_tools AS simple_kernel
EXPOSE 22
USER root
WORKDIR /root
RUN apt autoremove -y && apt clean -y && rm -rf /var/lib/apt/lists/* \
    && git config --global --add safe.directory /root/SimpleKernel
ENTRYPOINT ["/bin/zsh"]
