#ifndef _DEVPATH_H
#define _DEVPATH_H

/*++

Copyright (c) 1998  Intel Corporation

Module Name:

    devpath.h

Abstract:

    Defines for parsing the EFI Device Path structures



Revision History

--*/

//
// Device Path structures - Section C
//

typedef struct _EFI_DEVICE_PATH_PROTOCOL {
        UINT8                           Type;
        UINT8                           SubType;
        UINT8                           Length[2];
} EFI_DEVICE_PATH_PROTOCOL;

typedef struct _EFI_DEVICE_PATH_PROTOCOL _EFI_DEVICE_PATH;
typedef EFI_DEVICE_PATH_PROTOCOL EFI_DEVICE_PATH;

#define EFI_DP_TYPE_MASK                    0x7F
#define EFI_DP_TYPE_UNPACKED                0x80

//#define END_DEVICE_PATH_TYPE                0xff
#define END_DEVICE_PATH_TYPE                0x7f
//#define END_DEVICE_PATH_TYPE_UNPACKED       0x7f

#define END_ENTIRE_DEVICE_PATH_SUBTYPE      0xff
#define END_INSTANCE_DEVICE_PATH_SUBTYPE    0x01
#define END_DEVICE_PATH_LENGTH              (sizeof(EFI_DEVICE_PATH_PROTOCOL))


#define DP_IS_END_TYPE(a)
#define DP_IS_END_SUBTYPE(a)        ( ((a)->SubType == END_ENTIRE_DEVICE_PATH_SUBTYPE )

#define DevicePathType(a)           ( ((a)->Type) & EFI_DP_TYPE_MASK )
#define DevicePathSubType(a)        ( (a)->SubType )
#define DevicePathNodeLength(a)     ( ((a)->Length[0]) | ((a)->Length[1] << 8) )
#define NextDevicePathNode(a)       ( (EFI_DEVICE_PATH_PROTOCOL *) ( ((UINT8 *) (a)) + DevicePathNodeLength(a)))
//#define IsDevicePathEndType(a)      ( DevicePathType(a) == END_DEVICE_PATH_TYPE_UNPACKED )
#define IsDevicePathEndType(a)      ( DevicePathType(a) == END_DEVICE_PATH_TYPE )
#define IsDevicePathEndSubType(a)   ( (a)->SubType == END_ENTIRE_DEVICE_PATH_SUBTYPE )
#define IsDevicePathEnd(a)          ( IsDevicePathEndType(a) && IsDevicePathEndSubType(a) )
#define IsDevicePathUnpacked(a)     ( (a)->Type & EFI_DP_TYPE_UNPACKED )


#define SetDevicePathNodeLength(a,l) {                  \
            (a)->Length[0] = (UINT8) (l);               \
            (a)->Length[1] = (UINT8) ((l) >> 8);        \
            }

#define SetDevicePathEndNode(a)  {                      \
            (a)->Type = END_DEVICE_PATH_TYPE;           \
            (a)->SubType = END_ENTIRE_DEVICE_PATH_SUBTYPE;     \
            (a)->Length[0] = sizeof(EFI_DEVICE_PATH_PROTOCOL); \
            (a)->Length[1] = 0;                         \
            }


/*
 * Hardware Device Path (UEFI 2.4 specification, version 2.4 § 9.3.2.)
 */

#define HARDWARE_DEVICE_PATH            0x01

#define HW_PCI_DP                       0x01
typedef struct _PCI_DEVICE_PATH {
        EFI_DEVICE_PATH_PROTOCOL        Header;
        UINT8                           Function;
        UINT8                           Device;
} PCI_DEVICE_PATH;

#define HW_PCCARD_DP                    0x02
typedef struct _PCCARD_DEVICE_PATH {
        EFI_DEVICE_PATH_PROTOCOL        Header;
        UINT8                           FunctionNumber ;
} PCCARD_DEVICE_PATH;

#define HW_MEMMAP_DP                    0x03
typedef struct _MEMMAP_DEVICE_PATH {
        EFI_DEVICE_PATH_PROTOCOL        Header;
        UINT32                          MemoryType;
        EFI_PHYSICAL_ADDRESS            StartingAddress;
        EFI_PHYSICAL_ADDRESS            EndingAddress;
} MEMMAP_DEVICE_PATH;

