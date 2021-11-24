
# This file is a part of Simple-XX/SimpleKernel
# (https://github.com/Simple-XX/SimpleKernel).
# Based on https://android.googlesource.com/platform/bionic.git/+/gingerbread/libc/private/__dso_handle.S
# __dso_handle.S for Simple-XX/SimpleKernel.

# The __dso_handle global variable is used by static
# C++ constructors and destructors in the binary.
# See http://www.codesourcery.com/public/cxx-abi/abi.html#dso-dtor

.section .bss
.align 4
#ifndef CRT_LEGACY_WORKAROUND
.hidden __dso_handle
#endif
.globl __dso_handle
__dso_handle:
    .long 0
