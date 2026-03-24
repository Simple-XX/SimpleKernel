# Copyright The SimpleKernel Contributors

ADD_LIBRARY (pl011_driver INTERFACE)

TARGET_INCLUDE_DIRECTORIES (pl011_driver
                            INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/..)