#define HW_VENDOR_DP                    0x04
typedef struct _VENDOR_DEVICE_PATH {
        EFI_DEVICE_PATH_PROTOCOL        Header;
        EFI_GUID                        Guid;
} VENDOR_DEVICE_PATH;

#define UNKNOWN_DEVICE_GUID \
    { 0xcf31fac5, 0xc24e, 0x11d2,  {0x85, 0xf3, 0x0, 0xa0, 0xc9, 0x3e, 0xc9, 0x3b}  }

typedef struct _UKNOWN_DEVICE_VENDOR_DP {
    VENDOR_DEVICE_PATH      DevicePath;
    UINT8                   LegacyDriveLetter;
} UNKNOWN_DEVICE_VENDOR_DEVICE_PATH;

#define HW_CONTROLLER_DP            0x05
typedef struct _CONTROLLER_DEVICE_PATH {
        EFI_DEVICE_PATH_PROTOCOL  Header;
        UINT32                    Controller;
} CONTROLLER_DEVICE_PATH;


/*
 * ACPI Device Path (UEFI 2.4 specification, version 2.4 § 9.3.3 and 9.3.4.)
 */
#define ACPI_DEVICE_PATH                0x02

#define ACPI_DP                         0x01
typedef struct _ACPI_HID_DEVICE_PATH {
        EFI_DEVICE_PATH_PROTOCOL        Header;
        UINT32                          HID;
        UINT32                          UID;
} ACPI_HID_DEVICE_PATH;

#define EXPANDED_ACPI_DP                0x02
typedef struct _EXPANDED_ACPI_HID_DEVICE_PATH {
        EFI_DEVICE_PATH_PROTOCOL        Header;
        UINT32                          HID;
        UINT32                          UID;
        UINT32                          CID;
        UINT8                           HidStr[1];
} EXPANDED_ACPI_HID_DEVICE_PATH;

#define ACPI_ADR_DP 3
typedef struct _ACPI_ADR_DEVICE_PATH {
    EFI_DEVICE_PATH_PROTOCOL Header ;
    UINT32 ADR ;
} ACPI_ADR_DEVICE_PATH ;

//
// EISA ID Macro
// EISA ID Definition 32-bits
//  bits[15:0] - three character compressed ASCII EISA ID.
//  bits[31:16] - binary number
//   Compressed ASCII is 5 bits per character 0b00001 = 'A' 0b11010 = 'Z'
//
#define PNP_EISA_ID_CONST       0x41d0
#define EISA_ID(_Name, _Num)    ((UINT32) ((_Name) | (_Num) << 16))
#define EISA_PNP_ID(_PNPId)     (EISA_ID(PNP_EISA_ID_CONST, (_PNPId)))

#define PNP_EISA_ID_MASK        0xffff
#define EISA_ID_TO_NUM(_Id)     ((_Id) >> 16)


/*
 * Messaging Device Path (UEFI 2.4 specification, version 2.4 § 9.3.5.)
 */
#define MESSAGING_DEVICE_PATH           0x03

#define MSG_ATAPI_DP                    0x01
typedef struct _ATAPI_DEVICE_PATH {
        EFI_DEVICE_PATH_PROTOCOL        Header;
        UINT8                           PrimarySecondary;
        UINT8                           SlaveMaster;
        UINT16                          Lun;
} ATAPI_DEVICE_PATH;

#define MSG_SCSI_DP                     0x02
typedef struct _SCSI_DEVICE_PATH {
        EFI_DEVICE_PATH_PROTOCOL        Header;
        UINT16                          Pun;
        UINT16                          Lun;
} SCSI_DEVICE_PATH;

#define MSG_FIBRECHANNEL_DP             0x03
typedef struct _FIBRECHANNEL_DEVICE_PATH {
        EFI_DEVICE_PATH_PROTOCOL        Header;
        UINT32                          Reserved;
        UINT64                          WWN;
        UINT64                          Lun;
} FIBRECHANNEL_DEVICE_PATH;

/**
 * Fibre Channel Ex SubType.
 * UEFI 2.0 specification version 2.4 § 9.3.5.6.
 */
#define MSG_FIBRECHANNELEX_DP 21
typedef struct _FIBRECHANNELEX_DEVICE_PATH {
    EFI_DEVICE_PATH_PROTOCOL Header ;
    UINT32 Reserved ;
    UINT8 WWN[ 8 ] ; /* World Wide Name */
    UINT8 Lun[ 8 ] ; /* Logical unit, T-10 SCSI Architecture Model 4 specification */
} FIBRECHANNELEX_DEVICE_PATH ;

