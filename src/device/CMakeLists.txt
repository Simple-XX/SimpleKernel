
# This file is a part of Simple-XX/SimpleKernel (https://github.com/Simple-XX/SimpleKernel).
#
# CMakeLists.txt for Simple-XX/SimpleKernel.

PROJECT(dev CXX)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/ dev_cxx_src)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/platform_bus platform_bus_src)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/virtio virtio_src)
set(dev_src ${dev_cxx_src} ${platform_bus_src} ${virtio_src})

add_library(${PROJECT_NAME} OBJECT ${dev_src})

target_include_libc_header_files(${PROJECT_NAME})
target_include_libcxx_header_files(${PROJECT_NAME})
target_include_kernel_header_files(${PROJECT_NAME})
target_include_drv_header_files(${PROJECT_NAME})
target_include_dev_header_files(${PROJECT_NAME})
target_include_common_header_files(${PROJECT_NAME})
target_include_arch_header_files(${PROJECT_NAME})
