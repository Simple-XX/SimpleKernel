POSIX-UEFI Utilities
====================

These are small portable commands to help you with the UEFI toolchain (libc only, no EFI headers needed). They convert the .efi
output that you've compiled with POSIX-UEFI into different file formats required by the UEFI firmware.

* __efirom__ - creates a PCI Option ROM image
```
POSIX-UEFI utils - efirom by Michael Brown GPL

./efirom [--vendor=VVVV] [--device=DDDD] infile outfile
```

* __efiffs__ - creates a DXE UEFI driver image (see [this wiki](https://github.com/pbatard/efifs/wiki/Adding-a-driver-to-a-UEFI-firmware#adding-the-module-to-the-firmware) on how to add it to a firmware).
```
POSIX-UEFI utils - efiffs by bztsrc@gitlab MIT

./efiffs [-g <guid>] [-n <name>] [-v <ver>] [-t <type>] [-p <type>] infile [outfile]

  -g <guid>   specify the GUID (defaults to random)
  -n <name>   specify the driver's name (eg 'FAT')
  -v <ver>    specify the driver's version (eg '1.0')
  -t <type>   specify the ffs type (defaults to 7, EFI_FV_FILETYPE_DRIVER)
  -p <type>   specify the pe section type (defaults to 16, EFI_SECTION_PE32)
  infile      input .efi file
  outfile     output file name (default generated from infile)
```

* __efidsk__ - creates a bootable disk image with EFI System Partition from the contents of a directory. Copy the POSIX-UEFI
compiled file under the given directory as `EFI/BOOT/BOOTX64.EFI` to get it automatically booted.
```
POSIX-UEFI utils - efidsk by bztsrc@gitlab MIT

./efidsk [-p|-c] [-s <size>] indir outfile

  -p          save only the partition image without GPT
  -c          save EFI CDROM (ISO9660 El Torito no emulation boot catalog)
  -s <size>   set the size of partition in megabytes (defaults to 33M)
  indir       use the contents of this directory
  outfile     output image file name
```
