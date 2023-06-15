/*
 * uefi.h
 * https://gitlab.com/bztsrc/posix-uefi
 *
 * Copyright (C) 2021 bzt (bztsrc@gitlab)
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * This file is part of the POSIX-UEFI package.
 * @brief Main (and only) header file
 *
 */

#ifndef _UEFI_H_
#define _UEFI_H_

/*** configuration ***/
/* #define UEFI_NO_UTF8 */                  /* use wchar_t in your application */
/* #define UEFI_NO_TRACK_ALLOC */           /* do not track allocated buffers' size */
/*** configuration ends ***/

#ifdef  __cplusplus
extern "C" {
#endif

/* get these from the compiler or the efi headers, only define if we have neither */
#if !defined(_STDINT_H) && !defined(_GCC_STDINT_H) && !defined(_EFI_INCLUDE_)
#define _STDINT_H
typedef char                int8_t;
typedef unsigned char       uint8_t;
typedef short               int16_t;
typedef unsigned short      uint16_t;
typedef int                 int32_t;
typedef unsigned int        uint32_t;
#ifndef __clang__
typedef long int            int64_t;
typedef unsigned long int   uint64_t;
typedef unsigned long int   uintptr_t;
#else
typedef long long           int64_t;
typedef unsigned long long  uint64_t;
typedef unsigned long long  uintptr_t;
#endif
#endif
extern char c_assert1[sizeof(uint32_t) == 4  ? 1 : -1];
extern char c_assert2[sizeof(uint64_t) == 8  ? 1 : -1];
extern char c_assert3[sizeof(uintptr_t) == 8 ? 1 : -1];

#ifndef NULL
#define NULL ((void*)0)
#endif
/*** common defines and typedefs ***/
typedef int64_t  intn_t;
typedef uint8_t  boolean_t;
typedef uint16_t wchar_t;
typedef uint64_t uintn_t;
typedef uint64_t size_t;
typedef uint64_t time_t;
typedef uint64_t mode_t;
typedef uint64_t off_t;
typedef uint64_t blkcnt_t;
typedef uint64_t efi_status_t;
typedef uint64_t efi_tpl_t;
typedef uint64_t efi_lba_t;
typedef uint64_t efi_physical_address_t;
typedef uint64_t efi_virtual_address_t;
typedef void     *efi_handle_t;
typedef void     *efi_event_t;
#ifndef UEFI_NO_UTF8
typedef char    char_t;
#define CL(a)   a
extern char *__argvutf8;
#else
typedef wchar_t char_t;
#define CL(a)   L ## a
#endif

typedef struct {
    uint32_t    Data1;
    uint16_t    Data2;
    uint16_t    Data3;
    uint8_t     Data4[8];
} efi_guid_t;

typedef struct {
    uint8_t     Type;
    uint8_t     SubType;
    uint8_t     Length[2];
} efi_device_path_t;

typedef struct {
    uint32_t                Type;
    uint32_t                Pad;
    efi_physical_address_t  PhysicalStart;
    efi_virtual_address_t   VirtualStart;
    uint64_t                NumberOfPages;
    uint64_t                Attribute;
} efi_memory_descriptor_t;

typedef struct {
    uint64_t    Signature;
    uint32_t    Revision;
    uint32_t    HeaderSize;
    uint32_t    CRC32;
    uint32_t    Reserved;
} efi_table_header_t;

/*** definitions only needed when efi.h (either from EDK II or gnu-efi) is NOT included ***/

#ifndef EFI_SPECIFICATION_MAJOR_REVISION

/* efibind.h */
#ifndef __WCHAR_TYPE__
# define __WCHAR_TYPE__ short
#endif
#define EFIERR(a)           (0x8000000000000000 | (unsigned int)(a))
#define EFI_ERROR_MASK      0x8000000000000000
#define EFIERR_OEM(a)       (0xc000000000000000 | (unsigned int)(a))

#define BAD_POINTER         0xFBFBFBFBFBFBFBFB
#define MAX_ADDRESS         0xFFFFFFFFFFFFFFFF

#define EFI_SIGNATURE_16(A,B)             ((A) | (B<<8))
#define EFI_SIGNATURE_32(A,B,C,D)         (EFI_SIGNATURE_16(A,B)     | (EFI_SIGNATURE_16(C,D)     << 16))
#define EFI_SIGNATURE_64(A,B,C,D,E,F,G,H) (EFI_SIGNATURE_32(A,B,C,D) | ((uint64_t)(EFI_SIGNATURE_32(E,F,G,H)) << 32))

#ifndef EFIAPI
# ifdef _MSC_EXTENSIONS
#  define EFIAPI __cdecl
# elif defined(HAVE_USE_MS_ABI)
#  define EFIAPI __attribute__((ms_abi))
# else
#  define EFIAPI
# endif
#endif

/* efistdarg.h */
typedef __builtin_va_list va_list;
#define va_start(v,l)   __builtin_va_start(v,l)
#define va_end(v)       __builtin_va_end(v)
#define va_arg(v,l)     __builtin_va_arg(v,l)
#define va_copy(d,s)    __builtin_va_copy(d,s)

/* efierr.h */
#define EFIWARN(a)                            (a)
#define EFI_ERROR(a)           (((intn_t) a) < 0)
#define EFI_SUCCESS                            0
#define EFI_LOAD_ERROR                  EFIERR(1)
#define EFI_INVALID_PARAMETER           EFIERR(2)
#define EFI_UNSUPPORTED                 EFIERR(3)
#define EFI_BAD_BUFFER_SIZE             EFIERR(4)
#define EFI_BUFFER_TOO_SMALL            EFIERR(5)
#define EFI_NOT_READY                   EFIERR(6)
#define EFI_DEVICE_ERROR                EFIERR(7)
#define EFI_WRITE_PROTECTED             EFIERR(8)
#define EFI_OUT_OF_RESOURCES            EFIERR(9)
#define EFI_VOLUME_CORRUPTED            EFIERR(10)
#define EFI_VOLUME_FULL                 EFIERR(11)
#define EFI_NO_MEDIA                    EFIERR(12)
#define EFI_MEDIA_CHANGED               EFIERR(13)
#define EFI_NOT_FOUND                   EFIERR(14)
#define EFI_ACCESS_DENIED               EFIERR(15)
#define EFI_NO_RESPONSE                 EFIERR(16)
#define EFI_NO_MAPPING                  EFIERR(17)
#define EFI_TIMEOUT                     EFIERR(18)
#define EFI_NOT_STARTED                 EFIERR(19)
#define EFI_ALREADY_STARTED             EFIERR(20)
#define EFI_ABORTED                     EFIERR(21)
#define EFI_ICMP_ERROR                  EFIERR(22)
#define EFI_TFTP_ERROR                  EFIERR(23)
#define EFI_PROTOCOL_ERROR              EFIERR(24)
#define EFI_INCOMPATIBLE_VERSION        EFIERR(25)
#define EFI_SECURITY_VIOLATION          EFIERR(26)
#define EFI_CRC_ERROR                   EFIERR(27)
#define EFI_END_OF_MEDIA                EFIERR(28)
#define EFI_END_OF_FILE                 EFIERR(31)
#define EFI_INVALID_LANGUAGE            EFIERR(32)
#define EFI_COMPROMISED_DATA            EFIERR(33)

#define EFI_WARN_UNKOWN_GLYPH           EFIWARN(1)
#define EFI_WARN_UNKNOWN_GLYPH          EFIWARN(1)
#define EFI_WARN_DELETE_FAILURE         EFIWARN(2)
#define EFI_WARN_WRITE_FAILURE          EFIWARN(3)
#define EFI_WARN_BUFFER_TOO_SMALL       EFIWARN(4)

/* efisetjmp.h */
#ifdef __x86_64__
typedef struct {
    uint64_t    Rbx;
    uint64_t    Rsp;
    uint64_t    Rbp;
    uint64_t    Rdi;
    uint64_t    Rsi;
    uint64_t    R12;
    uint64_t    R13;
    uint64_t    R14;
    uint64_t    R15;
    uint64_t    Rip;
    uint64_t    MxCsr;
    uint8_t     XmmBuffer[160];
} __attribute__((aligned(8))) jmp_buf[1];
#endif
#ifdef __aarch64__
typedef struct {
    uint64_t    X19;
    uint64_t    X20;
    uint64_t    X21;
    uint64_t    X22;
    uint64_t    X23;
    uint64_t    X24;
    uint64_t    X25;
    uint64_t    X26;
    uint64_t    X27;
    uint64_t    X28;
    uint64_t    FP;
    uint64_t    LR;
    uint64_t    IP0;
    uint64_t    reserved;
    uint64_t    D8;
    uint64_t    D9;
    uint64_t    D10;
    uint64_t    D11;
    uint64_t    D12;
    uint64_t    D13;
    uint64_t    D14;
    uint64_t    D15;
} __attribute__((aligned(8))) jmp_buf[1];
#endif
#if defined(__riscv) && __riscv_xlen == 64
typedef struct {
    uint64_t    pc;
    uint64_t    sp;
    uint64_t    regs[12];
    double      fp[12];
} __attribute__((aligned(8))) jmp_buf[1];
#endif
extern uintn_t setjmp(jmp_buf env) __attribute__((returns_twice));
extern void longjmp(jmp_buf env, uintn_t value) __attribute__((noreturn));

/* efidevp.h */
#define EFI_DEVICE_PATH_PROTOCOL_GUID { 0x9576e91, 0x6d3f, 0x11d2, {0x8e, 0x39, 0x0, 0xa0, 0xc9, 0x69, 0x72, 0x3b} }
#define EFI_DP_TYPE_MASK                    0x7F
#define EFI_DP_TYPE_UNPACKED                0x80
#define END_DEVICE_PATH_TYPE                0x7f
#define END_ENTIRE_DEVICE_PATH_SUBTYPE      0xff
#define END_INSTANCE_DEVICE_PATH_SUBTYPE    0x01
#define END_DEVICE_PATH_LENGTH              (sizeof(efi_device_path_t))
#define DP_IS_END_TYPE(a)
#define DP_IS_END_SUBTYPE(a)        ( ((a)->SubType == END_ENTIRE_DEVICE_PATH_SUBTYPE )
#define DevicePathType(a)           ( ((a)->Type) & EFI_DP_TYPE_MASK )
#define DevicePathSubType(a)        ( (a)->SubType )
#define DevicePathNodeLength(a)     ( ((a)->Length[0]) | ((a)->Length[1] << 8) )
#define NextDevicePathNode(a)       ( (efi_device_path_t *) ( ((uint8_t *) (a)) + DevicePathNodeLength(a)))
#define IsDevicePathEndType(a)      ( DevicePathType(a) == END_DEVICE_PATH_TYPE )
#define IsDevicePathEndSubType(a)   ( (a)->SubType == END_ENTIRE_DEVICE_PATH_SUBTYPE )
#define IsDevicePathEnd(a)          ( IsDevicePathEndType(a) && IsDevicePathEndSubType(a) )
#define IsDevicePathUnpacked(a)     ( (a)->Type & EFI_DP_TYPE_UNPACKED )
#define SetDevicePathNodeLength(a,l) {                  \
            (a)->Length[0] = (uint8_t) (l);               \
            (a)->Length[1] = (uint8_t) ((l) >> 8);        \
            }
