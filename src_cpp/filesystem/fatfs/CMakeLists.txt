# Copyright The SimpleKernel Contributors

ADD_LIBRARY (fatfs INTERFACE)

TARGET_INCLUDE_DIRECTORIES (fatfs BEFORE INTERFACE include)

TARGET_SOURCES (fatfs INTERFACE diskio.cpp fatfs.cpp)

# @todo 原作者的 fatfs 库默认会包含一个 ffconf.h，但是没办法优雅地覆盖它，计划使用 c++ 重写
TARGET_COMPILE_OPTIONS (
    fatfs INTERFACE -include ${CMAKE_CURRENT_SOURCE_DIR}/include/ffconf.h)
