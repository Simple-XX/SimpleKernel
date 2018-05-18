# SimpleKernel

## 简介 INTRODUCE

一个用来练手的简单内核.
A simple kernel for practice.

使用 GRUB 进行引导. 
Loaded by GRUB.

## 开发环境 DEVELOPMENTENVIRONMENT

- macOS High Sierra 10.13.3
- Bochs x86 Emulator 2.6.9

        brew install bochs


- GCC Cross-Compiler

        sudo port install i386-elf-gcc

- GRUB

1. clone 源代码

        git clone git://git.savannah.gnu.org/grub.git


2.  下载 `objconv` 工具

        https://github.com/vertis/objconv
        
    下载好了编译，把路径加到 `PATH` 里
3. 在 GRUB 目录下运行 `autogen.sh`
4. 在与 GRUB 同级的目录下，新建一个文件夹，在该目录下执行以下脚本：

        ../grub/configure --disable-werror TARGET_CC=i386-elf-gcc TARGET_OBJCOPY=i386-elf-objcopy \
        TARGET_STRIP=i386-elf-strip TARGET_NM=i386-elf-nm TARGET_RANLIB=i386-elf-ranlib --target=i386-elf
          
5. Run `make` and `make install`

搞定

ps. 实际上我没有成功，不知道是哪里的问题，所以我在别处找了个镜像，先用着。

### 参考资料 REFERENCES

[交叉编译](https://wiki.osdev.org/GCC_Cross-Compiler)

[GRUB 在 Mac 上的安装](https://wiki.osdev.org/GRUB#Installing_GRUB_2_on_OS_X)

## BootLoader
### 文件说明
- setup.sh

    把 boot.img 挂载到当前目录，然后将 kernel.img 写入 boot 目录，以 bochrc.txt 为配置文件，运行 bochs。

- bochsrc.txt

    bochs 配置文件


- bochsout.txt

    bochs 输出

- Makefile

    指定源文件的生成规则，最终生成 kernel.
    
- link.ld

    链接脚本，规定了生成程序地址空间的布局.
    
- main.c

    操作系统入口程序

### 参考资料 REFERENCES

[JamesM's kernel development tutorials](http://www.jamesmolloy.co.uk/tutorial_html/1.-Environment%20setup.html)

[OS X Man Pages: hdiutil](https://developer.apple.com/legacy/library/documentation/Darwin/Reference/ManPages/man1/hdiutil.1.html)

[xOS](https://github.com/fengleicn/xOS)

[系统初始化](https://wiki.osdev.org/Bare_Bones)