#define SetDevicePathEndNode(a)  {                      \
            (a)->Type = END_DEVICE_PATH_TYPE;           \
            (a)->SubType = END_ENTIRE_DEVICE_PATH_SUBTYPE;     \
            (a)->Length[0] = sizeof(efi_device_path_t); \
            (a)->Length[1] = 0;                         \
            }

/* efiapi.h */
#define EFI_SPECIFICATION_MAJOR_REVISION 1
#define EFI_SPECIFICATION_MINOR_REVISION 02

#define TPL_APPLICATION      4
#define TPL_CALLBACK         8
#define TPL_NOTIFY          16
#define TPL_HIGH_LEVEL      31
#define EFI_TPL_APPLICATION TPL_APPLICATION
#define EFI_TPL_CALLBACK    TPL_CALLBACK
#define EFI_TPL_NOTIFY      TPL_NOTIFY
#define EFI_TPL_HIGH_LEVEL  TPL_HIGH_LEVEL

#define NextMemoryDescriptor(Ptr,Size)  ((efi_memory_descriptor_t *) (((uint8_t *) Ptr) + Size))

#define EFI_PAGE_SIZE           4096
#define EFI_PAGE_MASK           0xFFF
#define EFI_PAGE_SHIFT          12

#define EFI_SIZE_TO_PAGES(a)    ( ((a) >> EFI_PAGE_SHIFT) + ((a) & EFI_PAGE_MASK ? 1 : 0) )

#define EFI_MEMORY_UC           0x0000000000000001
#define EFI_MEMORY_WC           0x0000000000000002
#define EFI_MEMORY_WT           0x0000000000000004
#define EFI_MEMORY_WB           0x0000000000000008
#define EFI_MEMORY_UCE          0x0000000000000010
#define EFI_MEMORY_WP           0x0000000000001000
#define EFI_MEMORY_RP           0x0000000000002000
#define EFI_MEMORY_XP           0x0000000000004000
#define EFI_MEMORY_RUNTIME      0x8000000000000000
#define EFI_MEMORY_DESCRIPTOR_VERSION  1

#define EVT_TIMER                           0x80000000
#define EVT_RUNTIME                         0x40000000
#define EVT_RUNTIME_CONTEXT                 0x20000000

#define EVT_NOTIFY_WAIT                     0x00000100
#define EVT_NOTIFY_SIGNAL                   0x00000200

#define EVT_SIGNAL_EXIT_BOOT_SERVICES       0x00000201
#define EVT_SIGNAL_VIRTUAL_ADDRESS_CHANGE   0x60000202

#define EVT_EFI_SIGNAL_MASK                 0x000000FF
#define EVT_EFI_SIGNAL_MAX                  4

#define EFI_EVENT_TIMER                         EVT_TIMER
#define EFI_EVENT_RUNTIME                       EVT_RUNTIME
#define EFI_EVENT_RUNTIME_CONTEXT               EVT_RUNTIME_CONTEXT
#define EFI_EVENT_NOTIFY_WAIT                   EVT_NOTIFY_WAIT
#define EFI_EVENT_NOTIFY_SIGNAL                 EVT_NOTIFY_SIGNAL
#define EFI_EVENT_SIGNAL_EXIT_BOOT_SERVICES     EVT_SIGNAL_EXIT_BOOT_SERVICES
#define EFI_EVENT_SIGNAL_VIRTUAL_ADDRESS_CHANGE EVT_SIGNAL_VIRTUAL_ADDRESS_CHANGE
#define EFI_EVENT_EFI_SIGNAL_MASK               EVT_EFI_SIGNAL_MASK
#define EFI_EVENT_EFI_SIGNAL_MAX                EVT_EFI_SIGNAL_MAX

#define EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL  0x00000001
#define EFI_OPEN_PROTOCOL_GET_PROTOCOL        0x00000002
#define EFI_OPEN_PROTOCOL_TEST_PROTOCOL       0x00000004
#define EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER 0x00000008
#define EFI_OPEN_PROTOCOL_BY_DRIVER           0x00000010
#define EFI_OPEN_PROTOCOL_EXCLUSIVE           0x00000020

#define EFI_OPTIONAL_PTR            0x00000001
#define EFI_INTERNAL_FNC            0x00000002
#define EFI_INTERNAL_PTR            0x00000004

#define EFI_GLOBAL_VARIABLE             { 0x8BE4DF61, 0x93CA, 0x11d2, {0xAA, 0x0D, 0x00, 0xE0, 0x98, 0x03, 0x2B, 0x8C} }
#define EFI_VARIABLE_NON_VOLATILE                          0x00000001
#define EFI_VARIABLE_BOOTSERVICE_ACCESS                    0x00000002
#define EFI_VARIABLE_RUNTIME_ACCESS                        0x00000004
#define EFI_VARIABLE_HARDWARE_ERROR_RECORD                 0x00000008
#define EFI_VARIABLE_AUTHENTICATED_WRITE_ACCESS            0x00000010
#define EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS 0x00000020
#define EFI_VARIABLE_APPEND_WRITE                          0x00000040
#define EFI_MAXIMUM_VARIABLE_SIZE           1024

#define CAPSULE_FLAGS_PERSIST_ACROSS_RESET    0x00010000
#define CAPSULE_FLAGS_POPULATE_SYSTEM_TABLE   0x00020000
#define CAPSULE_FLAGS_INITIATE_RESET          0x00040000

#define MPS_TABLE_GUID                  { 0xeb9d2d2f, 0x2d88, 0x11d3, {0x9a, 0x16, 0x0, 0x90, 0x27, 0x3f, 0xc1, 0x4d} }
#define ACPI_TABLE_GUID                 { 0xeb9d2d30, 0x2d88, 0x11d3, {0x9a, 0x16, 0x0, 0x90, 0x27, 0x3f, 0xc1, 0x4d} }
#define ACPI_20_TABLE_GUID              { 0x8868e871, 0xe4f1, 0x11d3, {0xbc, 0x22, 0x0, 0x80, 0xc7, 0x3c, 0x88, 0x81} }
#define SMBIOS_TABLE_GUID               { 0xeb9d2d31, 0x2d88, 0x11d3, {0x9a, 0x16, 0x0, 0x90, 0x27, 0x3f, 0xc1, 0x4d} }
#define SMBIOS3_TABLE_GUID              { 0xf2fd1544, 0x9794, 0x4a2c, {0x99, 0x2e,0xe5, 0xbb, 0xcf, 0x20, 0xe3, 0x94} }
#define SAL_SYSTEM_TABLE_GUID           { 0xeb9d2d32, 0x2d88, 0x11d3, {0x9a, 0x16, 0x0, 0x90, 0x27, 0x3f, 0xc1, 0x4d} }

#define EFI_RUNTIME_SERVICES_SIGNATURE  0x56524553544e5552
#define EFI_RUNTIME_SERVICES_REVISION   (EFI_SPECIFICATION_MAJOR_REVISION<<16) | (EFI_SPECIFICATION_MINOR_REVISION)

#define EFI_BOOT_SERVICES_SIGNATURE     0x56524553544f4f42
#define EFI_BOOT_SERVICES_REVISION      (EFI_SPECIFICATION_MAJOR_REVISION<<16) | (EFI_SPECIFICATION_MINOR_REVISION)

