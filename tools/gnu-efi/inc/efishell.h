/**
  EFI Shell protocol as defined in the UEFI Shell Specification 2.2.

  (C) Copyright 2014 Hewlett-Packard Development Company, L.P.<BR>
  Copyright (c) 2006 - 2018, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

  This file is based on MdePkg/Include/Protocol/Shell.h from EDK2
  Ported to gnu-efi by Jiaqing Zhao <jiaqing.zhao@intel.com>
**/

#ifndef _EFI_SHELL_H
#define _EFI_SHELL_H

#include "efilink.h"

#define EFI_SHELL_PROTOCOL_GUID \
    { 0x6302d008, 0x7f9b, 0x4f30, { 0x87, 0xac, 0x60, 0xc9, 0xfe, 0xf5, 0xda, 0x4e } }

INTERFACE_DECL(_EFI_SHELL_PROTOCOL);

typedef enum {
    SHELL_SUCCESS              = 0,
    SHELL_LOAD_ERROR           = 1,
    SHELL_INVALID_PARAMETER    = 2,
    SHELL_UNSUPPORTED          = 3,
    SHELL_BAD_BUFFER_SIZE      = 4,
    SHELL_BUFFER_TOO_SMALL     = 5,
    SHELL_NOT_READY            = 6,
    SHELL_DEVICE_ERROR         = 7,
    SHELL_WRITE_PROTECTED      = 8,
    SHELL_OUT_OF_RESOURCES     = 9,
    SHELL_VOLUME_CORRUPTED     = 10,
    SHELL_VOLUME_FULL          = 11,
    SHELL_NO_MEDIA             = 12,
    SHELL_MEDIA_CHANGED        = 13,
    SHELL_NOT_FOUND            = 14,
    SHELL_ACCESS_DENIED        = 15,
    SHELL_TIMEOUT              = 18,
    SHELL_NOT_STARTED          = 19,
    SHELL_ALREADY_STARTED      = 20,
    SHELL_ABORTED              = 21,
    SHELL_INCOMPATIBLE_VERSION = 25,
    SHELL_SECURITY_VIOLATION   = 26,
    SHELL_NOT_EQUAL            = 27
} SHELL_STATUS;

typedef VOID *SHELL_FILE_HANDLE;

typedef struct {
    EFI_LIST_ENTRY    Link;
    EFI_STATUS        Status;
    CONST CHAR16      *FullName;
    CONST CHAR16      *FileName;
    SHELL_FILE_HANDLE Handle;
    EFI_FILE_INFO     *Info;
} EFI_SHELL_FILE_INFO;

typedef
BOOLEAN
(EFIAPI *EFI_SHELL_BATCH_IS_ACTIVE) (
    VOID
    );

typedef
EFI_STATUS
(EFIAPI *EFI_SHELL_CLOSE_FILE) (
    IN SHELL_FILE_HANDLE FileHandle
    );

typedef
EFI_STATUS
(EFIAPI *EFI_SHELL_CREATE_FILE) (
    IN CONST CHAR16       *FileName,
    IN UINT64             FileAttribs,
    OUT SHELL_FILE_HANDLE *FileHandle
    );

typedef
EFI_STATUS
(EFIAPI *EFI_SHELL_DELETE_FILE) (
    IN SHELL_FILE_HANDLE FileHandle
    );

typedef
EFI_STATUS
(EFIAPI *EFI_SHELL_DELETE_FILE_BY_NAME) (
    IN CONST CHAR16 *FileName
    );

typedef
VOID
(EFIAPI *EFI_SHELL_DISABLE_PAGE_BREAK) (
    VOID
    );

typedef
VOID
(EFIAPI *EFI_SHELL_ENABLE_PAGE_BREAK) (
    VOID
    );

typedef
EFI_STATUS
(EFIAPI *EFI_SHELL_EXECUTE) (
    IN EFI_HANDLE  *ParentImageHandle,
    IN CHAR16      *CommandLine OPTIONAL,
    IN CHAR16      **Environment OPTIONAL,
    OUT EFI_STATUS *StatusCode OPTIONAL
    );

typedef
EFI_STATUS
(EFIAPI *EFI_SHELL_FIND_FILES) (
    IN CONST CHAR16         *FilePattern,
    OUT EFI_SHELL_FILE_INFO **FileList
    );

typedef
EFI_STATUS
(EFIAPI *EFI_SHELL_FIND_FILES_IN_DIR) (
    IN SHELL_FILE_HANDLE    FileDirHandle,
    OUT EFI_SHELL_FILE_INFO **FileList
    );

