POSIX-UEFI
==========

<blockquote>Hányunk attól az ocsmány UEFI API-tól, a megszokott POSIX-ot akarjuk!</blockquote>

Ez egy nagyon minimális fordító környezet, ami segít UEFI-re fejleszteni Linux (vagy más POSIX kompatíbilis) rendszer alatt.
Nagy hatással volt rá a [gnu-efi](https://sourceforge.net/projects/gnu-efi) (minden elismerésem nekik), de annál kissebb,
egyszerűbb és könnyebben integrálható (működik LLVM Clang és GNU gcc környezettel is), valamint könnyebben is használható,
mivel POSIX-szerű API-t biztosít az UEFI alkalmazásod felé.

Az UEFI környezet két komponensből áll: a GUID protokoll interfészű firmverből és egy felhasználói könyvtárból. Az előbbit
nem tudjuk lecserélni, de az utóbbit barátságosabbá tehetjük. És pont ez az, amit a POSIX-UEFI csinál. Egy vékony
függvénykönyvtár ami becsomagolja a GUID protokollos API-t, nem több, nem kevesebb, nem egy teljes értékű libc implementáció.

Kétféleképp tudod integrálni a projektedbe:

Statikus Függvénykönyvtárként
-----------------------------

Azonos metódus, mint a gnu-efi-nél, nem igazán javasolt. Az `uefi` könyvtárban futtasd a következő parancsot
```sh
$ USE_GCC=1 make
```
Ez létrehozza az `build/uefi` mappát, minden szükséges fájllal együtt. Ezek:

 - **crt0.o**, futásidejű kód, ami elind0tja a POSIX-UEFI-t
 - **link.ld**, linkelő szkript, amit a POSIX-UEFI-vel használni kell (ugyanaz, mint gnu-efi esetén)
 - **libuefi.a**, a függvénykönyvtár maga
 - **uefi.h**, minden az egyben C / C++ fejlécfájl

Ezzel összeszerkesztheted a programodat, de nem fogod tudni újrafordítani, és a linkeléssel és konvertálással is magadra
maradsz.

Szigorúan véve csak a **crt0.o** és a **link.ld** fájlokra van szükség, ez elég ahhoz, hogy elindítsa és meghívja az alkalmazásod
"main()" eljárását. Viszont ahhoz, hogy a libc funkciókat (mint pl. memcmp, strcpy, malloc vagy fopen) használhasd, linkelned
kell a **libuefi.a** fájllal is.

Egyenlőre ez a metódus csak gcc-vel működik, mivel a Clang úgy van beállítva, hogy direktben PE fájlokat hoz létre, ezért nem tud
statikus ELF .a fájlokat generálni, sem linkelni velük.

Forrásként terjesztve
---------------------

Ez a javasolt mód, mivel ez biztosít egy Makefile-t is ami megfelelően beállítja a fordítókörnyezetedet.

 1. másold be a `uefi` könyvtárat a forrásfádba (vagy állíts be egy git submodule-t és egy symlinket). Egy tucat fájl, kb. 132K összesen.
 2. csinálj egy hihetetlenül egyszerű **Makefile**-t, mint például az alábbi
 3. fordítsd le a programodat UEFI-re egy `make` hívással

```
TARGET = helloworld.efi
include uefi/Makefile
```
Egy minta **helloworld.c** így néz ki:
```c
#include <uefi.h>

int main(int argc, char **argv)
{
    printf("Hello World!\n");
    return 0;
}
```
Alapértelmezetten Clang + lld környezetet keres és állít be, ami direktben PE fájlt hoz létre konvertálás nélkül. Ha a `USE_GCC`
környezeti változó be van állítva, akkor a hoszt natív GNU gcc + ld használatával egy megosztott függvénykönyvtárat fordít, amit
aztán objcopy-val átkonvertál .efi fájllá, pont, mint ahogy a gnu-efi is csinálja.

**MEGJEGYZÉS**: ha nem akarod az egész repót klónozni, csakis az `uefi` könyvtárat, akkor
```
git clone --no-checkout https://gitlab.com/bztsrc/posix-uefi.git .
git sparse-checkout set --no-cone '/uefi/*'
git checkout
```

### Elérhető Makefile opciók

| Változó    | Leírás                                                                                               |
|------------|------------------------------------------------------------------------------------------------------|
| `TARGET`   | a cél program (szükséges megadni)                                                                    |
| `SRCS`     | források listája, amiket fordítani kell (alapértelmezetten minden \*.c \*.S fájl)                    |
| `CFLAGS`   | további fordító opciók (alapértelmezetten üres, pl. "-Wall -pedantic -std=c99")                      |
| `LDFLAGS`  | további linkelő opciók (nem hiszem, hogy valaha is szükség lesz erre, csak a teljesség kedvéért)     |
| `LIBS`     | további függvénykönyvtárak, amikkel linkelni szeretnél (pl "-lm", csak statikus .a jöhet szóba)      |
| `EXTRA`    | bármi további obj fájl, amit még hozzá szeretnél linkelni                                            |
| `ALSO`     | további make szabályok futtatása                                                                     |
| `OUTDIR`   | ha meg van adva, akkor ide generálja a projekted obj fájljait (alapértelmezetten nincs beállítva)    |
| `USE_GCC`  | ha beállítod, akkor natív GNU gcc + ld + objcopy környzetet használ LLVM Clang + Lld helyett        |
| `ARCH`     | a cél architektúra                                                                                   |

Itt van egy teljesebb **Makefile** példa:
```
ARCH = x86_64
TARGET = helloworld.efi
SRCS = $(wildcard *.c)
CFLAGS = -pedantic -Wall -Wextra -Werror --std=c11 -O2
LDFLAGS =
LIBS = -lm
OUTDIR = build/loader

USE_GCC = 1
include uefi/Makefile
```
A fordítási környezet konfiguráló úgy lett kialakítva, hogy akárhány architektúrával elboldogul, azonban eddig csak az
`x86_64` crt0 lett alaposan letesztelve. Van `aarch64` és `riscv64` crt0 is, de mivel nekem nincs sem ARM UEFI-s, sem
RISC-V UEFI-s gépem, **mindkettő teszteletlen**. Elvileg kéne működnie. Ha új architectúrára akarod portolni, akkor a
setjmp struct-ot kell megadni az uefi.h-ban, valamint csinálni neki egy crt0_X.c fájlt. Ennyi. Minden más
platformfüggetlenül lett megírva.

### Elérhető konfigurációs opciók

Ezeket az `uefi.h` elején lehet állítani.

| Define                | Leírás                                                                                    |
|-----------------------|-------------------------------------------------------------------------------------------|
| `UEFI_NO_UTF8`        | Ne használjon transzparens UTF-8 konverziót az alkalmazás és az UEFI interfész között     |
| `UEFI_NO_TRACK_ALLOC` | Ne tartsa nyilván a foglalt méreteket (gyorsabb, de bufferen kívülről olvas realloc-nál)  |

Lényeges eltérések a POSIX libc-től
-----------------------------------

Ez a függvénykönyvtár korántsem annyira teljes, mint a glibc vagy a musl például. Csakis a legszükségesebb libc funkciókat
biztosítja, mivel egyszerűségre törekszik. A legjobb talán UEFI API burkolóként tekinteni rá, és nem pedig egy teljes POSIX
kompatíbilis libc-ként.

UEFI alatt minden sztring 16 bit széles karakterekkel van tárolva. A függvénykönyvtár biztosít egy `wchar_t` típust ehhez,
és egy `UEFI_NO_UTF8` define opciót a transzparens `char` és `wchar_t` közötti konvertáláshoz. Ha megadod az `UEFI_NO_UTF8`-at,
akkor például a main() függvényed NEM `main(int argc, char **argv)` lesz, hanem `main(int argc, wchar_t **argv)`. Az összes
többi sztring függvény (mint például az strlen() is) ezt a széles karaktertípust fogja használni. Emiatt az összes sztring
konstansot `L""`-el, a karakterkonstansokat pedig `L''`-el kell definiálni. Hogy mindkét konfigurációt kezelni lehessen,
adott egy `char_t` típus, ami vagy `char` vagy `wchar_t`, és a `CL()` makró, ami `L` előtagot ad a konstansokhoz, amikor kell.
Azok a funkciók, amik a karaktereket int típusként kezelik (pl. `getchar`, `putchar`), nem unsigned char-ra csonkítanak, hanem
wchar_t-re.

Sajnos az UEFI-ben nincs olyan, hogy buffer átméretezés. Az AllocatePool nem fogad bemenetet, és nincs mód egymár allokált
buffer méretének lekérésére sem. Szóval két rossz közül válaszhatunk a `realloc`-nál:
1. magunk tartjuk nyilván a méreteket, ami bonyolultabb kódot és lassabb futást jelent.
2. megbékélünk vele, hogy az adatok másolása az új bufferbe elkerülhetetlenül a régi bufferen túli olvasást eredményez.
Ez utőbbi opció választható az `UEFI_NO_TRACK_ALLOC` define megadásával.

A fájl típusok a dirent-ben nagyon limitáltak, csak könyvtár és fájl megengedett (DT_DIR, DT_REG), de a stat pluszban az
S_IFDIR és S_IFREG típusokhoz, S_IFIFO (konzol folyamok: stdin, stdout, stderr), S_IFBLK (Block IO esetén) és S_IFCHR
(Serial IO esetén) típusokat is visszaadhat.

Továbbá a `getenv` és `setenv` sem POSIX sztandard, mivel az UEFI környezeti változók bináris bitkolbászok.

Nagyjából ennyi, minden más a megszokott.

Az elérhető POSIX funkciók listája
----------------------------------

### dirent.h

| Funkció       | Leírás                                                                     |
|---------------|----------------------------------------------------------------------------|
| opendir       | megszokott, de széles karakterű sztringet is elfogadhat                    |
| readdir       | megszokott                                                                 |
| rewinddir     | megszokott                                                                 |
| closedir      | megszokott                                                                 |

Mivel az UEFI számára ismeretlen az eszközfájl és a szimbólikus link, a dirent mezők eléggé limitáltak, és csak DT_DIR
valamint DT_REF típusok támogatottak.

### stdlib.h

| Funkció       | Leírás                                                                     |
|---------------|----------------------------------------------------------------------------|
| atoi          | megszokott, de széles karakterű sztringet és "0x" prefixet is elfogadhat   |
| atol          | megszokott, de széles karakterű sztringet és "0x" prefixet is elfogadhat   |
| strtol        | megszokott, de széles karakterű sztringet is elfogadhat                    |
| malloc        | megszokott                                                                 |
| calloc        | megszokott                                                                 |
| realloc       | megszokott                                                                 |
| free          | megszokott                                                                 |
| abort         | megszokott                                                                 |
| exit          | megszokott                                                                 |
| exit_bs       | az egész UEFI szörnyűség elhagyása (exit Boot Services)                    |
| mbtowc        | megszokott (UTF-8 karakter wchar_t-á)                                      |
| wctomb        | megszokott (wchar_t-ról UTF-8 karakterré)                                  |
| mbstowcs      | megszokott (UTF-8 sztringről wchar_t sztringé)                             |
| wcstombs      | megszokott (wchar_t sztringről UTF-8 sztringé)                             |
| srand         | megszokott                                                                 |
| rand          | megszokott, de EFI_RNG_PROTOCOL-t használ, ha lehetséges                   |
| getenv        | eléggé UEFI specifikus                                                     |
| setenv        | eléggé UEFI specifikus                                                     |

```c
int exit_bs();
```
Exit Boot Services, az UEFI sárkánylakta vidékének elhagyása. Siker esetén 0-át ad vissza. A sikeres hívást követően nem fogsz
tudni visszatérni a main()-ből, direktben kell átadnod a vezérlést.

```c
uint8_t *getenv(char_t *name, uintn_t *len);
```
A `name` környezeti változó értékének lekérdezése. Siker esetén a `len` be lesz állítva, és egy frissen allokált buffert
ad vissza. A hívó felelőssége a visszaadott buffer felszabadítása, ha már nem kell. Hiba esetén NULL-t ad vissza.

```c
int setenv(char_t *name, uintn_t len, uint8_t *data);
```
A `name` környezeti változó beállítása `len` hosszú `data` értékkel. Siker esetén 1-el tér vissza, hibánál 0-val.

### stdio.h

| Funkció       | Leírás                                                                     |
|---------------|----------------------------------------------------------------------------|
| remove        | megszokott, de széles karakterű sztringet is elfogadhat                    |
| fopen         | megszokott, de széles karakterű sztringet is elfogadhat, mode esetén is    |
| fclose        | megszokott                                                                 |
| fflush        | megszokott                                                                 |
| fread         | megszokott, csak igazi fájlok és blk io (nem stdin)                        |
| fwrite        | megszokott, csak igazi fájlok és blk io (nem lehet stdout se stderr)       |
| fseek         | megszokott, csak igazi fájlok és blk io (nem stdin, stdout, stderr)        |
| ftell         | megszokott, csak igazi fájlok és blk io (nem stdin, stdout, stderr)        |
| feof          | megszokott, csak igazi fájlok és blk io (nem stdin, stdout, stderr)        |
| fprintf       | megszokott, de széles sztring is lehet, BUFSIZ, fájl, ser, stdout, stderr  |
| printf        | megszokott, de széles sztring is lehet, max BUFSIZ, csak stdout            |
| sprintf       | megszokott, de széles sztring is lehet, max BUFSIZ                         |
| vfprintf      | megszokott, de széles sztring is lehet, BUFSIZ, fájl, ser, stdout, stderr  |
| vprintf       | megszokott, de széles sztring is lehet, max BUFSIZ, csak stdout            |
| vsprintf      | megszokott, de széles sztring is lehet, max BUFSIZ                         |
| snprintf      | megszokott, de széles sztring is lehet                                     |
| vsnprintf     | megszokott, de széles sztring is lehet                                     |
| getchar       | megszokott, blokkol, csak stdin (nincs átirányítás), UNICODE-ot ad vissza  |
| getchar_ifany | nem blokkoló, 0-át ad vissza ha nem volt billentyű, egyébként UNICODE-ot   |
| putchar       | megszokott, csak stdout (nincs átriányítás)                                |

A sztring formázás limitált: csak pozitív számokat fogad el prefixnek, `%d` és `%i`, `%x`, `%X`, `%c`, `%s`, `%q` és `%p` (nincs `%e`,
`%f`, `%g`, nincs csillag és dollárjel). Ha a `UEFI_NO_UTF8` definiálva van, akkor a formázás wchar_t-t használ, ezért ilyenkor
támogatott a nem szabványos `%S` (UTF-8 sztring kiírás) és `%Q` (eszképelt UTF-8 sztring kiírás) is. Ezek a funkciók nem
foglalnak le memóriát, cserébe a teljes hossz `BUFSIZ` lehet (8k ha nem definiálták másképp), kivéve azokat a variánsokat,
amik elfogadnak maxlen hossz paramétert. Kényelmi okokból támogatott a `%D` aminek `efi_physical_address_t` paramétert kell
adni, és a memóriát dumpolja, 16 bájtos sorokban. A szám módosítókkal lehet több sort is dumpoltatni, például `%5D` 5 sort
fog dumpolni (80 bájt).

Fájl megnyitási módok: `"r"` olvasás, `"w"` írás, `"a"` hozzáfűzés. UEFI sajátosságok miatt, `"wd"` könyvtárat hoz létre.

Speciális "eszköz fájlok", amiket meg lehet nyitni:

| Név                 | Leírás                                                               |
|---------------------|----------------------------------------------------------------------|
| `/dev/stdin`        | ST->ConIn                                                            |
| `/dev/stdout`       | ST->ConOut, fprintf                                                  |
| `/dev/stderr`       | ST->StdErr, fprintf                                                  |
| `/dev/serial(baud)` | Serial IO protokoll, fread, fwrite, fprintf                          |
| `/dev/disk(n)`      | Block IO protokoll, fseek, ftell, fread, fwrite, feof                |

Block IO esetén az fseek és a buffer méret fread és fwritenál az eszköz blokméretére lesz igazítva. Például fseek(513)
az 512. bájtra pozicionál szabvány blokkméretnél, de 0-ra nagy 4096-os blokkoknál. A blokkméret detektálásához az fstat-ot
lehet használni.
```c
if(!fstat(f, &st))
    block_size = st.st_size / st.st_blocks;
```
A partíciós GPT tábla értelmezéséhez típusdefiníciók állnak a rendelkezésre, mint `efi_partition_table_header_t` és
`efi_partition_entry_t`, amikkel a beolvasott adatokra lehet mutatni.

### string.h

| Funkció       | Leírás                                                                     |
|---------------|----------------------------------------------------------------------------|
| memcpy        | megszokott, mindenképp bájt                                                |
| memmove       | megszokott, mindenképp bájt                                                |
| memset        | megszokott, mindenképp bájt                                                |
| memcmp        | megszokott, mindenképp bájt                                                |
| memchr        | megszokott, mindenképp bájt                                                |
| memrchr       | megszokott, mindenképp bájt                                                |
| memmem        | megszokott, mindenképp bájt                                                |
| memrmem       | megszokott, mindenképp bájt                                                |
| strcpy        | széles karakterű sztringet is elfogadhat                                   |
| strncpy       | széles karakterű sztringet is elfogadhat                                   |
| strcat        | széles karakterű sztringet is elfogadhat                                   |
| strncat       | széles karakterű sztringet is elfogadhat                                   |
| strcmp        | széles karakterű sztringet is elfogadhat                                   |
| strncmp       | széles karakterű sztringet is elfogadhat                                   |
| strdup        | széles karakterű sztringet is elfogadhat                                   |
| strchr        | széles karakterű sztringet is elfogadhat                                   |
| strrchr       | széles karakterű sztringet is elfogadhat                                   |
| strstr        | széles karakterű sztringet is elfogadhat                                   |
| strtok        | széles karakterű sztringet is elfogadhat                                   |
| strtok_r      | széles karakterű sztringet is elfogadhat                                   |
| strlen        | széles karakterű sztringet is elfogadhat                                   |

### sys/stat.h

| Funkció       | Leírás                                                                     |
|---------------|----------------------------------------------------------------------------|
| stat          | megszokott, de széles karakterű sztringet is elfogadhat                    |
| fstat         | UEFI alatt nincs fd, ezért FILE\*-ot használ                               |
| mkdir         | megszokott, de széles karakterű sztringet is elfogadhat, mode nem használt |

Mivel az UEFI számára ismeretlen az eszköz major és minor valamint az inode szám, a struct stat mezői limitáltak.
Az `fstat` implementációja az stdio.c-ben található, mivel el kell érnie bizonyos ott definiált statikus változókat.

### time.h

| Funkció       | Leírás                                                                     |
|---------------|----------------------------------------------------------------------------|
| localtime     | paraméterek nem használtak, mindig a pontos időt adja vissza struct tm-ben |
| mktime        | megszokott                                                                 |
| time          | megszokott                                                                 |

### unistd.h

| Funkció       | Leírás                                                                     |
|---------------|----------------------------------------------------------------------------|
| usleep        | megszokott (BS->Stall hívást használ)                                      |
| sleep         | megszokott                                                                 |
| unlink        | megszokott, de széles karakterű sztringet is elfogadhat                    |
| rmdir         | megszokott, de széles karakterű sztringet is elfogadhat                    |

UEFI szolgáltatások elérése
---------------------------

Elég valószínű, hogy direktben UEFI szolgáltatást kell majd hívni. Ehhez a POSIX-UEFI néhány globális változót biztosít
az `uefi.h`-ban definiálva:

| Globális változó | Leírás                                                   |
|------------------|----------------------------------------------------------|
| `*BS`, `gBS`     | *efi_boot_services_t*, mutató a Boot Time Services-ra    |
| `*RT`, `gRT`     | *efi_runtime_t*, mutató a Runtime Services-ra            |
| `*ST`, `gST`     | *efi_system_table_t*, mutató az UEFI System Table-re     |
| `IM`             | a betöltött fájlod *efi_handle_t*-je (loaded image)      |

Az EFI struktúrák, enumok, típusdefiníciók, defineok mind ANSI C szabványos POSIX stílusúra lettek konvertálva, például
BOOLEAN ->  boolean_t, UINTN -> uintn_t, EFI_MEMORY_DESCRIPTOR -> efi_memory_descriptor_t, és persze
EFI_BOOT_SERVICES -> efi_boot_services_t.

UEFI funkciók hívása pont olyan egyszerű, mint EDK II esetén, csak meg kell hívni, nincs szükség "uefi_call_wrapper"-re:
```c
    ST->ConOut->OutputString(ST->ConOut, L"Hello World!\r\n");
```
(Megjegyzés: a printf-el ellentétben az OutputString-nél mindig kell `L""` és ki kell írni `L"\r"`-t a `L"\n"` előtt. Ezek
azok az apróságok, amikkel a POSIX-UEFI kényelmesebbé teszi az életedet.)

Van továbbá két, nem POSIX szabványos függvény a könyvtárban. Az egyik az `exit_bs()` az UEFI elhagyására, a másik a nem
blokkoló `getchar_ifany()`.

A gnu-efi-vel ellentétben a POSIX-UEFI nem szennyezi a névteret nemhasznált GUID változókkal. Csak define-okat biztosít,
ezért mindig neked kell létrehozni a GUID példányt, ha és amikor szükséged van rá.

Példa:
```c
efi_guid_t gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
efi_gop_t *gop = NULL;

status = BS->LocateProtocol(&gopGuid, NULL, (void**)&gop);
```

Szintén a gnu-efi-vel ellentétben a POSIX-UEFI nem biztosítja a szabványos UEFI fejlécfájlokat. Úgy veszi, hogy azokat
vagy az EDk II-ből vagy a gnu.efi-ből átmásoltad az /usr/include/efi alá, és biztosítja, hogy be legyenek húzva névtér
ütközés nélkül. Maga a POSIX-UEFI csak egy nagyon minimális mennyiségű típusdefiniciót biztosít (és azokat is POSIX-osított
névvel).
```c
#include <efi.h>
#include <uefi.h> /* ez működik! Mind a POSIX-UEFI és az EDK II / gnu-efi typedef-ek elérhetők */
```
Ennek az az előnye, hogy használhatod a POSIX-UEFI egyszerű könyvtárát és fordító környezetét, ugyanakkor hozzáférhetsz
a legfrissebb protokoll és interfész definíciókoz egyaránt.

__FONTOS FIGYELMEZTETÉS__

Bizonyos esetekben a GNU-EFI hedörök és a Clang kombinációja hibásan 32 bites `uint64_t` típust eredményezhet. Ha ez előjön,
akkor a
```c
#undef __STDC_VERSION__
#include <efi.h>
#include <uefi.h>
```
kerülőmegoldás lehet a problémára, mivel így nem húzza be az stdint.h-t, helyette az efibind.h fogja definiálni az uint64_t
típust mint `unsigned long long`.

Licensz
-------

POSIX_UEFI az MIT licensz alatt kerül terjesztésre.

Hozzájárulások
--------------

Szeretnék köszönetet mondani @vladimir132 -nek az alapos tesztelésért és a pontos és részletes visszajelzésekért.

Ez minden,

bzt