#define EFI_SYSTEM_TABLE_SIGNATURE      0x5453595320494249
#define EFI_SYSTEM_TABLE_REVISION       (EFI_SPECIFICATION_MAJOR_REVISION<<16) | (EFI_SPECIFICATION_MINOR_REVISION)

/* eficon.h */
#define EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL_GUID { 0x387477c2, 0x69c7, 0x11d2, {0x8e, 0x39, 0x0, 0xa0, 0xc9, 0x69, 0x72, 0x3b} }
#define EFI_BLACK   0x00
#define EFI_BLUE    0x01
#define EFI_GREEN   0x02
#define EFI_CYAN            (EFI_BLUE | EFI_GREEN)
#define EFI_RED     0x04
#define EFI_MAGENTA         (EFI_BLUE | EFI_RED)
#define EFI_BROWN           (EFI_GREEN | EFI_RED)
#define EFI_LIGHTGRAY       (EFI_BLUE | EFI_GREEN | EFI_RED)
#define EFI_BRIGHT  0x08
#define EFI_DARKGRAY        (EFI_BRIGHT)
#define EFI_LIGHTBLUE       (EFI_BLUE | EFI_BRIGHT)
#define EFI_LIGHTGREEN      (EFI_GREEN | EFI_BRIGHT)
#define EFI_LIGHTCYAN       (EFI_CYAN | EFI_BRIGHT)
#define EFI_LIGHTRED        (EFI_RED | EFI_BRIGHT)
#define EFI_LIGHTMAGENTA    (EFI_MAGENTA | EFI_BRIGHT)
#define EFI_YELLOW          (EFI_BROWN | EFI_BRIGHT)
#define EFI_WHITE           (EFI_BLUE | EFI_GREEN | EFI_RED | EFI_BRIGHT)
#define EFI_TEXT_ATTR(f,b)  ((f) | ((b) << 4))
#define EFI_BACKGROUND_BLACK        0x00
#define EFI_BACKGROUND_BLUE         0x10
#define EFI_BACKGROUND_GREEN        0x20
#define EFI_BACKGROUND_CYAN         (EFI_BACKGROUND_BLUE | EFI_BACKGROUND_GREEN)
#define EFI_BACKGROUND_RED          0x40
#define EFI_BACKGROUND_MAGENTA      (EFI_BACKGROUND_BLUE | EFI_BACKGROUND_RED)
#define EFI_BACKGROUND_BROWN        (EFI_BACKGROUND_GREEN | EFI_BACKGROUND_RED)
#define EFI_BACKGROUND_LIGHTGRAY    (EFI_BACKGROUND_BLUE | EFI_BACKGROUND_GREEN | EFI_BACKGROUND_RED)
#define BOXDRAW_HORIZONTAL                  0x2500
#define BOXDRAW_VERTICAL                    0x2502
#define BOXDRAW_DOWN_RIGHT                  0x250c
#define BOXDRAW_DOWN_LEFT                   0x2510
#define BOXDRAW_UP_RIGHT                    0x2514
#define BOXDRAW_UP_LEFT                     0x2518
#define BOXDRAW_VERTICAL_RIGHT              0x251c
#define BOXDRAW_VERTICAL_LEFT               0x2524
#define BOXDRAW_DOWN_HORIZONTAL             0x252c
#define BOXDRAW_UP_HORIZONTAL               0x2534
#define BOXDRAW_VERTICAL_HORIZONTAL         0x253c
#define BOXDRAW_DOUBLE_HORIZONTAL           0x2550
#define BOXDRAW_DOUBLE_VERTICAL             0x2551
#define BOXDRAW_DOWN_RIGHT_DOUBLE           0x2552
#define BOXDRAW_DOWN_DOUBLE_RIGHT           0x2553
#define BOXDRAW_DOUBLE_DOWN_RIGHT           0x2554
#define BOXDRAW_DOWN_LEFT_DOUBLE            0x2555
#define BOXDRAW_DOWN_DOUBLE_LEFT            0x2556
#define BOXDRAW_DOUBLE_DOWN_LEFT            0x2557
#define BOXDRAW_UP_RIGHT_DOUBLE             0x2558
#define BOXDRAW_UP_DOUBLE_RIGHT             0x2559
#define BOXDRAW_DOUBLE_UP_RIGHT             0x255a
#define BOXDRAW_UP_LEFT_DOUBLE              0x255b
#define BOXDRAW_UP_DOUBLE_LEFT              0x255c
#define BOXDRAW_DOUBLE_UP_LEFT              0x255d
#define BOXDRAW_VERTICAL_RIGHT_DOUBLE       0x255e
#define BOXDRAW_VERTICAL_DOUBLE_RIGHT       0x255f
#define BOXDRAW_DOUBLE_VERTICAL_RIGHT       0x2560
#define BOXDRAW_VERTICAL_LEFT_DOUBLE        0x2561
#define BOXDRAW_VERTICAL_DOUBLE_LEFT        0x2562
#define BOXDRAW_DOUBLE_VERTICAL_LEFT        0x2563
#define BOXDRAW_DOWN_HORIZONTAL_DOUBLE      0x2564
#define BOXDRAW_DOWN_DOUBLE_HORIZONTAL      0x2565
#define BOXDRAW_DOUBLE_DOWN_HORIZONTAL      0x2566
#define BOXDRAW_UP_HORIZONTAL_DOUBLE        0x2567
#define BOXDRAW_UP_DOUBLE_HORIZONTAL        0x2568
#define BOXDRAW_DOUBLE_UP_HORIZONTAL        0x2569
#define BOXDRAW_VERTICAL_HORIZONTAL_DOUBLE  0x256a
#define BOXDRAW_VERTICAL_DOUBLE_HORIZONTAL  0x256b
#define BOXDRAW_DOUBLE_VERTICAL_HORIZONTAL  0x256c
#define BLOCKELEMENT_FULL_BLOCK             0x2588
#define BLOCKELEMENT_LIGHT_SHADE            0x2591
#define GEOMETRICSHAPE_UP_TRIANGLE          0x25b2
#define GEOMETRICSHAPE_RIGHT_TRIANGLE       0x25ba
#define GEOMETRICSHAPE_DOWN_TRIANGLE        0x25bc
#define GEOMETRICSHAPE_LEFT_TRIANGLE        0x25c4
#define ARROW_UP                            0x2191
#define ARROW_DOWN                          0x2193

#define EFI_SIMPLE_TEXT_INPUT_PROTOCOL_GUID { 0x387477c1, 0x69c7, 0x11d2, {0x8e, 0x39, 0x0, 0xa0, 0xc9, 0x69, 0x72, 0x3b} }
#define CHAR_NULL                       0x0000
#define CHAR_BACKSPACE                  0x0008
#define CHAR_TAB                        0x0009
#define CHAR_LINEFEED                   0x000A
#define CHAR_CARRIAGE_RETURN            0x000D
#define SCAN_NULL                       0x0000
#define SCAN_UP                         0x0001
#define SCAN_DOWN                       0x0002
#define SCAN_RIGHT                      0x0003
#define SCAN_LEFT                       0x0004
#define SCAN_HOME                       0x0005
#define SCAN_END                        0x0006
#define SCAN_INSERT                     0x0007
#define SCAN_DELETE                     0x0008
#define SCAN_PAGE_UP                    0x0009
#define SCAN_PAGE_DOWN                  0x000A
#define SCAN_F1                         0x000B
#define SCAN_F2                         0x000C
#define SCAN_F3                         0x000D
#define SCAN_F4                         0x000E
#define SCAN_F5                         0x000F
#define SCAN_F6                         0x0010
#define SCAN_F7                         0x0011
#define SCAN_F8                         0x0012
#define SCAN_F9                         0x0013
#define SCAN_F10                        0x0014
#define SCAN_F11                        0x0015
#define SCAN_F12                        0x0016
#define SCAN_ESC                        0x0017

/* efigpt.h */
#define PRIMARY_PART_HEADER_LBA         1
#define EFI_PTAB_HEADER_ID  "EFI PART"
#define EFI_PART_USED_BY_EFI            0x0000000000000001
#define EFI_PART_REQUIRED_TO_FUNCTION   0x0000000000000002
#define EFI_PART_USED_BY_OS             0x0000000000000004
#define EFI_PART_REQUIRED_BY_OS         0x0000000000000008
#define EFI_PART_BACKUP_REQUIRED        0x0000000000000010
#define EFI_PART_USER_DATA              0x0000000000000020
#define EFI_PART_CRITICAL_USER_DATA     0x0000000000000040
#define EFI_PART_REDUNDANT_PARTITION    0x0000000000000080
#define EFI_PART_TYPE_UNUSED_GUID           { 0x00000000, 0x0000, 0x0000, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} }
#define EFI_PART_TYPE_EFI_SYSTEM_PART_GUID  { 0xc12a7328, 0xf81f, 0x11d2, {0xba, 0x4b, 0x00, 0xa0, 0xc9, 0x3e, 0xc9, 0x3b} }
#define EFI_PART_TYPE_LEGACY_MBR_GUID       { 0x024dee41, 0x33e7, 0x11d3, {0x9d, 0x69, 0x00, 0x08, 0xc7, 0x81, 0xf3, 0x9f} }

/* Protocol GUIDs */
#ifndef INTERNAL_SHELL_GUID
#define INTERNAL_SHELL_GUID         { 0xd65a6b8c, 0x71e5, 0x4df0, {0xa9, 0x09, 0xf0, 0xd2, 0x99, 0x2b, 0x5a, 0xa9} }
#endif