#define MSG_1394_DP                     0x04
typedef struct _F1394_DEVICE_PATH {
        EFI_DEVICE_PATH_PROTOCOL        Header;
        UINT32                          Reserved;
        UINT64                          Guid;
} F1394_DEVICE_PATH;

#define MSG_USB_DP                      0x05
typedef struct _USB_DEVICE_PATH {
        EFI_DEVICE_PATH_PROTOCOL        Header;
        UINT8                           Port;
        UINT8                           Endpoint;
} USB_DEVICE_PATH;

/**
 * SATA Device Path SubType.
 * UEFI 2.0 specification version 2.4 § 9.3.5.6.
 */
#define MSG_SATA_DP 18
typedef struct _SATA_DEVICE_PATH {
    EFI_DEVICE_PATH_PROTOCOL Header ;
    UINT16 HBAPortNumber ;
    UINT16 PortMultiplierPortNumber ;
    UINT16 Lun ; /* Logical Unit Number */
} SATA_DEVICE_PATH ;

/**
 * USB WWID Device Path SubType.
 * UEFI 2.0 specification version 2.4 § 9.3.5.7.
 */
#define MSG_USB_WWID_DP 16
typedef struct _USB_WWID_DEVICE_PATH {
    EFI_DEVICE_PATH_PROTOCOL Header ;
    UINT16 InterfaceNumber ;
    UINT16 VendorId ;
    UINT16 ProductId ;
    CHAR16 SerialNumber[ 1 ] ; /* UTF-16 characters of the USB serial number */
} USB_WWID_DEVICE_PATH ;

/**
 * Device Logical Unit SubType.
 * UEFI 2.0 specification version 2.4 § 9.3.5.8.
 */
#define MSG_DEVICE_LOGICAL_UNIT_DP 17
typedef struct _DEVICE_LOGICAL_UNIT_DEVICE_PATH {
    EFI_DEVICE_PATH_PROTOCOL Header ;
    UINT8 Lun ; /* Logical Unit Number */
} DEVICE_LOGICAL_UNIT_DEVICE_PATH ;

#define MSG_USB_CLASS_DP                0x0F
typedef struct _USB_CLASS_DEVICE_PATH {
        EFI_DEVICE_PATH_PROTOCOL        Header;
        UINT16                          VendorId;
        UINT16                          ProductId;
        UINT8                           DeviceClass;
        UINT8                           DeviceSubclass;
        UINT8                           DeviceProtocol;
} USB_CLASS_DEVICE_PATH;

#define MSG_I2O_DP                      0x06
typedef struct _I2O_DEVICE_PATH {
        EFI_DEVICE_PATH_PROTOCOL        Header;
        UINT32                          Tid;
} I2O_DEVICE_PATH;

#define MSG_MAC_ADDR_DP                 0x0b
typedef struct _MAC_ADDR_DEVICE_PATH {
        EFI_DEVICE_PATH_PROTOCOL        Header;
        EFI_MAC_ADDRESS                 MacAddress;
        UINT8                           IfType;
} MAC_ADDR_DEVICE_PATH;

#define MSG_IPv4_DP                     0x0c
typedef struct _IPv4_DEVICE_PATH {
        EFI_DEVICE_PATH_PROTOCOL        Header;
        EFI_IPv4_ADDRESS                LocalIpAddress;
        EFI_IPv4_ADDRESS                RemoteIpAddress;
        UINT16                          LocalPort;
        UINT16                          RemotePort;
        UINT16                          Protocol;
        BOOLEAN                         StaticIpAddress;
        /* new from UEFI version 2, code must check Length field in Header */
        EFI_IPv4_ADDRESS                GatewayIpAddress ;
        EFI_IPv4_ADDRESS                SubnetMask ;
} IPv4_DEVICE_PATH;

#define MSG_IPv6_DP                     0x0d
typedef struct _IPv6_DEVICE_PATH {
        EFI_DEVICE_PATH_PROTOCOL        Header;
        EFI_IPv6_ADDRESS                LocalIpAddress;
        EFI_IPv6_ADDRESS                RemoteIpAddress;
        UINT16                          LocalPort;
        UINT16                          RemotePort;
        UINT16                          Protocol;
        BOOLEAN                         IPAddressOrigin ;
        /* new from UEFI version 2, code must check Length field in Header */
        UINT8                           PrefixLength ;
        EFI_IPv6_ADDRESS                GatewayIpAddress ;
} IPv6_DEVICE_PATH;


