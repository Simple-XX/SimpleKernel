
# This file is a part of Simple-XX/SimpleKernel (https://github.com/Simple-XX/SimpleKernel).
#
# CMakeLists.txt for Simple-XX/SimpleKernel.

PROJECT(init C ASM)

find_asm_source_files(ASM_SOURCE_FILES ${init_SOURCE_DIR})

set(init_src ${ASM_SOURCE_FILES})

add_library(${PROJECT_NAME} OBJECT ${init_src})
