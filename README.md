# SimpleKernel

## 简介 INTRODUCE

一个用来练手的简单内核.
A simple kernel for practice.

- bootloader


    使用 GNU Grub 进行引导. Loaded by GRUB.
    


## 开发环境 DEVELOPMENTENVIRONMENT

- macOS High Sierra 10.13.3
- Bochs x86 Emulator 2.6.9

        brew install bochs


- GNU toolchain

        brew install binutils
        
- gcc

        brew install gcc

- GRUB

        git clone git://git.savannah.gnu.org/grub.git



## 文件说明
- setup.sh
- Makefile
- link.ld
- bochsrc.txt
- bochsout.txt
- 


## 参考资料 REFERENCES

[JamesM's kernel development tutorials](http://www.jamesmolloy.co.uk/tutorial_html/1.-Environment%20setup.html)



[OS X Man Pages: hdiutil](https://developer.apple.com/legacy/library/documentation/Darwin/Reference/ManPages/man1/hdiutil.1.html)

[xOS](https://github.com/fengleicn/xOS)