/**
 * Uniform Resource Identifiers SubType.
 * UEFI 2.0 specification version 2.4C § 9.3.5.23.
 */
#define MSG_URI_DP                      24
typedef struct _URI_DEVICE_PATH {
        EFI_DEVICE_PATH_PROTOCOL        Header;
        CHAR8                           Uri[1];
} URI_DEVICE_PATH;

/**
 * Device Logical Unit SubType.
 * UEFI 2.0 specification version 2.4 § 9.3.5.8.
 */
#define MSG_VLAN_DP 20
typedef struct _VLAN_DEVICE_PATH {
    EFI_DEVICE_PATH_PROTOCOL Header ;
    UINT16 VlanId ;
} VLAN_DEVICE_PATH;

#define MSG_INFINIBAND_DP               0x09
typedef struct _INFINIBAND_DEVICE_PATH {
        EFI_DEVICE_PATH_PROTOCOL        Header;
        UINT32                          ResourceFlags;
        UINT8                           PortGid[16];
        UINT64                          ServiceId;
        UINT64                          TargetPortId;
        UINT64                          DeviceId;
} INFINIBAND_DEVICE_PATH;

#define MSG_UART_DP                     0x0e
typedef struct _UART_DEVICE_PATH {
        EFI_DEVICE_PATH_PROTOCOL        Header;
        UINT32                          Reserved;
        UINT64                          BaudRate;
        UINT8                           DataBits;
        UINT8                           Parity;
        UINT8                           StopBits;
} UART_DEVICE_PATH;

#define MSG_VENDOR_DP                   0x0A
/* Use VENDOR_DEVICE_PATH struct */

#define EFI_PC_ANSI_GUID \
    { 0xe0c14753, 0xf9be, 0x11d2, {0x9a, 0x0c, 0x00, 0x90, 0x27, 0x3f, 0xc1, 0x4d} }
#define DEVICE_PATH_MESSAGING_PC_ANSI EFI_PC_ANSI_GUID

#define EFI_VT_100_GUID \
    { 0xdfa66065, 0xb419, 0x11d3, {0x9a, 0x2d, 0x00, 0x90, 0x27, 0x3f, 0xc1, 0x4d} }
#define DEVICE_PATH_MESSAGING_VT_100 EFI_VT_100_GUID

#define EFI_VT_100_PLUS_GUID \
    { 0x7baec70b, 0x57e0, 0x4c76, {0x8e, 0x87, 0x2f, 0x9e, 0x28, 0x08, 0x83, 0x43} }
#define DEVICE_PATH_MESSAGING_VT_100_PLUS EFI_VT_100_PLUS_GUID

#define EFI_VT_UTF8_GUID \
    { 0xad15a0d6, 0x8bec, 0x4acf, {0xa0, 0x73, 0xd0, 0x1d, 0xe7, 0x7e, 0x2d, 0x88} }
#define DEVICE_PATH_MESSAGING_VT_UTF8 EFI_VT_UTF8_GUID


/*
 * Media Device Path (UEFI 2.4 specification, version 2.4 § 9.3.6.)
 */
#define MEDIA_DEVICE_PATH               0x04

#define MEDIA_HARDDRIVE_DP              0x01
typedef struct _HARDDRIVE_DEVICE_PATH {
        EFI_DEVICE_PATH_PROTOCOL        Header;
        UINT32                          PartitionNumber;
        UINT64                          PartitionStart;
        UINT64                          PartitionSize;
        UINT8                           Signature[16];
        UINT8                           MBRType;
        UINT8                           SignatureType;
} HARDDRIVE_DEVICE_PATH;

#define MBR_TYPE_PCAT                       0x01
#define MBR_TYPE_EFI_PARTITION_TABLE_HEADER 0x02

#define SIGNATURE_TYPE_MBR                  0x01
#define SIGNATURE_TYPE_GUID                 0x02

#define MEDIA_CDROM_DP                  0x02
typedef struct _CDROM_DEVICE_PATH {
        EFI_DEVICE_PATH_PROTOCOL        Header;
        UINT32                          BootEntry;
        UINT64                          PartitionStart;
        UINT64                          PartitionSize;
} CDROM_DEVICE_PATH;

