#!/bin/bash

# This file is a part of Simple-XX/SimpleKernel
# (https://github.com/Simple-XX/SimpleKernel).
#
# docker.sh for Simple-XX/SimpleKernel.
# 运行预制了工具链的 docker

# shell 执行出错时终止运行
set -e
# 输出实际执行内容
set -x

docker pull ptrnull233/simple_kernel:latest
docker run --name SimpleKernel-container -itd -p 233:22 -v ./:/root/SimpleKernel -v ~/.ssh:/root/.ssh -v ~/.gitconfig:/root/.gitconfig  ptrnull233/simple_kernel:latest
docker exec -it SimpleKernel-container /bin/zsh