typedef enum {
    AllocateAnyPages,
    AllocateMaxAddress,
    AllocateAddress,
    MaxAllocateType
} efi_allocate_type_t;

typedef enum {
    EfiReservedMemoryType,
    EfiLoaderCode,
    EfiLoaderData,
    EfiBootServicesCode,
    EfiBootServicesData,
    EfiRuntimeServicesCode,
    EfiRuntimeServicesData,
    EfiConventionalMemory,
    EfiUnusableMemory,
    EfiACPIReclaimMemory,
    EfiACPIMemoryNVS,
    EfiMemoryMappedIO,
    EfiMemoryMappedIOPortSpace,
    EfiPalCode,
    EfiPersistentMemory,
    EfiUnacceptedMemoryType,
    EfiMaxMemoryType
} efi_memory_type_t;

typedef enum {
    TimerCancel,
    TimerPeriodic,
    TimerRelative,
    TimerTypeMax
} efi_timer_delay_t;

typedef enum {
    AllHandles,
    ByRegisterNotify,
    ByProtocol
} efi_locate_search_type_t;

typedef enum {
    EfiResetCold,
    EfiResetWarm,
    EfiResetShutdown
} efi_reset_type_t;

#else

#define efi_allocate_type_t EFI_ALLOCATE_TYPE
#define efi_memory_type_t EFI_MEMORY_TYPE
#define efi_timer_delay_t EFI_TIMER_DELAY
#define efi_locate_search_type_t EFI_LOCATE_SEARCH_TYPE
#define efi_reset_type_t EFI_RESET_TYPE

#endif

/*** standard input, output and error streams via ConIn, ConOut and StdErr ***/
typedef struct {
    uint16_t    ScanCode;
    wchar_t     UnicodeChar;
} efi_input_key_t;

typedef efi_status_t (EFIAPI *efi_input_reset_t)(void *This, boolean_t ExtendedVerification);
typedef efi_status_t (EFIAPI *efi_input_read_key_t)(void *This, efi_input_key_t *Key);

typedef struct {
    efi_input_reset_t           Reset;
    efi_input_read_key_t        ReadKeyStroke;
    efi_event_t                 WaitForKey;
} simple_input_interface_t;

typedef efi_status_t (EFIAPI *efi_text_reset_t)(void *This, boolean_t ExtendedVerification);
typedef efi_status_t (EFIAPI *efi_text_output_string_t)(void *This, wchar_t *String);
typedef efi_status_t (EFIAPI *efi_text_test_string_t)(void *This, wchar_t *String);
typedef efi_status_t (EFIAPI *efi_text_query_mode_t)(void *This, uintn_t ModeNumber, uintn_t *Column, uintn_t *Row);
typedef efi_status_t (EFIAPI *efi_text_set_mode_t)(void *This, uintn_t ModeNumber);
typedef efi_status_t (EFIAPI *efi_text_set_attribute_t)(void *This, uintn_t Attribute);
typedef efi_status_t (EFIAPI *efi_text_clear_screen_t)(void *This);
typedef efi_status_t (EFIAPI *efi_text_set_cursor_t)(void *This, uintn_t Column, uintn_t Row);
typedef efi_status_t (EFIAPI *efi_text_enable_cursor_t)(void *This,  boolean_t Enable);

typedef struct {
    int32_t                     MaxMode;
    int32_t                     Mode;
    int32_t                     Attribute;
    int32_t                     CursorColumn;
    int32_t                     CursorRow;
    boolean_t                   CursorVisible;
} simple_text_output_mode_t;

typedef struct {
    efi_text_reset_t            Reset;
    efi_text_output_string_t    OutputString;
    efi_text_test_string_t      TestString;
    efi_text_query_mode_t       QueryMode;
    efi_text_set_mode_t         SetMode;
    efi_text_set_attribute_t    SetAttribute;
    efi_text_clear_screen_t     ClearScreen;
    efi_text_set_cursor_t       SetCursorPosition;
    efi_text_enable_cursor_t    EnableCursor;
    simple_text_output_mode_t   *Mode;
} simple_text_output_interface_t;

/*** Runtime Services ***/
typedef struct {
    uint16_t    Year;       /* 1998 - 2XXX */
    uint8_t     Month;      /* 1 - 12 */
    uint8_t     Day;        /* 1 - 31 */
    uint8_t     Hour;       /* 0 - 23 */
    uint8_t     Minute;     /* 0 - 59 */
    uint8_t     Second;     /* 0 - 59 */
    uint8_t     Pad1;
    uint32_t    Nanosecond; /* 0 - 999,999,999 */
    int16_t     TimeZone;   /* -1440 to 1440 or 2047 */
    uint8_t     Daylight;
    uint8_t     Pad2;
} efi_time_t;

typedef struct {
    uint32_t Resolution;
    uint32_t Accuracy;
    boolean_t SetsToZero;
} efi_time_capabilities_t;

typedef struct {
    efi_guid_t  CapsuleGuid;
    uint32_t    HeaderSize;
    uint32_t    Flags;
    uint32_t    CapsuleImageSize;
} efi_capsule_header_t;

#ifndef DataBlock
#define DataBlock ContinuationPointer
#endif
typedef struct {
    uint64_t                Length;
    efi_physical_address_t  ContinuationPointer;
} efi_capsule_block_descriptor_t;

typedef efi_status_t (EFIAPI *efi_get_time_t)(efi_time_t *Time, efi_time_capabilities_t *Capabilities);
typedef efi_status_t (EFIAPI *efi_set_time_t)(efi_time_t *Time);
typedef efi_status_t (EFIAPI *efi_get_wakeup_time_t)(boolean_t *Enable, boolean_t *Pending, efi_time_t *Time);
typedef efi_status_t (EFIAPI *efi_set_wakeup_time_t)(boolean_t Enable, efi_time_t *Time);
typedef efi_status_t (EFIAPI *efi_set_virtual_address_map_t)(uintn_t MemoryMapSize, uintn_t DescriptorSize,
    uint32_t DescriptorVersion, efi_memory_descriptor_t *VirtualMap);
typedef efi_status_t (EFIAPI *efi_convert_pointer_t)(uintn_t DebugDisposition, void **Address);
typedef efi_status_t (EFIAPI *efi_get_variable_t)(wchar_t *VariableName, efi_guid_t *VendorGuid, uint32_t *Attributes,
    uintn_t *DataSize, void *Data);
typedef efi_status_t (EFIAPI *efi_get_next_variable_name_t)(uintn_t *VariableNameSize, wchar_t *VariableName,
    efi_guid_t *VendorGuid);
typedef efi_status_t (EFIAPI *efi_set_variable_t)(wchar_t *VariableName, efi_guid_t *VendorGuid, uint32_t Attributes,
    uintn_t DataSize, void *Data);
typedef efi_status_t (EFIAPI *efi_get_next_high_mono_t)(uint64_t *Count);
typedef efi_status_t (EFIAPI *efi_reset_system_t)(efi_reset_type_t ResetType, efi_status_t ResetStatus, uintn_t DataSize,
    wchar_t *ResetData);
typedef efi_status_t (EFIAPI *efi_update_capsule_t)(efi_capsule_header_t **CapsuleHeaderArray, uintn_t CapsuleCount,
    efi_physical_address_t ScatterGatherList);
typedef efi_status_t (EFIAPI *efi_query_capsule_capabilities_t)(efi_capsule_header_t **CapsuleHeaderArray, uintn_t CapsuleCount,
    uint64_t *MaximumCapsuleSize, efi_reset_type_t *ResetType);
typedef efi_status_t (EFIAPI *efi_query_variable_info_t)(uint32_t Attributes, uint64_t *MaximumVariableStorageSize,
    uint64_t *RemainingVariableStorageSize, uint64_t *MaximumVariableSize);

typedef struct {
    efi_table_header_t              Hdr;

    efi_get_time_t                  GetTime;
    efi_set_time_t                  SetTime;
    efi_get_wakeup_time_t           GetWakeupTime;
    efi_set_wakeup_time_t           SetWakeupTime;

    efi_set_virtual_address_map_t   SetVirtualAddressMap;
    efi_convert_pointer_t           ConvertPointer;

    efi_get_variable_t              GetVariable;
    efi_get_next_variable_name_t    GetNextVariableName;
    efi_set_variable_t              SetVariable;

    efi_get_next_high_mono_t        GetNextHighMonotonicCount;
    efi_reset_system_t              ResetSystem;

    efi_update_capsule_t            UpdateCapsule;
    efi_query_capsule_capabilities_t QueryCapsuleCapabilities;
    efi_query_variable_info_t       QueryVariableInfo;
} efi_runtime_services_t;
extern efi_runtime_services_t *RT;
#define gRT RT

/** Boot Services ***/
typedef struct {
    efi_handle_t    AgentHandle;
    efi_handle_t    ControllerHandle;
    uint32_t        Attributes;
    uint32_t        OpenCount;
} efi_open_protocol_information_entry_t;

typedef efi_tpl_t (EFIAPI *efi_raise_tpl_t)(efi_tpl_t NewTpl);
typedef efi_tpl_t (EFIAPI *efi_restore_tpl_t)(efi_tpl_t OldTpl);
typedef efi_status_t (EFIAPI *efi_allocate_pages_t)(efi_allocate_type_t Type, efi_memory_type_t MemoryType,
    uintn_t NoPages, efi_physical_address_t *Memory);