#define MEDIA_VENDOR_DP                 0x03
/* Use VENDOR_DEVICE_PATH struct */

#define MEDIA_FILEPATH_DP               0x04
typedef struct _FILEPATH_DEVICE_PATH {
        EFI_DEVICE_PATH_PROTOCOL        Header;
        CHAR16                          PathName[1];
} FILEPATH_DEVICE_PATH;

#define SIZE_OF_FILEPATH_DEVICE_PATH EFI_FIELD_OFFSET(FILEPATH_DEVICE_PATH,PathName)

#define MEDIA_PROTOCOL_DP               0x05
typedef struct _MEDIA_PROTOCOL_DEVICE_PATH {
        EFI_DEVICE_PATH_PROTOCOL        Header;
        EFI_GUID                        Protocol;
} MEDIA_PROTOCOL_DEVICE_PATH;

/**
 * PIWG Firmware File SubType.
 * UEFI 2.0 specification version 2.4 § 9.3.6.6.
 */
#define MEDIA_PIWG_FW_FILE_DP 6
typedef struct _MEDIA_FW_VOL_FILEPATH_DEVICE_PATH {
    EFI_DEVICE_PATH_PROTOCOL Header ;
    EFI_GUID FvFileName ;
} MEDIA_FW_VOL_FILEPATH_DEVICE_PATH ;

/**
 * PIWG Firmware Volume Device Path SubType.
 * UEFI 2.0 specification version 2.4 § 9.3.6.7.
 */
#define MEDIA_PIWG_FW_VOL_DP 7
typedef struct _MEDIA_FW_VOL_DEVICE_PATH {
    EFI_DEVICE_PATH_PROTOCOL Header ;
    EFI_GUID FvName ;
} MEDIA_FW_VOL_DEVICE_PATH ;

/**
 * Media relative offset range device path.
 * UEFI 2.0 specification version 2.4 § 9.3.6.8.
 */
#define MEDIA_RELATIVE_OFFSET_RANGE_DP 8
typedef struct _MEDIA_RELATIVE_OFFSET_RANGE_DEVICE_PATH {
    EFI_DEVICE_PATH_PROTOCOL Header ;
    UINT32 Reserved ;
    UINT64 StartingOffset ;
    UINT64 EndingOffset ;
} MEDIA_RELATIVE_OFFSET_RANGE_DEVICE_PATH ;


/*
 * BIOS Boot Specification Device Path (UEFI 2.4 specification, version 2.4 § 9.3.7.)
 */
#define BBS_DEVICE_PATH                 0x05

#define BBS_BBS_DP                      0x01
typedef struct _BBS_BBS_DEVICE_PATH {
        EFI_DEVICE_PATH_PROTOCOL        Header;
        UINT16                          DeviceType;
        UINT16                          StatusFlag;
        CHAR8                           String[1];
} BBS_BBS_DEVICE_PATH;

/* DeviceType definitions - from BBS specification */
#define BBS_TYPE_FLOPPY                 0x01
#define BBS_TYPE_HARDDRIVE              0x02
#define BBS_TYPE_CDROM                  0x03
#define BBS_TYPE_PCMCIA                 0x04
#define BBS_TYPE_USB                    0x05
#define BBS_TYPE_EMBEDDED_NETWORK       0x06
#define BBS_TYPE_DEV                    0x80
#define BBS_TYPE_UNKNOWN                0xFF

typedef union {
    EFI_DEVICE_PATH_PROTOCOL             DevPath;
    PCI_DEVICE_PATH                      Pci;
    PCCARD_DEVICE_PATH                   PcCard;
    MEMMAP_DEVICE_PATH                   MemMap;
    VENDOR_DEVICE_PATH                   Vendor;
    UNKNOWN_DEVICE_VENDOR_DEVICE_PATH    UnknownVendor;
    CONTROLLER_DEVICE_PATH               Controller;
    ACPI_HID_DEVICE_PATH                 Acpi;

    ATAPI_DEVICE_PATH                    Atapi;
    SCSI_DEVICE_PATH                     Scsi;
    FIBRECHANNEL_DEVICE_PATH             FibreChannel;

    F1394_DEVICE_PATH                    F1394;
    USB_DEVICE_PATH                      Usb;
    USB_CLASS_DEVICE_PATH                UsbClass;
    I2O_DEVICE_PATH                      I2O;
    MAC_ADDR_DEVICE_PATH                 MacAddr;
    IPv4_DEVICE_PATH                     Ipv4;
    IPv6_DEVICE_PATH                     Ipv6;
    URI_DEVICE_PATH                      Uri;
    INFINIBAND_DEVICE_PATH               InfiniBand;
    UART_DEVICE_PATH                     Uart;

    HARDDRIVE_DEVICE_PATH                HardDrive;
    CDROM_DEVICE_PATH                    CD;

    FILEPATH_DEVICE_PATH                 FilePath;
    MEDIA_PROTOCOL_DEVICE_PATH           MediaProtocol;

    BBS_BBS_DEVICE_PATH                  Bbs;

} EFI_DEV_PATH;

