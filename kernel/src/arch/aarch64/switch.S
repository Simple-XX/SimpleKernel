#include "macro.S"

.section .text
.global switch_to
.type switch_to, @function

switch_to:
    SaveCalleeSavedContext x0
    RestoreCalleeSavedContext x1
    ret

.global kernel_thread_entry
.type kernel_thread_entry, @function
.extern kernel_thread_bootstrap
kernel_thread_entry:
    mov x0, x19
    mov x1, x20
    bl kernel_thread_bootstrap
