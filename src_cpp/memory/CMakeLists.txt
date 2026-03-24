# Copyright The SimpleKernel Contributors

ADD_LIBRARY (memory INTERFACE)

TARGET_INCLUDE_DIRECTORIES (memory INTERFACE include)

TARGET_SOURCES (memory INTERFACE memory.cpp virtual_memory.cpp)