typedef union {
    EFI_DEVICE_PATH_PROTOCOL             *DevPath;
    PCI_DEVICE_PATH                      *Pci;
    PCCARD_DEVICE_PATH                   *PcCard;
    MEMMAP_DEVICE_PATH                   *MemMap;
    VENDOR_DEVICE_PATH                   *Vendor;
    UNKNOWN_DEVICE_VENDOR_DEVICE_PATH    *UnknownVendor;
    CONTROLLER_DEVICE_PATH               *Controller;
    ACPI_HID_DEVICE_PATH                 *Acpi;

    ATAPI_DEVICE_PATH                    *Atapi;
    SCSI_DEVICE_PATH                     *Scsi;
    FIBRECHANNEL_DEVICE_PATH             *FibreChannel;

    F1394_DEVICE_PATH                    *F1394;
    USB_DEVICE_PATH                      *Usb;
    USB_CLASS_DEVICE_PATH                *UsbClass;
    I2O_DEVICE_PATH                      *I2O;
    MAC_ADDR_DEVICE_PATH                 *MacAddr;
    IPv4_DEVICE_PATH                     *Ipv4;
    IPv6_DEVICE_PATH                     *Ipv6;
    URI_DEVICE_PATH                      *Uri;
    INFINIBAND_DEVICE_PATH               *InfiniBand;
    UART_DEVICE_PATH                     *Uart;

    HARDDRIVE_DEVICE_PATH                *HardDrive;

    FILEPATH_DEVICE_PATH                 *FilePath;
    MEDIA_PROTOCOL_DEVICE_PATH           *MediaProtocol;

    CDROM_DEVICE_PATH                    *CD;
    BBS_BBS_DEVICE_PATH                  *Bbs;

} EFI_DEV_PATH_PTR;

#define EFI_DEVICE_PATH_TO_TEXT_PROTOCOL_GUID \
  { 0x8b843e20, 0x8132, 0x4852, {0x90, 0xcc, 0x55, 0x1a, 0x4e, 0x4a, 0x7f, 0x1c} }

typedef
CHAR16*
(EFIAPI *EFI_DEVICE_PATH_TO_TEXT_NODE) (
    IN CONST EFI_DEVICE_PATH_PROTOCOL    *DeviceNode,
    IN BOOLEAN                           DisplayOnly,
    IN BOOLEAN                           AllowShortcuts
    );

typedef
CHAR16*
(EFIAPI *EFI_DEVICE_PATH_TO_TEXT_PATH) (
    IN CONST EFI_DEVICE_PATH_PROTOCOL    *DevicePath,
    IN BOOLEAN                           DisplayOnly,
    IN BOOLEAN                           AllowShortcuts
    );

typedef struct _EFI_DEVICE_PATH_TO_TEXT_PROTOCOL {
    EFI_DEVICE_PATH_TO_TEXT_NODE         ConvertDeviceNodeToText;
    EFI_DEVICE_PATH_TO_TEXT_PATH         ConvertDevicePathToText;
} EFI_DEVICE_PATH_TO_TEXT_PROTOCOL;

#define EFI_DEVICE_PATH_FROM_TEXT_PROTOCOL_GUID \
  { 0x5c99a21, 0xc70f, 0x4ad2, {0x8a, 0x5f, 0x35, 0xdf, 0x33, 0x43, 0xf5, 0x1e} }

typedef
EFI_DEVICE_PATH_PROTOCOL*
(EFIAPI *EFI_DEVICE_PATH_FROM_TEXT_NODE) (
    IN CONST CHAR16                      *TextDeviceNode
    );

