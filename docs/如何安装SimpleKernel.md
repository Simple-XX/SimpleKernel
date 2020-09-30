# 如何安装SimpleKernel



安装本项目主要使用linux或者mac进行，使用的工具有:

- bochs:一个x86硬件平台的开源模拟器。它可以模拟各种硬件的配置，我们的操作系统就跑在这个平台上，有了它可以轻松管理和查看相关配置，并且可以保护。

- xorriso:一个用来创建和操作 ISO 9660 文件系统（光盘格式）映像文件的工具，可以用来将映像文件烧录到 CD/DVD 光盘中。

- i386-elf tool chain

  - i386-elf-binutils：
    - binutils 是一套binary utilities,主要包括:
      - **ld** - the GNU linker.
      - **as** - the GNU assembler.
  - i386-elf-gcc：一系列的编译器集 包括有c cpp
  - i386-elf-grub：grub

  

  在我们安装os时候需要使用shell 脚本来进行安装，这样的好处是可以方便安装，把所有linux 的命令都写在.sh 的文件中，所以我们需要了解一些基本的linux shell 脚本的知识。

  

  #### Shell 脚本背景知识

  shell可以理解为一门编程语言，只不过这门语言主要是用于和shell交互，在服务器运维和批量化安装处理相关软件。我们以repo中的setup.sh 为例来进行说明。可以阅读其中的注释，来理解整体脚本的作用。

  当有一些不懂的命令时，可以在互联网上进行检索，大多数的命令均可以查询到，主要是bochs的基本使用，elf tool的使用方法。

  ```shell
  
  # This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
  #
  # setup.sh for MRNIU/SimpleKernel.
  
  #!/usr/bin/env bash
  # shell 执行出错时终止运行
  # set -e
  # 输出实际执行内容
  # set -x
  # 这里主要是设置一些环境变量，source意为在当前bash环境下读bai取并执行FileName中的命令。
  source ./tools/env.sh
  
  # 重新编译
  cd src/
  make remake
  cd ../
  
  # 这里看到的${xx}为变量，可以用echo ${xx}来显示变量中存储的信息。
  # 检查该文件的grub的类型是否是multiboot2
  if ${GEUB}-file --is-x86-multiboot2 ${kernel}; then
      echo Multiboot2 Confirmed!
  elif [ ${ARCH} == "x86_64" ]; then
      if ${GEUB}-file --is-x86-multiboot2 ${bootloader}; then
          echo Multiboot2 Confirmed!
      else
          echo The File is Not Multiboot.
          exit
      fi
  fi
  # rm：删除文件
  # cp 复制
  # mkdir 新建文件夹
  # touch 新建文件
  rm -rf -f ${iso_boot}/*
  cp ${kernel} ${iso_boot}
  mkdir ${iso_boot_grub}
  touch ${iso_boot_grub}/grub.cfg
  if [ ${ARCH} == "x86_64" ]; then
      cp ${bootloader} ${iso_boot}
      echo 'set timeout=15
      set default=0
      menuentry "SimpleKernel" {
         multiboot2 /boot/bootloader.bin
         module /boot/kernel.bin "KERNEL_BIN"
     }' >${iso_boot_grub}/grub.cfg
  else
      echo 'set timeout=15
      set default=0
      menuentry "SimpleKernel" {
         multiboot2 /boot/kernel.bin "KERNEL_BIN"
     }' >${iso_boot_grub}/grub.cfg
  fi
  i386-elf-grub-mkrescue -o ${iso} ${iso_folder}
  ${SIMULATOR} -q -f ${bochsrc} -rc ./tools/bochsinit
  
  ```

  

