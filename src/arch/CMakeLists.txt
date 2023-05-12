
# This file is a part of Simple-XX/SimpleKernel
# (https://github.com/Simple-XX/SimpleKernel).
#
# CMakeLists.txt for Simple-XX/SimpleKernel.
# 根据 SimpleKernelArch 判断跳转到哪个目录

# 如果是 ia32/i386 或 ia32/x86_64，跳转到 ia32 目录
if (SimpleKernelArch STREQUAL "ia32/i386" OR SimpleKernelArch STREQUAL "ia32/x86_64")
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/ia32)
else ()
    # 其它情况跳转到与 SimpleKernelArch 同名目录即可
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/${SimpleKernelArch})
endif ()