typedef
EFI_DEVICE_PATH_PROTOCOL*
(EFIAPI *EFI_DEVICE_PATH_FROM_TEXT_PATH) (
    IN CONST CHAR16                      *TextDevicePath
    );

typedef struct {
    EFI_DEVICE_PATH_FROM_TEXT_NODE       ConvertTextToDeviceNode;
    EFI_DEVICE_PATH_FROM_TEXT_PATH       ConvertTextToDevicePath;
} EFI_DEVICE_PATH_FROM_TEXT_PROTOCOL;

#define EFI_DEVICE_PATH_UTILITIES_PROTOCOL_GUID \
  { 0x379be4e, 0xd706, 0x437d, {0xb0, 0x37, 0xed, 0xb8, 0x2f, 0xb7, 0x72, 0xa4} }

typedef
UINTN
(EFIAPI *EFI_DEVICE_PATH_UTILS_GET_DEVICE_PATH_SIZE) (
    IN CONST EFI_DEVICE_PATH_PROTOCOL    *DevicePath
    );

typedef
EFI_DEVICE_PATH_PROTOCOL*
(EFIAPI *EFI_DEVICE_PATH_UTILS_DUP_DEVICE_PATH) (
    IN CONST EFI_DEVICE_PATH_PROTOCOL    *DevicePath
    );

typedef
EFI_DEVICE_PATH_PROTOCOL*
(EFIAPI *EFI_DEVICE_PATH_UTILS_APPEND_PATH) (
    IN CONST EFI_DEVICE_PATH_PROTOCOL    *Src1,
    IN CONST EFI_DEVICE_PATH_PROTOCOL    *Src2
    );

typedef
EFI_DEVICE_PATH_PROTOCOL*
(EFIAPI *EFI_DEVICE_PATH_UTILS_APPEND_NODE) (
    IN CONST EFI_DEVICE_PATH_PROTOCOL    *DevicePath,
    IN CONST EFI_DEVICE_PATH_PROTOCOL    *DeviceNode
);

typedef
EFI_DEVICE_PATH_PROTOCOL*
(EFIAPI *EFI_DEVICE_PATH_UTILS_APPEND_INSTANCE) (
    IN CONST EFI_DEVICE_PATH_PROTOCOL    *DevicePath,
    IN CONST EFI_DEVICE_PATH_PROTOCOL    *DevicePathInstance
);

typedef
EFI_DEVICE_PATH_PROTOCOL*
(EFIAPI *EFI_DEVICE_PATH_UTILS_GET_NEXT_INSTANCE) (
    IN OUT EFI_DEVICE_PATH_PROTOCOL      **DevicePathInstance,
    OUT UINTN                            *DevicePathInstanceSize OPTIONAL
);

typedef
EFI_DEVICE_PATH_PROTOCOL*
(EFIAPI *EFI_DEVICE_PATH_UTILS_CREATE_NODE) (
    IN UINT8                             NodeType,
    IN UINT8                             NodeSubType,
    IN UINT16                            NodeLength
    );

typedef
BOOLEAN
(EFIAPI *EFI_DEVICE_PATH_UTILS_IS_MULTI_INSTANCE) (
    IN CONST EFI_DEVICE_PATH_PROTOCOL    *DevicePath
    );

typedef struct _EFI_DEVICE_PATH_UTILITIES_PROTOCOL {
    EFI_DEVICE_PATH_UTILS_GET_DEVICE_PATH_SIZE   GetDevicePathSize;
    EFI_DEVICE_PATH_UTILS_DUP_DEVICE_PATH        DuplicateDevicePath;
    EFI_DEVICE_PATH_UTILS_APPEND_PATH            AppendDevicePath;
    EFI_DEVICE_PATH_UTILS_APPEND_NODE            AppendDeviceNode;
    EFI_DEVICE_PATH_UTILS_APPEND_INSTANCE        AppendDevicePathInstance;
    EFI_DEVICE_PATH_UTILS_GET_NEXT_INSTANCE      GetNextDevicePathInstance;
    EFI_DEVICE_PATH_UTILS_IS_MULTI_INSTANCE      IsDevicePathMultiInstance;
    EFI_DEVICE_PATH_UTILS_CREATE_NODE            CreateDeviceNode;
} EFI_DEVICE_PATH_UTILITIES_PROTOCOL;

#endif
