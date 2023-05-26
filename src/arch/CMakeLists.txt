
# This file is a part of Simple-XX/SimpleKernel
# (https://github.com/Simple-XX/SimpleKernel).
#
# CMakeLists.txt for Simple-XX/SimpleKernel.
# 根据 SimpleKernelArch 判断跳转到哪个目录

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/${ARCH})
