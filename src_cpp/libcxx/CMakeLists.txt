# Copyright The SimpleKernel Contributors

ADD_LIBRARY (libcxx INTERFACE)

TARGET_INCLUDE_DIRECTORIES (libcxx INTERFACE include)

TARGET_SOURCES (libcxx INTERFACE kstd_libcxx.cpp kstd_new.cpp)
