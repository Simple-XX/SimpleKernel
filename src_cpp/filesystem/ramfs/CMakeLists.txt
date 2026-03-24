# Copyright The SimpleKernel Contributors

ADD_LIBRARY (ramfs INTERFACE)

TARGET_INCLUDE_DIRECTORIES (ramfs INTERFACE include)

TARGET_SOURCES (ramfs INTERFACE ramfs.cpp)