typedef
EFI_STATUS
(EFIAPI *EFI_SHELL_FLUSH_FILE) (
    IN SHELL_FILE_HANDLE FileHandle
    );

typedef
EFI_STATUS
(EFIAPI *EFI_SHELL_FREE_FILE_LIST) (
    IN EFI_SHELL_FILE_INFO **FileList
    );

typedef
CONST CHAR16 *
(EFIAPI *EFI_SHELL_GET_ALIAS) (
  IN  CONST CHAR16 *Alias,
  OUT BOOLEAN      *Volatile OPTIONAL
  );

typedef
CONST CHAR16 *
(EFIAPI *EFI_SHELL_GET_CUR_DIR) (
  IN CONST CHAR16 *FileSystemMapping OPTIONAL
  );

typedef UINT32 EFI_SHELL_DEVICE_NAME_FLAGS;
#define EFI_DEVICE_NAME_USE_COMPONENT_NAME 0x00000001
#define EFI_DEVICE_NAME_USE_DEVICE_PATH    0x00000002

typedef
EFI_STATUS
(EFIAPI *EFI_SHELL_GET_DEVICE_NAME) (
    IN EFI_HANDLE                   DeviceHandle,
    IN EFI_SHELL_DEVICE_NAME_FLAGS  Flags,
    IN CHAR8                        *Language,
    OUT CHAR16                      **BestDeviceName
    );

typedef
CONST EFI_DEVICE_PATH_PROTOCOL *
(EFIAPI *EFI_SHELL_GET_DEVICE_PATH_FROM_MAP) (
    IN CONST CHAR16 *Mapping
    );

typedef
EFI_DEVICE_PATH_PROTOCOL *
(EFIAPI *EFI_SHELL_GET_DEVICE_PATH_FROM_FILE_PATH) (
    IN CONST CHAR16 *Path
    );

typedef
CONST CHAR16 *
(EFIAPI *EFI_SHELL_GET_ENV) (
    IN CONST CHAR16 *Name
    );

typedef
CONST CHAR16 *
(EFIAPI *EFI_SHELL_GET_ENV_EX) (
    IN CONST CHAR16 *Name,
    OUT UINT32      *Attributes OPTIONAL
    );

typedef
EFI_FILE_INFO *
(EFIAPI *EFI_SHELL_GET_FILE_INFO) (
    IN SHELL_FILE_HANDLE FileHandle
    );

typedef
CHAR16 *
(EFIAPI *EFI_SHELL_GET_FILE_PATH_FROM_DEVICE_PATH) (
    IN CONST EFI_DEVICE_PATH_PROTOCOL *Path
    );

typedef
EFI_STATUS
(EFIAPI *EFI_SHELL_GET_FILE_POSITION) (
    IN SHELL_FILE_HANDLE FileHandle,
    OUT UINT64           *Position
    );

typedef
EFI_STATUS
(EFIAPI *EFI_SHELL_GET_FILE_SIZE) (
    IN SHELL_FILE_HANDLE FileHandle,
    OUT UINT64           *Size
    );

typedef
EFI_STATUS
(EFIAPI *EFI_SHELL_GET_GUID_FROM_NAME) (
    IN CONST CHAR16 *GuidName,
    OUT EFI_GUID    *Guid
    );

typedef
EFI_STATUS
(EFIAPI *EFI_SHELL_GET_GUID_NAME)(
    IN CONST EFI_GUID *Guid,
    OUT CONST CHAR16  **GuidName
    );

typedef
EFI_STATUS
(EFIAPI *EFI_SHELL_GET_HELP_TEXT) (
    IN CONST CHAR16 *Command,
    IN CONST CHAR16 *Sections,
    OUT CHAR16      **HelpText
    );

typedef
CONST CHAR16 *
(EFIAPI *EFI_SHELL_GET_MAP_FROM_DEVICE_PATH) (
    IN OUT EFI_DEVICE_PATH_PROTOCOL **DevicePath
    );

typedef
BOOLEAN
(EFIAPI *EFI_SHELL_GET_PAGE_BREAK) (
    VOID
    );

typedef
BOOLEAN
(EFIAPI *EFI_SHELL_IS_ROOT_SHELL) (
    VOID
    );

typedef
EFI_STATUS
(EFIAPI *EFI_SHELL_OPEN_FILE_BY_NAME) (
    IN CONST CHAR16       *FileName,
    OUT SHELL_FILE_HANDLE *FileHandle,
    IN UINT64             OpenMode
    );

