/*++

Copyright (c) 1998  Intel Corporation

Module Name:

    efefind.h

Abstract:

    EFI to compile bindings




Revision History

--*/

#ifndef __GNUC__
#pragma pack()
#endif

//
// Basic int types of various widths
//

#if !defined(__STDC_VERSION__) || (__STDC_VERSION__ < 199901L ) && !defined(__cplusplus)

    // No ANSI C 1999/2000 stdint.h integer width declarations 

    #if defined(_MSC_EXTENSIONS)

        // Use Microsoft C compiler integer width declarations 

        typedef unsigned __int64    uint64_t;
        typedef __int64             int64_t;
        typedef unsigned __int32    uint32_t;
        typedef __int32             int32_t;
        typedef unsigned short      uint16_t;
        typedef short               int16_t;
        typedef unsigned char       uint8_t;
        typedef char                int8_t;
    #elif defined(__GNUC__)
        typedef int __attribute__((__mode__(__DI__)))           int64_t;
        typedef unsigned int __attribute__((__mode__(__DI__)))  uint64_t;
        typedef unsigned int        uint32_t;
        typedef int                 int32_t;
        typedef unsigned short      uint16_t;
        typedef short               int16_t;
        typedef unsigned char       uint8_t;
        typedef signed char         int8_t;
    #elif defined(UNIX_LP64)

        /*  Use LP64 programming model from C_FLAGS for integer width declarations */

       typedef unsigned long       uint64_t;
       typedef long                int64_t;
       typedef unsigned int        uint32_t;
       typedef int                 int32_t;
       typedef unsigned short      uint16_t;
       typedef short               int16_t;
       typedef unsigned char       uint8_t;
       typedef char                int8_t;
    #else

       /*  Assume P64 programming model from C_FLAGS for integer width declarations */

       typedef unsigned long long  uint64_t __attribute__((aligned (8)));
       typedef long long           int64_t __attribute__((aligned (8)));
       typedef unsigned int        uint32_t;
       typedef int                 int32_t;
       typedef unsigned short      uint16_t;
       typedef short               int16_t;
       typedef unsigned char       uint8_t;
       typedef char                int8_t;
    #endif
    typedef uint32_t            uintptr_t;
    typedef int32_t             intptr_t;
#elif defined(__GNUC__)
    #include <stdint.h>
#endif

//
// Basic EFI types of various widths
//

#ifndef __WCHAR_TYPE__
# define __WCHAR_TYPE__ short
#endif

typedef uint64_t   UINT64;
typedef int64_t    INT64;

#ifndef _BASETSD_H_
    typedef uint32_t   UINT32;
    typedef int32_t    INT32;
#endif

typedef uint16_t   UINT16;
typedef int16_t    INT16;
typedef uint8_t    UINT8;
typedef int8_t     INT8;
typedef __WCHAR_TYPE__ WCHAR;

#undef VOID
#define VOID    void


typedef int32_t    INTN;
typedef uint32_t   UINTN;

#ifdef EFI_NT_EMULATOR
    #define POST_CODE(_Data)
#else    
    #ifdef EFI_DEBUG
#define POST_CODE(_Data)    __asm mov eax,(_Data) __asm out 0x80,al
    #else
        #define POST_CODE(_Data)
    #endif  
#endif

#define EFIERR(a)           (0x80000000 | a)
#define EFI_ERROR_MASK      0x80000000
#define EFIERR_OEM(a)       (0xc0000000 | a)      


#define BAD_POINTER         0xFBFBFBFB
#define MAX_ADDRESS         0xFFFFFFFF

#ifdef EFI_NT_EMULATOR
    #define BREAKPOINT()        __asm { int 3 }
#else
    #define BREAKPOINT()        while (TRUE);    // Make it hang on Bios[Dbg]32
#endif

//
// Pointers must be aligned to these address to function
//

#define MIN_ALIGNMENT_SIZE  4

#define ALIGN_VARIABLE(Value ,Adjustment) \
            (UINTN)Adjustment = 0; \
            if((UINTN)Value % MIN_ALIGNMENT_SIZE) \
                (UINTN)Adjustment = MIN_ALIGNMENT_SIZE - ((UINTN)Value % MIN_ALIGNMENT_SIZE); \
            Value = (UINTN)Value + (UINTN)Adjustment


//
// Define macros to build data structure signatures from characters.
//

#define EFI_SIGNATURE_16(A,B)             ((A) | (B<<8))
#define EFI_SIGNATURE_32(A,B,C,D)         (EFI_SIGNATURE_16(A,B)     | (EFI_SIGNATURE_16(C,D)     << 16))
#define EFI_SIGNATURE_64(A,B,C,D,E,F,G,H) (EFI_SIGNATURE_32(A,B,C,D) | ((UINT64)(EFI_SIGNATURE_32(E,F,G,H)) << 32))
//
// To export & import functions in the EFI emulator environment
//

