
# This file is a part of SimpleXX/SimpleKernel (https://github.com/SimpleXX/SimpleKernel).
#
# CMakeLists.txt for SimpleXX/SimpleKernel.

include_directories(./include)

set(cfiles test.c)

c2o("${cfiles}")
