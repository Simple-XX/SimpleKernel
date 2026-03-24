# Copyright The SimpleKernel Contributors

ADD_LIBRARY (ns16550a_driver INTERFACE)

TARGET_INCLUDE_DIRECTORIES (ns16550a_driver
                            INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/..)
