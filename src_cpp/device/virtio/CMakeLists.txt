# Copyright The SimpleKernel Contributors

ADD_LIBRARY (virtio_driver INTERFACE)

TARGET_INCLUDE_DIRECTORIES (virtio_driver
                            INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/..)

TARGET_SOURCES (virtio_driver INTERFACE virtio_driver.cpp)