typedef efi_status_t (EFIAPI *efi_free_pages_t)(efi_physical_address_t Memory, uintn_t NoPages);
typedef efi_status_t (EFIAPI *efi_get_memory_map_t)(uintn_t *MemoryMapSize, efi_memory_descriptor_t *MemoryMap,
    uintn_t *MapKey, uintn_t *DescriptorSize, uint32_t *DescriptorVersion);
typedef efi_status_t (EFIAPI *efi_allocate_pool_t)(efi_memory_type_t PoolType, uintn_t Size, void **Buffer);
typedef efi_status_t (EFIAPI *efi_free_pool_t)(void *Buffer);
typedef void (EFIAPI *efi_event_notify_t)(efi_event_t Event, void *Context);
typedef efi_status_t (EFIAPI *efi_create_event_t)(uint32_t Type, efi_tpl_t NotifyTpl, efi_event_notify_t NotifyFunction,
    void *NextContext, efi_event_t *Event);
typedef efi_status_t (EFIAPI *efi_set_timer_t)(efi_event_t Event, efi_timer_delay_t Type, uint64_t TriggerTime);
typedef efi_status_t (EFIAPI *efi_wait_for_event_t)(uintn_t NumberOfEvents, efi_event_t *Event, uintn_t *Index);
typedef efi_status_t (EFIAPI *efi_signal_event_t)(efi_event_t Event);
typedef efi_status_t (EFIAPI *efi_close_event_t)(efi_event_t Event);
typedef efi_status_t (EFIAPI *efi_check_event_t)(efi_event_t Event);
typedef efi_status_t (EFIAPI *efi_handle_protocol_t)(efi_handle_t Handle, efi_guid_t *Protocol, void **Interface);
typedef efi_status_t (EFIAPI *efi_register_protocol_notify_t)(efi_guid_t *Protocol, efi_event_t Event, void **Registration);
typedef efi_status_t (EFIAPI *efi_locate_handle_t)(efi_locate_search_type_t SearchType, efi_guid_t *Protocol,
    void *SearchKey, uintn_t *BufferSize, efi_handle_t *Buffer);
typedef efi_status_t (EFIAPI *efi_locate_device_path_t)(efi_guid_t *Protocol, efi_device_path_t **DevicePath,
    efi_handle_t *Device);
typedef efi_status_t (EFIAPI *efi_install_configuration_table_t)(efi_guid_t *Guid, void *Table);
typedef efi_status_t (EFIAPI *efi_image_load_t)(boolean_t BootPolicy, efi_handle_t ParentImageHandle, efi_device_path_t *FilePath,
    void *SourceBuffer, uintn_t SourceSize, efi_handle_t *ImageHandle);
typedef efi_status_t (EFIAPI *efi_image_start_t)(efi_handle_t ImageHandle, uintn_t *ExitDataSize, wchar_t **ExitData);
typedef efi_status_t (EFIAPI *efi_exit_t)(efi_handle_t ImageHandle, efi_status_t ExitStatus, uintn_t ExitDataSize,
    wchar_t *ExitData);
typedef efi_status_t (EFIAPI *efi_exit_boot_services_t)(efi_handle_t ImageHandle, uintn_t MapKey);
typedef efi_status_t (EFIAPI *efi_get_next_monotonic_t)(uint64_t *Count);
typedef efi_status_t (EFIAPI *efi_stall_t)(uintn_t Microseconds);
typedef efi_status_t (EFIAPI *efi_set_watchdog_timer_t)(uintn_t Timeout, uint64_t WatchdogCode, uintn_t DataSize,
    wchar_t *WatchdogData);
typedef efi_status_t (EFIAPI *efi_connect_controller_t)(efi_handle_t ControllerHandle, efi_handle_t *DriverImageHandle,
    efi_device_path_t *RemainingDevicePath, boolean_t Recursive);
typedef efi_status_t (EFIAPI *efi_disconnect_controller_t)(efi_handle_t ControllerHandle, efi_handle_t DriverImageHandle,
    efi_handle_t ChildHandle);
typedef efi_status_t (EFIAPI *efi_open_protocol_t)(efi_handle_t Handle, efi_guid_t *Protocol, void **Interface,
    efi_handle_t AgentHandle, efi_handle_t ControllerHandle, uint32_t Attributes);
typedef efi_status_t (EFIAPI *efi_close_protocol_t)(efi_handle_t Handle, efi_guid_t *Protocol, efi_handle_t AgentHandle,
    efi_handle_t ControllerHandle);
typedef efi_status_t (EFIAPI *efi_open_protocol_information_t)(efi_handle_t Handle, efi_guid_t *Protocol,
    efi_open_protocol_information_entry_t**EntryBuffer, uintn_t *EntryCount);
typedef efi_status_t (EFIAPI *efi_protocols_per_handle_t)(efi_handle_t Handle, efi_guid_t ***ProtocolBuffer,
    uintn_t *ProtocolBufferCount);
typedef efi_status_t (EFIAPI *efi_locate_handle_buffer_t)(efi_locate_search_type_t SearchType, efi_guid_t *Protocol,
    void *SearchKey, uintn_t *NoHandles, efi_handle_t **Handles);
typedef efi_status_t (EFIAPI *efi_locate_protocol_t)(efi_guid_t *Protocol, void *Registration, void **Interface);
typedef efi_status_t (EFIAPI *efi_calculate_crc32_t)(void *Data, uintn_t DataSize, uint32_t *Crc32);

typedef struct {
    efi_table_header_t          Hdr;

    efi_raise_tpl_t             RaiseTPL;
    efi_restore_tpl_t           RestoreTPL;

    efi_allocate_pages_t        AllocatePages;
    efi_free_pages_t            FreePages;
    efi_get_memory_map_t        GetMemoryMap;
    efi_allocate_pool_t         AllocatePool;
    efi_free_pool_t             FreePool;

    efi_create_event_t          CreateEvent;
    efi_set_timer_t             SetTimer;
    efi_wait_for_event_t        WaitForEvent;
    efi_signal_event_t          SignalEvent;
    efi_close_event_t           CloseEvent;
    efi_check_event_t           CheckEvent;

    void*                       InstallProtocolInterface;       /* not defined yet */
    void*                       ReinstallProtocolInterface;
    void*                       UninstallProtocolInterface;
    efi_handle_protocol_t       HandleProtocol;
    efi_handle_protocol_t       PCHandleProtocol;
    efi_register_protocol_notify_t RegisterProtocolNotify;
    efi_locate_handle_t         LocateHandle;
    efi_locate_device_path_t    LocateDevicePath;
    efi_install_configuration_table_t InstallConfigurationTable;

    efi_image_load_t            LoadImage;
    efi_image_start_t           StartImage;
    efi_exit_t                  Exit;
    void*                       UnloadImage;                    /* not defined in gnu-efi either */
    efi_exit_boot_services_t    ExitBootServices;

    efi_get_next_monotonic_t    GetNextHighMonotonicCount;
    efi_stall_t                 Stall;
    efi_set_watchdog_timer_t    SetWatchdogTimer;

    efi_connect_controller_t    ConnectController;
    efi_disconnect_controller_t DisconnectController;

    efi_open_protocol_t         OpenProtocol;
    efi_close_protocol_t        CloseProtocol;
    efi_open_protocol_information_t OpenProtocolInformation;

    efi_protocols_per_handle_t  ProtocolsPerHandle;
    efi_locate_handle_buffer_t  LocateHandleBuffer;
    efi_locate_protocol_t       LocateProtocol;
    void*                       InstallMultipleProtocolInterfaces;
    void*                       UninstallMultipleProtocolInterfaces;

    efi_calculate_crc32_t       CalculateCrc32;
} efi_boot_services_t;
extern efi_boot_services_t *BS;
#define gBS BS

/*** Loaded Image Protocol ***/
#ifndef EFI_LOADED_IMAGE_PROTOCOL_GUID
#define EFI_LOADED_IMAGE_PROTOCOL_GUID { 0x5B1B31A1, 0x9562, 0x11d2, {0x8E, 0x3F, 0x00, 0xA0, 0xC9, 0x69, 0x72, 0x3B} }
#define LOADED_IMAGE_PROTOCOL EFI_LOADED_IMAGE_PROTOCOL_GUID

#define EFI_LOADED_IMAGE_PROTOCOL_REVISION  0x1000
#define EFI_IMAGE_INFORMATION_REVISION  EFI_LOADED_IMAGE_PROTOCOL_REVISION
#endif

typedef struct {
    uint32_t                Revision;
    efi_handle_t            ParentHandle;
    void                    *SystemTable;
    efi_handle_t            DeviceHandle;
    efi_device_path_t       *FilePath;
    void                    *Reserved;
    uint32_t                LoadOptionsSize;
    void                    *LoadOptions;
    void                    *ImageBase;
    uint64_t                ImageSize;
    efi_memory_type_t       ImageCodeType;
    efi_memory_type_t       ImageDataType;
} efi_loaded_image_protocol_t;
extern efi_loaded_image_protocol_t *LIP;
extern efi_handle_t IM;

/*** System Table ***/
typedef struct {
    efi_guid_t  VendorGuid;
    void        *VendorTable;
} efi_configuration_table_t;

