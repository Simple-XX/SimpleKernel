// Copy from JamesM's kernel development tutorials:
// (http://www.jamesmolloy.co.uk/tutorial_html/1.-Environment%20setup.html).

// This file is apart of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).


// main.c -- Defines the C-code kernel entry point, calls initialisation routines.
// Made for JamesM's tutorials

int main(struct multiboot *mboot_ptr)
{
  // All our initialisation calls will go in here.
  return 0xDEADBABA;
}