#ifdef EFI_NT_EMULATOR
    #define EXPORTAPI           __declspec( dllexport )
#else
    #define EXPORTAPI
#endif


//
// EFIAPI - prototype calling convention for EFI function pointers
// BOOTSERVICE - prototype for implementation of a boot service interface
// RUNTIMESERVICE - prototype for implementation of a runtime service interface
// RUNTIMEFUNCTION - prototype for implementation of a runtime function that is not a service
// RUNTIME_CODE - pragma macro for declaring runtime code    
//

#ifndef EFIAPI                  // Forces EFI calling conventions reguardless of compiler options 
    #ifdef _MSC_EXTENSIONS
        #define EFIAPI __cdecl  // Force C calling convention for Microsoft C compiler 
    #else
        #define EFIAPI          // Substitute expresion to force C calling convention 
    #endif
#endif

#define BOOTSERVICE
//#define RUNTIMESERVICE(proto,a)    alloc_text("rtcode",a); proto a
//#define RUNTIMEFUNCTION(proto,a)   alloc_text("rtcode",a); proto a
#define RUNTIMESERVICE
#define RUNTIMEFUNCTION


#define RUNTIME_CODE(a)         alloc_text("rtcode", a)
#define BEGIN_RUNTIME_DATA()    data_seg("rtdata")
#define END_RUNTIME_DATA()      data_seg("")

#define VOLATILE    volatile

#define MEMORY_FENCE()    

#ifdef EFI_NT_EMULATOR

//
// To help ensure proper coding of integrated drivers, they are
// compiled as DLLs.  In NT they require a dll init entry pointer.
// The macro puts a stub entry point into the DLL so it will load.
//

#define EFI_DRIVER_ENTRY_POINT(InitFunction)    \
    UINTN                                       \
    __stdcall                                   \
    _DllMainCRTStartup (                        \
        UINTN    Inst,                          \
        UINTN    reason_for_call,               \
        VOID    *rserved                        \
        )                                       \
    {                                           \
        return 1;                               \
    }                                           \
                                                \
    int                                         \
    EXPORTAPI                                   \
    __cdecl                                     \
    InitializeDriver (                          \
        void *ImageHandle,                      \
        void *SystemTable                       \
        )                                       \
    {                                           \
        return InitFunction(ImageHandle, SystemTable);       \
    }


    #define LOAD_INTERNAL_DRIVER(_if, type, name, entry)      \
        (_if)->LoadInternal(type, name, NULL)             

#else // EFI_NT_EMULATOR 

//
// When build similiar to FW, then link everything together as
// one big module. For the MSVC toolchain, we simply tell the
// linker what our driver init function is using /ENTRY.
//
#if defined(_MSC_EXTENSIONS)
    #define EFI_DRIVER_ENTRY_POINT(InitFunction) \
        __pragma(comment(linker, "/ENTRY:" # InitFunction))
#else
    #define EFI_DRIVER_ENTRY_POINT(InitFunction)    \
        UINTN                                       \
        InitializeDriver (                          \
            VOID    *ImageHandle,                   \
            VOID    *SystemTable                    \
            )                                       \
        {                                           \
            return InitFunction(ImageHandle,        \
                    SystemTable);                   \
        }                                           \
                                                    \
        EFI_STATUS efi_main(                        \
            EFI_HANDLE image,                       \
            EFI_SYSTEM_TABLE *systab                \
            ) __attribute__((weak,                  \
                    alias ("InitializeDriver")));
#endif

    #define LOAD_INTERNAL_DRIVER(_if, type, name, entry)    \
            (_if)->LoadInternal(type, name, entry)

#endif // EFI_FW_NT 

//
// Some compilers don't support the forward reference construct:
//  typedef struct XXXXX
//
// The following macro provide a workaround for such cases.
//
#ifdef NO_INTERFACE_DECL
#define INTERFACE_DECL(x)
#else
#if defined(__GNUC__) || defined(_MSC_EXTENSIONS)
#define INTERFACE_DECL(x) struct x
#else
#define INTERFACE_DECL(x) typedef struct x
#endif
#endif

/* No efi call wrapper for IA32 architecture */
#define uefi_call_wrapper(func, va_num, ...)	func(__VA_ARGS__)
#define EFI_FUNCTION

#ifdef _MSC_EXTENSIONS
#pragma warning ( disable : 4731 )  // Suppress warnings about modification of EBP
#endif

