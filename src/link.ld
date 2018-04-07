/* Copy from JamesM's kernel development tutorials:*/
// (http://www.jamesmolloy.co.uk/tutorial_html/1.-Environment%20setup.html).

// This file is apart of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).


/* Link.ld -- Linker script for the kernel - ensure everything goes in the */
/*            Correct place.  */
/*            Original file taken from Bran's Kernel Development */
/*            tutorials: http://www.osdever.net/bkerndev/index.php. */

OUTPUT_FORMAT(elf32-i386)
ENTRY(start)
SECTIONS
 {
   . = 0x100000;
   .text : { *(.text) }
   .data : { *(.data) }
   .bss  : { *(.bss)  }
 }