typedef
EFI_STATUS
(EFIAPI *EFI_SHELL_OPEN_FILE_LIST) (
    IN CHAR16                  *Path,
    IN UINT64                  OpenMode,
    IN OUT EFI_SHELL_FILE_INFO **FileList
    );

typedef
EFI_STATUS
(EFIAPI *EFI_SHELL_OPEN_ROOT) (
    IN EFI_DEVICE_PATH_PROTOCOL *DevicePath,
    OUT SHELL_FILE_HANDLE       *FileHandle
    );

typedef
EFI_STATUS
(EFIAPI *EFI_SHELL_OPEN_ROOT_BY_HANDLE) (
    IN EFI_HANDLE         DeviceHandle,
    OUT SHELL_FILE_HANDLE *FileHandle
    );

typedef
EFI_STATUS
(EFIAPI *EFI_SHELL_READ_FILE) (
    IN SHELL_FILE_HANDLE FileHandle,
    IN OUT UINTN         *ReadSize,
    IN OUT VOID          *Buffer
    );

typedef
EFI_STATUS
(EFIAPI *EFI_SHELL_REGISTER_GUID_NAME) (
    IN CONST EFI_GUID *Guid,
    IN CONST CHAR16   *GuidName
    );

typedef
EFI_STATUS
(EFIAPI *EFI_SHELL_REMOVE_DUP_IN_FILE_LIST) (
    IN EFI_SHELL_FILE_INFO **FileList
    );

typedef
EFI_STATUS
(EFIAPI *EFI_SHELL_SET_ALIAS) (
    IN CONST CHAR16 *Command,
    IN CONST CHAR16 *Alias,
    IN BOOLEAN      Replace,
    IN BOOLEAN      Volatile
    );

typedef
EFI_STATUS
(EFIAPI *EFI_SHELL_SET_CUR_DIR) (
    IN CONST CHAR16 *FileSystem OPTIONAL,
    IN CONST CHAR16 *Dir
    );

typedef
EFI_STATUS
(EFIAPI *EFI_SHELL_SET_ENV) (
    IN CONST CHAR16 *Name,
    IN CONST CHAR16 *Value,
    IN BOOLEAN      Volatile
    );

typedef
EFI_STATUS
(EFIAPI *EFI_SHELL_SET_FILE_INFO) (
    IN SHELL_FILE_HANDLE   FileHandle,
    IN CONST EFI_FILE_INFO *FileInfo
    );

typedef
EFI_STATUS
(EFIAPI *EFI_SHELL_SET_FILE_POSITION) (
    IN SHELL_FILE_HANDLE FileHandle,
    IN UINT64            Position
    );

typedef
EFI_STATUS
(EFIAPI *EFI_SHELL_SET_MAP) (
    IN CONST EFI_DEVICE_PATH_PROTOCOL *DevicePath,
    IN CONST CHAR16                   *Mapping
    );

typedef
EFI_STATUS
(EFIAPI *EFI_SHELL_WRITE_FILE) (
    IN SHELL_FILE_HANDLE FileHandle,
    IN OUT UINTN         *BufferSize,
    IN VOID              *Buffer
    );

