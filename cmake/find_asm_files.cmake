
# This file is a part of Simple-XX/SimpleKernel (https://github.com/Simple-XX/SimpleKernel).
# 
# find_asm_files.cmake for Simple-XX/SimpleKernel.

macro(find_asm_source_files OutValue Directory)
    file(GLOB ${OutValue} LIST_DIRECTORIES false ${Directory}/*.[sS])
    message(STATUS "found asm files: ${${OutValue}}")
endmacro()
