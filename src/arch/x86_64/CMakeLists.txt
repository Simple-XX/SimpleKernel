
# This file is a part of SimpleXX/SimpleKernel (https://github.com/SimpleXX/SimpleKernel).
#
# CMakeLists.txt for SimpleXX/SimpleKernel.

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/boot)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/gdt)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/intr)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/debug)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/mm)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/heap)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/sched)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/task)
