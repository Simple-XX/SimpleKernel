# Copyright The SimpleKernel Contributors

ADD_LIBRARY (libc INTERFACE)

TARGET_INCLUDE_DIRECTORIES (libc INTERFACE include)

TARGET_SOURCES (libc INTERFACE sk_stdlib.c sk_string.c sk_ctype.c)
