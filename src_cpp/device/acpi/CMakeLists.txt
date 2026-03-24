# Copyright The SimpleKernel Contributors

ADD_LIBRARY (acpi_driver INTERFACE)

TARGET_INCLUDE_DIRECTORIES (acpi_driver
                            INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/..)