typedef struct {
    efi_table_header_t              Hdr;

    wchar_t                         *FirmwareVendor;
    uint32_t                        FirmwareRevision;

    efi_handle_t                    ConsoleInHandle;
    simple_input_interface_t        *ConIn;

    efi_handle_t                    ConsoleOutHandle;
    simple_text_output_interface_t  *ConOut;

    efi_handle_t                    ConsoleErrorHandle;
    simple_text_output_interface_t  *StdErr;

    efi_runtime_services_t          *RuntimeServices;
    efi_boot_services_t             *BootServices;

    uintn_t                         NumberOfTableEntries;
    efi_configuration_table_t       *ConfigurationTable;
} efi_system_table_t;
extern efi_system_table_t *ST;
#define gST ST

/*** Simple File System Protocol ***/
#ifndef EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID
#define EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID { 0x964e5b22, 0x6459, 0x11d2, {0x8e, 0x39, 0x0, 0xa0, 0xc9, 0x69, 0x72, 0x3b} }

#define EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_REVISION  0x00010000
#define EFI_FILE_IO_INTERFACE_REVISION EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_REVISION

#define EFI_FILE_MODE_READ      0x0000000000000001
#define EFI_FILE_MODE_WRITE     0x0000000000000002
#define EFI_FILE_MODE_CREATE    0x8000000000000000

#define EFI_FILE_READ_ONLY      0x0000000000000001
#define EFI_FILE_HIDDEN         0x0000000000000002
#define EFI_FILE_SYSTEM         0x0000000000000004
#define EFI_FILE_RESERVED       0x0000000000000008
#define EFI_FILE_DIRECTORY      0x0000000000000010
#define EFI_FILE_ARCHIVE        0x0000000000000020
#define EFI_FILE_VALID_ATTR     0x0000000000000037

#define EFI_FILE_PROTOCOL_REVISION         0x00010000
#define EFI_FILE_HANDLE_REVISION           EFI_FILE_PROTOCOL_REVISION
#endif

#ifndef EFI_FILE_INFO_GUID
#define EFI_FILE_INFO_GUID  { 0x9576e92, 0x6d3f, 0x11d2, {0x8e, 0x39, 0x0, 0xa0, 0xc9, 0x69, 0x72, 0x3b} }
#endif

#ifndef FILENAME_MAX
#define FILENAME_MAX 262    /* from FAT spec */
#endif

typedef struct {
    uint64_t                Size;
    uint64_t                FileSize;
    uint64_t                PhysicalSize;
    efi_time_t              CreateTime;
    efi_time_t              LastAccessTime;
    efi_time_t              ModificationTime;
    uint64_t                Attribute;
    wchar_t                 FileName[FILENAME_MAX];
} efi_file_info_t;

typedef struct efi_file_handle_s efi_file_handle_t;

typedef efi_status_t (EFIAPI *efi_volume_open_t)(void *This, efi_file_handle_t **Root);
typedef struct {
    uint64_t                Revision;
    efi_volume_open_t       OpenVolume;
} efi_simple_file_system_protocol_t;

typedef efi_status_t (EFIAPI *efi_file_open_t)(efi_file_handle_t *File, efi_file_handle_t **NewHandle, wchar_t *FileName,
    uint64_t OpenMode, uint64_t Attributes);
typedef efi_status_t (EFIAPI *efi_file_close_t)(efi_file_handle_t *File);
typedef efi_status_t (EFIAPI *efi_file_delete_t)(efi_file_handle_t *File);
typedef efi_status_t (EFIAPI *efi_file_read_t)(efi_file_handle_t *File, uintn_t *BufferSize, void *Buffer);
typedef efi_status_t (EFIAPI *efi_file_write_t)(efi_file_handle_t *File, uintn_t *BufferSize, void *Buffer);
typedef efi_status_t (EFIAPI *efi_file_get_pos_t)(efi_file_handle_t *File, uint64_t *Position);
typedef efi_status_t (EFIAPI *efi_file_set_pos_t)(efi_file_handle_t *File, uint64_t Position);
typedef efi_status_t (EFIAPI *efi_file_get_info_t)(efi_file_handle_t *File, efi_guid_t *InformationType, uintn_t *BufferSize,
    void *Buffer);
typedef efi_status_t (EFIAPI *efi_file_set_info_t)(efi_file_handle_t *File, efi_guid_t *InformationType, uintn_t BufferSize,
    void *Buffer);
typedef efi_status_t (EFIAPI *efi_file_flush_t)(efi_file_handle_t *File);

struct efi_file_handle_s {
    uint64_t                Revision;
    efi_file_open_t         Open;
    efi_file_close_t        Close;
    efi_file_delete_t       Delete;
    efi_file_read_t         Read;
    efi_file_write_t        Write;
    efi_file_get_pos_t      GetPosition;
    efi_file_set_pos_t      SetPosition;
    efi_file_get_info_t     GetInfo;
    efi_file_set_info_t     SetInfo;
    efi_file_flush_t        Flush;
};

/*** Shell Parameter Protocols ***/
#ifndef EFI_SHELL_PARAMETERS_PROTOCOL_GUID
#define EFI_SHELL_PARAMETERS_PROTOCOL_GUID  { 0x752f3136, 0x4e16, 0x4fdc, {0xa2, 0x2a, 0xe5, 0xf4, 0x68, 0x12, 0xf4, 0xca} }
#endif

typedef struct {
    wchar_t         **Argv;
    uintn_t         Argc;
    efi_handle_t    StdIn;
    efi_handle_t    StdOut;
    efi_handle_t    StdErr;
} efi_shell_parameters_protocol_t;

#ifndef SHELL_INTERFACE_PROTOCOL_GUID
#define SHELL_INTERFACE_PROTOCOL_GUID       { 0x47c7b223, 0xc42a, 0x11d2, {0x8e, 0x57, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b} }
#endif

typedef struct {
    efi_handle_t    ImageHandle;
    void*           *Info;
    wchar_t         **Argv;
    uintn_t         Argc;
    wchar_t         **RedirArgv;
    uintn_t         RedirArgc;
    efi_handle_t    StdIn;
    efi_handle_t    StdOut;
    efi_handle_t    StdErr;
} efi_shell_interface_protocol_t;

/*** Random Number Generator ***/
#ifndef EFI_RNG_PROTOCOL_GUID
#define EFI_RNG_PROTOCOL_GUID               { 0x3152bca5, 0xeade, 0x433d, {0x86, 0x2e, 0xc0, 0x1c, 0xdc, 0x29, 0x1f, 0x44} }
#endif

typedef efi_status_t (EFIAPI *efi_rng_get_info_t)(void *This, uintn_t *RNGAlgorithmListSize, efi_guid_t *RNGAlgorithmList);
typedef efi_status_t (EFIAPI *efi_rng_get_rng_t)(void *This, efi_guid_t *RNGAlgorithm, uintn_t RNGValueLength, uint8_t *RNGValue);

typedef struct {
    efi_rng_get_info_t  GetInfo;
    efi_rng_get_rng_t   GetRNG;
} efi_rng_protocol_t;

/*** Serial IO Protocol ***/
#ifndef EFI_SERIAL_IO_PROTOCOL_GUID
#define EFI_SERIAL_IO_PROTOCOL_GUID { 0xBB25CF6F, 0xF1D4, 0x11D2, {0x9A, 0x0C, 0x00, 0x90, 0x27, 0x3F, 0xC1, 0xFD} }

#define SERIAL_IO_INTERFACE_REVISION                0x00010000
#define EFI_SERIAL_CLEAR_TO_SEND                    0x0010
#define EFI_SERIAL_DATA_SET_READY                   0x0020
#define EFI_SERIAL_RING_INDICATE                    0x0040
#define EFI_SERIAL_CARRIER_DETECT                   0x0080
#define EFI_SERIAL_REQUEST_TO_SEND                  0x0002
#define EFI_SERIAL_DATA_TERMINAL_READY              0x0001
#define EFI_SERIAL_INPUT_BUFFER_EMPTY               0x0100
#define EFI_SERIAL_OUTPUT_BUFFER_EMPTY              0x0200
#define EFI_SERIAL_HARDWARE_LOOPBACK_ENABLE         0x1000
#define EFI_SERIAL_SOFTWARE_LOOPBACK_ENABLE         0x2000
#define EFI_SERIAL_HARDWARE_FLOW_CONTROL_ENABLE     0x4000

typedef enum {
    DefaultParity,
    NoParity,
    EvenParity,
    OddParity,
    MarkParity,
    SpaceParity
} efi_parity_type_t;

typedef enum {
    DefaultStopBits,
    OneStopBit,
    OneFiveStopBits,
    TwoStopBits
} efi_stop_bits_type_t;

#else

#define efi_parity_type_t EFI_PARITY_TYPE
#define efi_stop_bits_type_t EFI_STOP_BITS_TYPE

#endif

typedef struct {
    uint32_t                        ControlMask;
    uint32_t                        Timeout;
    uint64_t                        BaudRate;
    uint32_t                        ReceiveFifoDepth;
    uint32_t                        DataBits;
    uint32_t                        Parity;
    uint32_t                        StopBits;
} efi_serial_io_mode_t;

typedef efi_status_t (EFIAPI *efi_serial_reset_t)(void *This);
typedef efi_status_t (EFIAPI *efi_serial_set_attributes_t)(void *This, uint64_t BaudRate, uint32_t ReceiveFifoDepth,
    uint32_t Timeout, efi_parity_type_t Parity, uint8_t DataBits, efi_stop_bits_type_t StopBits);
