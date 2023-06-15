POSIX-UEFI Segédeszközök
========================

Ezek kis parancsok, amik besegítenek az UEFI toolchain-edbe (csak libc-t használnak, nincs szükség EFI hedörökre). A POSIX-UEFI
által lefordított .efi kimeneti fájlokat konvertálják olyan különböző formátumú fájlokká, amiket az UEFI firmware használ.

* __efirom__ - ez PCI Option ROM képet készít
```
POSIX-UEFI utils - efirom by Michael Brown GPL

./efirom [--vendor=VVVV] [--device=DDDD] bementifájl kimentifájl
```

* __efiffs__ - ez DXE UEFI eszközmeghajtó képet készít (bővebb infó [ebben a wikiben](https://github.com/pbatard/efifs/wiki/Adding-a-driver-to-a-UEFI-firmware#adding-the-module-to-the-firmware) található arról, hogy hogyan kell a firmwarehez adni).
```
POSIX-UEFI utils - efiffs by bztsrc@gitlab MIT

./efiffs [-g <guid>] [-n <név>] [-v <ver>] [-t <típus>] [-p <típus>] bemenet [kimenet]

  -g <guid>   a GUID megadása (alapértelmezetten véletlenszám)
  -n <név>    az eszközmeghajtó neve (pl 'FAT')
  -v <ver>    az eszközmeghajtó verziója (pl '1.0')
  -t <típus>  az ffs típusa (alapértelmezetten 7, EFI_FV_FILETYPE_DRIVER)
  -p <típus>  a pe szekció típusa (alapértelmezetten 16, EFI_SECTION_PE32)
  bemenet     a bemeneti .efi fájl neve
  kimenet     a kimeneti fájl neve (alapértelmezetten a bemeneti névből generált)
```

* __efidsk__ - indítható lemezképet készít EFI Rendszer Partícióval egy könyvtár tartalmából. A POSIX-EFI-vel lefordított
programodat `EFI/BOOT/BOOTX64.EFI` néven kell bemásolni a könyvtárba, hogy magától elinduljon.
```
POSIX-UEFI utils - efidsk by bztsrc@gitlab MIT

./efidsk [-p|-c] [-s <méret>] bekönyvtár kimenet

  -p          csak a partíciót mentse, GPT nélkül
  -c          EFI CDROM mentése (ISO9660 El Torito no emulation boot catalog)
  -s <méret>  a partíció méretének megadása megabájtban (alapértelmezetten 33M)
  bekönvtár   ennek a könyvtárnak a tartalmából generálja
  kimenet     kimenti lemezképfájl neve
```
