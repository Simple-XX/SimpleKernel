
# This file is a part of Simple-XX/SimpleKernel
# (https://github.com/Simple-XX/SimpleKernel).
# Based on https://github.com/SynestiaOS/SynestiaOS
# CMakeLists.txt for Simple-XX/SimpleKernel.
# 在 Directory 目录下寻找 .s/.S 格式的文件，并添加到 OutValue 中

macro(find_asm_source_files OutValue Directory)
    file(GLOB ${OutValue} LIST_DIRECTORIES false ${Directory}/*.[sS])
    message(STATUS "found asm files: ${${OutValue}}")
endmacro()
