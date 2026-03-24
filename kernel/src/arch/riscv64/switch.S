#include "macro.S"

.section .text

.global switch_to
.type switch_to, @function

.global kernel_thread_entry
.type kernel_thread_entry, @function

.extern kernel_thread_bootstrap

switch_to:
    SaveCalleeSavedContext a0
    RestoreCalleeSavedContext a1
    ret

kernel_thread_entry:
    mv a0, s0
    mv a1, s1
    call kernel_thread_bootstrap
