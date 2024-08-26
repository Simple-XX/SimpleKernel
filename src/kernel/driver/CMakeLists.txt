
# This file is a part of Simple-XX/SimpleKernel
# (https://github.com/Simple-XX/SimpleKernel).
#
# CMakeLists.txt for Simple-XX/SimpleKernel.


add_library(driver INTERFACE)

target_include_directories(driver INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_sources(driver INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/driver.cpp
)

target_link_libraries(driver INTERFACE
        $<$<STREQUAL:${CMAKE_SYSTEM_PROCESSOR},riscv64>:
                ns16550a
        >
)

if (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "riscv64")
    add_subdirectory(
            ns16550a
    )
endif ()
