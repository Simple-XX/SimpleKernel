# Copyright The SimpleKernel Contributors

ADD_LIBRARY (vfs INTERFACE)

TARGET_INCLUDE_DIRECTORIES (vfs INTERFACE include)

TARGET_SOURCES (
    vfs
    INTERFACE vfs.cpp
              mount.cpp
              lookup.cpp
              open.cpp
              close.cpp
              read.cpp
              write.cpp
              seek.cpp
              mkdir.cpp
              rmdir.cpp
              unlink.cpp
              readdir.cpp)