typedef struct _EFI_SHELL_PROTOCOL {
    EFI_SHELL_EXECUTE                         Execute;
    EFI_SHELL_GET_ENV                         GetEnv;
    EFI_SHELL_SET_ENV                         SetEnv;
    EFI_SHELL_GET_ALIAS                       GetAlias;
    EFI_SHELL_SET_ALIAS                       SetAlias;
    EFI_SHELL_GET_HELP_TEXT                   GetHelpText;
    EFI_SHELL_GET_DEVICE_PATH_FROM_MAP        GetDevicePathFromMap;
    EFI_SHELL_GET_MAP_FROM_DEVICE_PATH        GetMapFromDevicePath;
    EFI_SHELL_GET_DEVICE_PATH_FROM_FILE_PATH  GetDevicePathFromFilePath;
    EFI_SHELL_GET_FILE_PATH_FROM_DEVICE_PATH  GetFilePathFromDevicePath;
    EFI_SHELL_SET_MAP                         SetMap;
    EFI_SHELL_GET_CUR_DIR                     GetCurDir;
    EFI_SHELL_SET_CUR_DIR                     SetCurDir;
    EFI_SHELL_OPEN_FILE_LIST                  OpenFileList;
    EFI_SHELL_FREE_FILE_LIST                  FreeFileList;
    EFI_SHELL_REMOVE_DUP_IN_FILE_LIST         RemoveDupInFileList;
    EFI_SHELL_BATCH_IS_ACTIVE                 BatchIsActive;
    EFI_SHELL_IS_ROOT_SHELL                   IsRootShell;
    EFI_SHELL_ENABLE_PAGE_BREAK               EnablePageBreak;
    EFI_SHELL_DISABLE_PAGE_BREAK              DisablePageBreak;
    EFI_SHELL_GET_PAGE_BREAK                  GetPageBreak;
    EFI_SHELL_GET_DEVICE_NAME                 GetDeviceName;
    EFI_SHELL_GET_FILE_INFO                   GetFileInfo;
    EFI_SHELL_SET_FILE_INFO                   SetFileInfo;
    EFI_SHELL_OPEN_FILE_BY_NAME               OpenFileByName;
    EFI_SHELL_CLOSE_FILE                      CloseFile;
    EFI_SHELL_CREATE_FILE                     CreateFile;
    EFI_SHELL_READ_FILE                       ReadFile;
    EFI_SHELL_WRITE_FILE                      WriteFile;
    EFI_SHELL_DELETE_FILE                     DeleteFile;
    EFI_SHELL_DELETE_FILE_BY_NAME             DeleteFileByName;
    EFI_SHELL_GET_FILE_POSITION               GetFilePosition;
    EFI_SHELL_SET_FILE_POSITION               SetFilePosition;
    EFI_SHELL_FLUSH_FILE                      FlushFile;
    EFI_SHELL_FIND_FILES                      FindFiles;
    EFI_SHELL_FIND_FILES_IN_DIR               FindFilesInDir;
    EFI_SHELL_GET_FILE_SIZE                   GetFileSize;
    EFI_SHELL_OPEN_ROOT                       OpenRoot;
    EFI_SHELL_OPEN_ROOT_BY_HANDLE             OpenRootByHandle;
    EFI_EVENT                                 ExecutionBreak;
    UINT32                                    MajorVersion;
    UINT32                                    MinorVersion;
    // Added for Shell 2.1
    EFI_SHELL_REGISTER_GUID_NAME              RegisterGuidName;
    EFI_SHELL_GET_GUID_NAME                   GetGuidName;
    EFI_SHELL_GET_GUID_FROM_NAME              GetGuidFromName;
    EFI_SHELL_GET_ENV_EX                      GetEnvEx;
} EFI_SHELL_PROTOCOL;

#define EFI_SHELL_PARAMETERS_PROTOCOL_GUID \
    { 0x752f3136, 0x4e16, 0x4fdc, { 0xa2, 0x2a, 0xe5, 0xf4, 0x68, 0x12, 0xf4, 0xca } }

INTERFACE_DECL(_EFI_SHELL_PARAMETERS_PROTOCOL);

typedef struct _EFI_SHELL_PARAMETERS_PROTOCOL {
    CHAR16            **Argv;
    UINTN             Argc;
    SHELL_FILE_HANDLE StdIn;
    SHELL_FILE_HANDLE StdOut;
    SHELL_FILE_HANDLE StdErr;
} EFI_SHELL_PARAMETERS_PROTOCOL;

#define EFI_SHELL_DYNAMIC_COMMAND_PROTOCOL_GUID \
    { 0x3c7200e9, 0x005f, 0x4ea4, { 0x87, 0xde, 0xa3, 0xdf, 0xac, 0x8a, 0x27, 0xc3 } }

INTERFACE_DECL(_EFI_SHELL_DYNAMIC_COMMAND_PROTOCOL);

typedef
SHELL_STATUS
(EFIAPI *SHELL_COMMAND_HANDLER)(
    IN struct _EFI_SHELL_DYNAMIC_COMMAND_PROTOCOL *This,
    IN EFI_SYSTEM_TABLE                           *SystemTable,
    IN EFI_SHELL_PARAMETERS_PROTOCOL              *ShellParameters,
    IN EFI_SHELL_PROTOCOL                         *Shell
    );

typedef
CHAR16*
(EFIAPI *SHELL_COMMAND_GETHELP)(
    IN struct _EFI_SHELL_DYNAMIC_COMMAND_PROTOCOL *This,
    IN CONST CHAR8                                *Language
    );

typedef struct _EFI_SHELL_DYNAMIC_COMMAND_PROTOCOL {
    CONST CHAR16          *CommandName;
    SHELL_COMMAND_HANDLER Handler;
    SHELL_COMMAND_GETHELP GetHelp;
} EFI_SHELL_DYNAMIC_COMMAND_PROTOCOL;

#endif