typedef efi_status_t (EFIAPI *efi_serial_set_control_bits_t)(void *This, uint32_t Control);
typedef efi_status_t (EFIAPI *efi_serial_get_control_bits_t)(void *This, uint32_t *Control);
typedef efi_status_t (EFIAPI *efi_serial_write_t)(void *This, uintn_t *BufferSize, void *Buffer);
typedef efi_status_t (EFIAPI *efi_serial_read_t)(void *This, uintn_t *BufferSize, void *Buffer);

typedef struct {
    uint32_t                        Revision;
    efi_serial_reset_t              Reset;
    efi_serial_set_attributes_t     SetAttributes;
    efi_serial_set_control_bits_t   SetControl;
    efi_serial_get_control_bits_t   GetControl;
    efi_serial_write_t              Write;
    efi_serial_read_t               Read;
    efi_serial_io_mode_t            *Mode;
} efi_serial_io_protocol_t;

/*** Block IO Protocol ***/
#ifndef EFI_BLOCK_IO_PROTOCOL_GUID
#define EFI_BLOCK_IO_PROTOCOL_GUID { 0x964e5b21, 0x6459, 0x11d2, {0x8e, 0x39, 0x0, 0xa0, 0xc9, 0x69, 0x72, 0x3b} }

#define EFI_BLOCK_IO_PROTOCOL_REVISION    0x00010000
#define EFI_BLOCK_IO_INTERFACE_REVISION   EFI_BLOCK_IO_PROTOCOL_REVISION

#endif

typedef struct {
    uint32_t                MediaId;
    boolean_t               RemovableMedia;
    boolean_t               MediaPresent;
    boolean_t               LogicalPartition;
    boolean_t               ReadOnly;
    boolean_t               WriteCaching;
    uint32_t                BlockSize;
    uint32_t                IoAlign;
    efi_lba_t               LastBlock;
} efi_block_io_media_t;

typedef efi_status_t (EFIAPI *efi_block_reset_t)(void *This, boolean_t ExtendedVerification);
typedef efi_status_t (EFIAPI *efi_block_read_t)(void *This, uint32_t MediaId, efi_lba_t LBA, uintn_t BufferSize, void *Buffer);
typedef efi_status_t (EFIAPI *efi_block_write_t)(void *This, uint32_t MediaId, efi_lba_t LBA, uintn_t BufferSize, void *Buffer);
typedef efi_status_t (EFIAPI *efi_block_flush_t)(void *This);

typedef struct {
    uint64_t                Revision;
    efi_block_io_media_t    *Media;
    efi_block_reset_t       Reset;
    efi_block_read_t        ReadBlocks;
    efi_block_write_t       WriteBlocks;
    efi_block_flush_t       FlushBlocks;
} efi_block_io_t;

typedef struct {
    off_t                   offset;
    efi_block_io_t          *bio;
} block_file_t;

/*** Graphics Output Protocol (not used, but could be useful to have) ***/
#ifndef EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID
#define EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID { 0x9042a9de, 0x23dc, 0x4a38, {0x96, 0xfb, 0x7a, 0xde, 0xd0, 0x80, 0x51, 0x6a } }

typedef enum {
  PixelRedGreenBlueReserved8BitPerColor,
  PixelBlueGreenRedReserved8BitPerColor,
  PixelBitMask,
  PixelBltOnly,
  PixelFormatMax
} efi_gop_pixel_format_t;

typedef enum {
  EfiBltVideoFill,
  EfiBltVideoToBltBuffer,
  EfiBltBufferToVideo,
  EfiBltVideoToVideo,
  EfiGraphicsOutputBltOperationMax
} efi_gop_blt_operation_t;

#else

#define efi_gop_pixel_format_t EFI_GRAPHICS_PIXEL_FORMAT
#define efi_gop_blt_operation_t EFI_GRAPHICS_OUTPUT_BLT_OPERATION

#endif

typedef struct {
  uint32_t                  RedMask;
  uint32_t                  GreenMask;
  uint32_t                  BlueMask;
  uint32_t                  ReservedMask;
} efi_gop_pixel_bitmask_t;

typedef struct {
    uint32_t                Version;
    uint32_t                HorizontalResolution;
    uint32_t                VerticalResolution;
    efi_gop_pixel_format_t  PixelFormat;
    efi_gop_pixel_bitmask_t PixelInformation;
    uint32_t                PixelsPerScanLine;
} efi_gop_mode_info_t;

typedef struct {
    uint32_t                MaxMode;
    uint32_t                Mode;
    efi_gop_mode_info_t     *Information;
    uintn_t                 SizeOfInfo;
    efi_physical_address_t  FrameBufferBase;
    uintn_t                 FrameBufferSize;
} efi_gop_mode_t;

typedef efi_status_t (EFIAPI *efi_gop_query_mode_t)(void *This, uint32_t ModeNumber, uintn_t *SizeOfInfo,
    efi_gop_mode_info_t **Info);
typedef efi_status_t (EFIAPI *efi_gop_set_mode_t)(void *This, uint32_t ModeNumber);
typedef efi_status_t (EFIAPI *efi_gop_blt_t)(void *This, uint32_t *BltBuffer, efi_gop_blt_operation_t BltOperation,
    uintn_t SourceX, uintn_t SourceY, uintn_t DestinationX, uintn_t DestionationY, uintn_t Width, uintn_t Height, uintn_t Delta);

typedef struct {
    efi_gop_query_mode_t    QueryMode;
    efi_gop_set_mode_t      SetMode;
    efi_gop_blt_t           Blt;
    efi_gop_mode_t          *Mode;
} efi_gop_t;

/*** Simple Pointer Protocol (not used, but could be useful to have) ***/
#ifndef EFI_SIMPLE_POINTER_PROTOCOL_GUID
#define EFI_SIMPLE_POINTER_PROTOCOL_GUID { 0x31878c87, 0xb75, 0x11d5, { 0x9a, 0x4f, 0x0, 0x90, 0x27, 0x3f, 0xc1, 0x4d } }
#endif

typedef struct {
    int32_t                 RelativeMovementX;
    int32_t                 RelativeMovementY;
    int32_t                 RelativeMovementZ;
    boolean_t               LeftButton;
    boolean_t               RightButton;
} efi_simple_pointer_state_t;

typedef struct {
    uint64_t                ResolutionX;
    uint64_t                ResolutionY;
    uint64_t                ResolutionZ;
    boolean_t               LeftButton;
    boolean_t               RightButton;
} efi_simple_pointer_mode_t;

typedef efi_status_t (EFIAPI *efi_simple_pointer_reset_t) (void *This, boolean_t ExtendedVerification);
typedef efi_status_t (EFIAPI *efi_simple_pointer_get_state_t) (void *This, efi_simple_pointer_state_t *State);

typedef struct {
    efi_simple_pointer_reset_t Reset;
    efi_simple_pointer_get_state_t GetState;
    efi_event_t WaitForInput;
    efi_simple_pointer_mode_t *Mode;
} efi_simple_pointer_protocol_t;

/*** Option ROM Protocol (not used, but could be useful to have) ***/
#ifndef EFI_PCI_OPTION_ROM_TABLE_GUID
#define EFI_PCI_OPTION_ROM_TABLE_GUID { 0x7462660f, 0x1cbd, 0x48da, {0xad, 0x11, 0x91, 0x71, 0x79, 0x13, 0x83, 0x1c} }
#endif

typedef struct {
  efi_physical_address_t            RomAddress;
  efi_memory_type_t                 MemoryType;
  uint32_t                          RomLength;
  uint32_t                          Seg;
  uint8_t                           Bus;
  uint8_t                           Dev;
  uint8_t                           Func;
  boolean_t                         ExecutedLegacyBiosImage;
  boolean_t                         DontLoadEfiRom;
} efi_pci_option_rom_descriptor_t;

typedef struct {
  uint64_t                          PciOptionRomCount;
  efi_pci_option_rom_descriptor_t   *PciOptionRomDescriptors;
} efi_pci_option_rom_table_t;

/*** GPT partitioning table (not used, but could be useful to have) ***/
typedef struct {
    efi_table_header_t  Header;
    efi_lba_t           MyLBA;
    efi_lba_t           AlternateLBA;
    efi_lba_t           FirstUsableLBA;
    efi_lba_t           LastUsableLBA;
    efi_guid_t          DiskGUID;
    efi_lba_t           PartitionEntryLBA;
    uint32_t            NumberOfPartitionEntries;
    uint32_t            SizeOfPartitionEntry;
    uint32_t            PartitionEntryArrayCRC32;
} efi_partition_table_header_t;

typedef struct {
    efi_guid_t  PartitionTypeGUID;
    efi_guid_t  UniquePartitionGUID;
    efi_lba_t   StartingLBA;
    efi_lba_t   EndingLBA;
    uint64_t    Attributes;
    wchar_t     PartitionName[36];
} efi_partition_entry_t;

/*** POSIX definitions ***/
#define abs(x) ((x)<0?-(x):(x))
#define min(x,y) ((x)<(y)?(x):(y))
#define max(x,y) ((x)>(y)?(x):(y))

