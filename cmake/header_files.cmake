
# This file is a part of Simple-XX/SimpleKernel
# (https://github.com/Simple-XX/SimpleKernel).
# Based on https://github.com/SynestiaOS/SynestiaOS
# CMakeLists.txt for Simple-XX/SimpleKernel.
# 将头文件路径添加到 Target 的搜索路径中

function(target_include_arch_header_files Target)
    target_include_directories(${Target} PRIVATE ${SimpleKernel_SOURCE_CODE_DIR}/arch/${SimpleKernelArch}/)
endfunction()

function(target_include_kernel_header_files Target)
    target_include_directories(${Target} PRIVATE ${SimpleKernel_SOURCE_CODE_DIR}/kernel/include)
endfunction()
