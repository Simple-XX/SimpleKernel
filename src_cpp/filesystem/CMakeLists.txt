# Copyright The SimpleKernel Contributors

ADD_LIBRARY (filesystem INTERFACE)

TARGET_INCLUDE_DIRECTORIES (filesystem INTERFACE include)

TARGET_SOURCES (filesystem INTERFACE file_descriptor.cpp filesystem.cpp)

ADD_SUBDIRECTORY (vfs)
ADD_SUBDIRECTORY (ramfs)
ADD_SUBDIRECTORY (fatfs)

TARGET_LINK_LIBRARIES (filesystem INTERFACE vfs ramfs fatfs device)