/* dirent.h */
#define IFTODT(mode)    (((mode) & 0170000) >> 12)
#define DTTOIF(dirtype) ((dirtype) << 12)
#define DT_DIR          4
#define DT_REG          8
struct dirent {
    unsigned short int d_reclen;
    unsigned char d_type;
    char_t d_name[FILENAME_MAX];
};
typedef struct efi_file_handle_s DIR;
extern DIR *opendir (const char_t *__name);
extern struct dirent *readdir (DIR *__dirp);
extern void rewinddir (DIR *__dirp);
extern int closedir (DIR *__dirp);

/* errno.h */
extern int errno;
#define	EPERM		 1	/* Operation not permitted */
#define	ENOENT		 2	/* No such file or directory */
#define	ESRCH		 3	/* No such process */
#define	EINTR		 4	/* Interrupted system call */
#define	EIO		 5	/* I/O error */
#define	ENXIO		 6	/* No such device or address */
#define	E2BIG		 7	/* Argument list too long */
#define	ENOEXEC		 8	/* Exec format error */
#define	EBADF		 9	/* Bad file number */
#define	ECHILD		10	/* No child processes */
#define	EAGAIN		11	/* Try again */
#define	ENOMEM		12	/* Out of memory */
#define	EACCES		13	/* Permission denied */
#define	EFAULT		14	/* Bad address */
#define	ENOTBLK		15	/* Block device required */
#define	EBUSY		16	/* Device or resource busy */
#define	EEXIST		17	/* File exists */
#define	EXDEV		18	/* Cross-device link */
#define	ENODEV		19	/* No such device */
#define	ENOTDIR		20	/* Not a directory */
#define	EISDIR		21	/* Is a directory */
#define	EINVAL		22	/* Invalid argument */
#define	ENFILE		23	/* File table overflow */
#define	EMFILE		24	/* Too many open files */
#define	ENOTTY		25	/* Not a typewriter */
#define	ETXTBSY		26	/* Text file busy */
#define	EFBIG		27	/* File too large */
#define	ENOSPC		28	/* No space left on device */
#define	ESPIPE		29	/* Illegal seek */
#define	EROFS		30	/* Read-only file system */
#define	EMLINK		31	/* Too many links */
#define	EPIPE		32	/* Broken pipe */
#define	EDOM		33	/* Math argument out of domain of func */
#define	ERANGE		34	/* Math result not representable */

/* stdlib.h */
#define RAND_MAX       2147483647
typedef int (*__compar_fn_t) (const void *, const void *);
extern int atoi (const char_t *__nptr);
extern int64_t atol (const char_t *__nptr);
extern int64_t strtol (const char_t *__nptr, char_t **__endptr, int __base);
extern void *malloc (size_t __size);
extern void *calloc (size_t __nmemb, size_t __size);
extern void *realloc (void *__ptr, size_t __size);
extern void free (void *__ptr);
extern void abort (void);
extern void exit (int __status);
/* exit Boot Services function. Returns 0 on success. */
extern int exit_bs(void);
extern void *bsearch (const void *__key, const void *__base, size_t __nmemb, size_t __size, __compar_fn_t __compar);
extern void qsort (void *__base, size_t __nmemb, size_t __size, __compar_fn_t __compar);
extern int mblen (const char *__s, size_t __n);
extern int mbtowc (wchar_t * __pwc, const char * __s, size_t __n);
extern int wctomb (char *__s, wchar_t __wchar);
extern size_t mbstowcs (wchar_t *__pwcs, const char *__s, size_t __n);
extern size_t wcstombs (char *__s, const wchar_t *__pwcs, size_t __n);
extern void srand(unsigned int __seed);
extern int rand(void);
extern uint8_t *getenv(char_t *name, uintn_t *len);
extern int setenv(char_t *name, uintn_t len, uint8_t *data);

/* stdio.h */
#ifndef BUFSIZ
#define BUFSIZ 8192
#endif
#define SEEK_SET	0	/* Seek from beginning of file.  */
#define SEEK_CUR	1	/* Seek from current position.  */
#define SEEK_END	2	/* Seek from end of file.  */
#define stdin (FILE*)ST->ConsoleInHandle
#define stdout (FILE*)ST->ConsoleOutHandle
#define stderr (FILE*)ST->ConsoleErrorHandle
typedef struct efi_file_handle_s FILE;
extern int fclose (FILE *__stream);
extern int fflush (FILE *__stream);
extern int remove (const char_t *__filename);
extern FILE *fopen (const char_t *__filename, const char_t *__modes);
extern size_t fread (void *__ptr, size_t __size, size_t __n, FILE *__stream);
extern size_t fwrite (const void *__ptr, size_t __size, size_t __n, FILE *__s);
extern int fseek (FILE *__stream, long int __off, int __whence);
extern long int ftell (FILE *__stream);
extern int feof (FILE *__stream);
extern int fprintf (FILE *__stream, const char_t *__format, ...);
extern int printf (const char_t *__format, ...);
extern int sprintf (char_t *__s, const char_t *__format, ...);
extern int vfprintf (FILE *__s, const char_t *__format, __builtin_va_list __arg);
extern int vprintf (const char_t *__format, __builtin_va_list __arg);
extern int vsprintf (char_t *__s, const char_t *__format, __builtin_va_list __arg);
extern int snprintf (char_t *__s, size_t __maxlen, const char_t *__format, ...);
extern int vsnprintf (char_t *__s, size_t __maxlen, const char_t *__format, __builtin_va_list __arg);
extern int getchar (void);
/* non-blocking, only returns UNICODE if there's any key pressed, 0 otherwise */
extern int getchar_ifany (void);
extern int putchar (int __c);

/* string.h */
extern void *memcpy(void *__dest, const void *__src, size_t __n);
extern void *memmove(void *__dest, const void *__src, size_t __n);
extern void *memset(void *__s, int __c, size_t __n);
extern int memcmp(const void *__s1, const void *__s2, size_t __n);
extern void *memchr(const void *__s, int __c, size_t __n);
extern void *memrchr(const void *__s, int __c, size_t __n);
void *memmem(const void *haystack, size_t hl, const void *needle, size_t nl);
void *memrmem(const void *haystack, size_t hl, const void *needle, size_t nl);
extern char_t *strcpy (char_t *__dest, const char_t *__src);
extern char_t *strncpy (char_t *__dest, const char_t *__src, size_t __n);
extern char_t *strcat (char_t *__dest, const char_t *__src);
extern char_t *strncat (char_t *__dest, const char_t *__src, size_t __n);
extern int strcmp (const char_t *__s1, const char_t *__s2);
extern int strncmp (const char_t *__s1, const char_t *__s2, size_t __n);
extern char_t *strdup (const char_t *__s);
extern char_t *strchr (const char_t *__s, int __c);
extern char_t *strrchr (const char_t *__s, int __c);
extern char_t *strstr (const char_t *__haystack, const char_t *__needle);
extern char_t *strtok (char_t *__s, const char_t *__delim);
extern char_t *strtok_r (char_t *__s, const char_t *__delim, char_t **__save_ptr);
extern size_t strlen (const char_t *__s);

/* sys/stat.h */
#define S_IREAD    0400 /* Read by owner.  */
#define S_IWRITE   0200 /* Write by owner.  */
#define S_IFMT  0170000 /* These bits determine file type.  */
#define S_IFIFO 0010000 /* FIFO.  */
#define S_IFCHR 0020000 /* Character device.  */
#define S_IFDIR 0040000 /* Directory.  */
#define S_IFBLK 0060000 /* Block device.  */
#define S_IFREG 0100000 /* Regular file.  */
#define S_ISTYPE(mode, mask)    (((mode) & S_IFMT) == (mask))
#define S_ISCHR(mode)   S_ISTYPE((mode), S_IFCHR)
#define S_ISDIR(mode)   S_ISTYPE((mode), S_IFDIR)
#define S_ISBLK(mode)   S_ISTYPE((mode), S_IFBLK)
#define S_ISREG(mode)   S_ISTYPE((mode), S_IFREG)
#define S_ISFIFO(mode)  S_ISTYPE((mode), S_IFIFO)
struct stat {
    mode_t      st_mode;
    off_t       st_size;
    blkcnt_t    st_blocks;
    time_t      st_atime;
    time_t      st_mtime;
    time_t      st_ctime;
};
extern int stat (const char_t *__file, struct stat *__buf);
extern int fstat (FILE *__f, struct stat *__buf);
extern int mkdir (const char_t *__path, mode_t __mode);

/* time.h */
struct tm {
  int tm_sec;   /* Seconds. [0-60] (1 leap second) */
  int tm_min;   /* Minutes. [0-59] */
  int tm_hour;  /* Hours.   [0-23] */
  int tm_mday;  /* Day.	    [1-31] */
  int tm_mon;   /* Month.   [0-11] */
  int tm_year;  /* Year     - 1900.  */
  int tm_wday;  /* Day of week. [0-6] (not set) */
  int tm_yday;  /* Days in year.[0-365] (not set) */
  int tm_isdst; /* DST.     [-1/0/1]*/
};
extern struct tm *localtime (const time_t *__timer);
extern time_t mktime(const struct tm *__tm);
extern time_t time(time_t *__timer);

/* unistd.h */
extern unsigned int sleep (unsigned int __seconds);
extern int usleep (unsigned long int __useconds);
extern int unlink (const wchar_t *__filename);
extern int rmdir (const wchar_t *__filename);

#ifdef  __cplusplus
}
#endif

#endif /* _UEFI_H_ */
