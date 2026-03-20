# Copyright The SimpleKernel Contributors

ADD_LIBRARY (arch INTERFACE)

TARGET_INCLUDE_DIRECTORIES (arch INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
                                           ${CMAKE_SYSTEM_PROCESSOR}/include)

TARGET_SOURCES (
    arch
    INTERFACE ${CMAKE_SYSTEM_PROCESSOR}/boot.S
              ${CMAKE_SYSTEM_PROCESSOR}/arch_main.cpp
              ${CMAKE_SYSTEM_PROCESSOR}/early_console.cpp
              ${CMAKE_SYSTEM_PROCESSOR}/backtrace.cpp
              ${CMAKE_SYSTEM_PROCESSOR}/interrupt.S
              ${CMAKE_SYSTEM_PROCESSOR}/switch.S
              ${CMAKE_SYSTEM_PROCESSOR}/interrupt.cpp
              ${CMAKE_SYSTEM_PROCESSOR}/timer.cpp
              ${CMAKE_SYSTEM_PROCESSOR}/interrupt_main.cpp
              ${CMAKE_SYSTEM_PROCESSOR}/syscall.cpp)

IF(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "riscv64")
    ADD_SUBDIRECTORY (${CMAKE_SYSTEM_PROCESSOR}/plic)
ELSEIF(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "aarch64")
    ADD_SUBDIRECTORY (${CMAKE_SYSTEM_PROCESSOR}/gic)
ENDIF()

TARGET_LINK_LIBRARIES (
    arch
    INTERFACE $<$<STREQUAL:${CMAKE_SYSTEM_PROCESSOR},riscv64>:
              plic>
              $<$<STREQUAL:${CMAKE_SYSTEM_PROCESSOR},aarch64>:
              gic>
              # Documented exception: arch code needs direct driver access
              # aarch64: pl011_driver (early_console, interrupt_main)
              # riscv64: ns16550a_driver, virtio_driver (interrupt_main)
              $<$<STREQUAL:${CMAKE_SYSTEM_PROCESSOR},aarch64>:
              pl011_driver>
              $<$<STREQUAL:${CMAKE_SYSTEM_PROCESSOR},riscv64>:
              ns16550a_driver>
              $<$<STREQUAL:${CMAKE_SYSTEM_PROCESSOR},riscv64>:
              virtio_driver>)
