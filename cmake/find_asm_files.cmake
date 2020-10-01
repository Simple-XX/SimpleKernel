
# This file is a part of SimpleXX/SimpleKernel (https://github.com/SimpleXX/SimpleKernel).
# 
# find_asm_files.cmake for SimpleXX/SimpleKernel.

macro(find_asm_source_files OutValue Directory)
    file(GLOB ${OutValue} LIST_DIRECTORIES false ${Directory}/*.[sS])
    message(STATUS "found asm files: ${${OutValue}}")
endmacro()
