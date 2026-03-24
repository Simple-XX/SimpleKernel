# Copyright The SimpleKernel Contributors

ADD_LIBRARY (device INTERFACE)

ADD_SUBDIRECTORY (ns16550a)
ADD_SUBDIRECTORY (pl011)
ADD_SUBDIRECTORY (acpi)
ADD_SUBDIRECTORY (virtio)

TARGET_INCLUDE_DIRECTORIES (device INTERFACE include)

TARGET_SOURCES (device INTERFACE device.cpp device_manager.cpp)

TARGET_LINK_LIBRARIES (device INTERFACE ns16550a_driver pl011_driver
                                        acpi_driver virtio_driver)
